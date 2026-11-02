//+------------------------------------------------------------------+
//|                                                EA31337 framework |
//|                                 Copyright 2016-2024, EA31337 Ltd |
//|                                        https://ea31337.github.io |
//+------------------------------------------------------------------+

/*
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * @file
 * Implements Williams Percent Range (WPR) indicator.
 */

#ifndef __MQL__
// Allows the preprocessor to include a header file when it is needed.
#pragma once
#endif

// Includes.
#include "../../Indicator/Indicator.h"
#include "../../Platform/Terminal.h"

#ifdef __MQL__
#define INDI_WPR_DEFAULT_SOURCE_MODE IDATA_BUILTIN
#else
#define INDI_WPR_DEFAULT_SOURCE_MODE IDATA_INDICATOR
#endif

// Structs.
struct IndiWPRParams : IndicatorParams {
  unsigned int period;
  // Struct constructors.
  IndiWPRParams(unsigned int _period = 14, int _shift = 0) : IndicatorParams(INDI_WPR), period(_period) {
    shift = _shift;
    SetCustomIndicatorName("Examples\\WPR");
  };
  IndiWPRParams(IndiWPRParams &_params) { THIS_REF = _params; };
};

/**
 * Implements the Larry Williams' Percent Range.
 */
class Indi_WPR : public Indicator<IndiWPRParams> {
 public:
  /**
   * Class constructor.
   */
  Indi_WPR(IndiWPRParams &_p, ENUM_IDATA_SOURCE_TYPE _idstype = INDI_WPR_DEFAULT_SOURCE_MODE,
           IndicatorData *_indi_src = NULL,
           int _indi_src_mode = 0)
      : Indicator(_p, IndicatorDataParams::GetInstance(1, TYPE_DOUBLE, _idstype, IDATA_RANGE_RANGE, _indi_src_mode),
                  _indi_src) {}
  Indi_WPR(int _shift = 0, ENUM_IDATA_SOURCE_TYPE _idstype = INDI_WPR_DEFAULT_SOURCE_MODE,
           IndicatorData *_indi_src = NULL,
           int _indi_src_mode = 0)
      : Indicator(IndiWPRParams(),
                  IndicatorDataParams::GetInstance(1, TYPE_DOUBLE, _idstype, IDATA_RANGE_RANGE, _indi_src_mode),
                  _indi_src) {}
  /**
   * Returns possible data source types. It is a bit mask of ENUM_INDI_SUITABLE_DS_TYPE.
   */
  unsigned int GetSuitableDataSourceTypes() override { return INDI_SUITABLE_DS_TYPE_CANDLE; }

  /**
   * Returns possible data source modes. It is a bit mask of ENUM_IDATA_SOURCE_TYPE.
   */
  unsigned int GetPossibleDataModes() override { return IDATA_BUILTIN | IDATA_ICUSTOM | IDATA_INDICATOR; }

  /**
   * Checks if indicator entry values are valid.
   */
  virtual bool IsValidEntry(IndicatorDataEntry &_entry) override { return _entry.IsWithinRange(-100.0, 0.0); }

  /**
   * Calculates the Larry Williams' Percent Range and returns its value.
   *
   * @docs
   * - https://docs.mql4.com/indicators/iwpr
   * - https://www.mql5.com/en/docs/indicators/iwpr
   */
  static double iWPR(string _symbol = NULL_STRING, ENUM_TIMEFRAMES _tf = PERIOD_CURRENT, unsigned int _period = 14,
                     int _shift = 0, IndicatorData *_obj = NULL) {
#ifdef __MQL__
#ifdef __MQL4__
    return ::iWPR(_symbol, _tf, _period, _shift);
#else  // __MQL5__
    int _handle = Object::IsValid(_obj) ? _obj.Get<int>(IndicatorDataState::INDICATOR_DATA_STATE_PROP_HANDLE) : NULL;
    double _res[];
    if (_handle == NULL || _handle == INVALID_HANDLE) {
      if ((_handle = ::iWPR(_symbol, _tf, _period)) == INVALID_HANDLE) {
        SetUserError(ERR_USER_INVALID_HANDLE);
        return EMPTY_VALUE;
      } else if (Object::IsValid(_obj)) {
        _obj.SetHandle(_handle);
      }
    }
    if (Terminal::IsVisualMode()) {
      // To avoid error 4806 (ERR_INDICATOR_DATA_NOT_FOUND),
      // we check the number of calculated data only in visual mode.
      int _bars_calc = BarsCalculated(_handle);
      if (GetLastError() > 0) {
        return EMPTY_VALUE;
      } else if (_bars_calc <= 2) {
        SetUserError(ERR_USER_INVALID_BUFF_NUM);
        return EMPTY_VALUE;
      }
    }
    if (CopyBuffer(_handle, 0, _shift, 1, _res) < 0) {
      return ArraySize(_res) > 0 ? _res[0] : EMPTY_VALUE;
    }
    return _res[0];
#endif
#else  // Non-MQL.
    if (_obj != NULL && _obj PTR_DEREF HasDataSource()) {
      return iWPROnIndicator((Indi_WPR *)_obj, _obj PTR_DEREF GetCandle(), _period, _shift);
    }
    RUNTIME_ERROR("WPR requires a candle data source.");
    return EMPTY_VALUE;
#endif
  }

  /** Calculates WPR on the candle source and caches each computed value. */
  static double iWPROnIndicator(Indi_WPR *_target, IndicatorData *_source, unsigned int _period, int _shift = 0) {
    if (_source == NULL || _period == 0) return EMPTY_VALUE;

    ValueStorage<double> *_high = (ValueStorage<double> *)_source PTR_DEREF
        GetSpecificValueStorage(INDI_DATA_VS_TYPE_PRICE_HIGH);
    ValueStorage<double> *_low = (ValueStorage<double> *)_source PTR_DEREF
        GetSpecificValueStorage(INDI_DATA_VS_TYPE_PRICE_LOW);
    ValueStorage<double> *_close = (ValueStorage<double> *)_source PTR_DEREF
        GetSpecificValueStorage(INDI_DATA_VS_TYPE_PRICE_CLOSE);
    if (_high == NULL || _low == NULL || _close == NULL) return EMPTY_VALUE;

    IndiBufferCache<double> *_cache = _target PTR_DEREF GetCache();
    const int _rates_total = _close PTR_DEREF Size();
    _cache PTR_DEREF SetPriceBuffer(PTR_TO_REF(_close), _rates_total);
    if (!_cache PTR_DEREF HasBuffers()) {
      _cache PTR_DEREF AddBuffer<NativeValueStorage<double>>();
    }

    ValueStorage<double> &_wpr = _cache PTR_DEREF GetBuffer<double>(0);
    const int _begin = (int)_period - 1;
    const int _prev_calculated = _cache PTR_DEREF GetPrevCalculated();
    const int _start = _prev_calculated == 0 || _prev_calculated - 1 < _begin ? _begin : _prev_calculated - 1;

    for (int i = _start; i < _rates_total && !IsStopped(); ++i) {
      double _highest = -DBL_MAX;
      double _lowest = DBL_MAX;
      for (int j = i - (int)_period + 1; j <= i; ++j) {
        _highest = fmax(_highest, PTR_TO_REF(_high)[j].Get());
        _lowest = fmin(_lowest, PTR_TO_REF(_low)[j].Get());
      }

      const double _range = _highest - _lowest;
      _wpr[i] = _range == 0.0 ? 0.0 : -100.0 * (_highest - PTR_TO_REF(_close)[i].Get()) / _range;
    }

    _cache PTR_DEREF SetPrevCalculated(_rates_total);
    if (_shift < 0 || _shift >= _rates_total) return EMPTY_VALUE;
    return _cache PTR_DEREF GetTailValue<double>(0, _shift);
  }

  /**
   * Returns the indicator's value.
   */
  virtual IndicatorDataEntryValue GetEntryValue(int _mode = 0, int _abs_shift = 0) override {
    double _value = EMPTY_VALUE;
    switch (Get<ENUM_IDATA_SOURCE_TYPE>(STRUCT_ENUM(IndicatorDataParams, IDATA_PARAM_IDSTYPE))) {
      case IDATA_BUILTIN:
        _value = Indi_WPR::iWPR(GetSymbol(), GetTf(), GetPeriod(), ToRelShift(_abs_shift), THIS_PTR);
        break;
      case IDATA_INDICATOR:
        _value = Indi_WPR::iWPROnIndicator(THIS_PTR, GetCandle(), GetPeriod(), ToRelShift(_abs_shift));
        break;
      case IDATA_ICUSTOM:
        _value = iCustom(istate.handle, GetSymbol(), GetTf(), iparams.GetCustomIndicatorName(), /*[*/ GetPeriod() /*]*/,
                         0, ToRelShift(_abs_shift));
        break;
      default:
        SetUserError(ERR_INVALID_PARAMETER);
    }
    return _value;
  }

  /* Getters */

  /**
   * Get period value.
   */
  unsigned int GetPeriod() { return iparams.period; }

  /* Setters */

  /**
   * Set period (bars count) for the indicator calculation.
   */
  void SetPeriod(unsigned int _period) {
    istate.is_changed = true;
    iparams.period = _period;
  }
};

#undef INDI_WPR_DEFAULT_SOURCE_MODE

#ifndef __MQL4__
// Defines global functions (for MQL4 backward compability).
double iWPR(string _symbol, int _tf, int _period, int _shift) {
  ResetLastError();
  return Indi_WPR::iWPR(_symbol, (ENUM_TIMEFRAMES)_tf, _period, _shift);
}
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>

EMSCRIPTEN_BINDINGS(Indi_WPR_Params) {
  emscripten::value_object<IndiWPRParams>("indicators.WPRParams")
      .field("period", &IndiWPRParams::period)
      .field("shift", &IndiWPRParams::shift);
}

EMSCRIPTEN_BINDINGS(Indi_WPRBase) {
  emscripten::class_<Indicator<IndiWPRParams>, emscripten::base<IndicatorData>>("Indi_WPRBase");
}

EMSCRIPTEN_BINDINGS(Indi_WPR) {
  emscripten::class_<Indi_WPR, emscripten::base<Indicator<IndiWPRParams>>>("indicators.WPR")
      .smart_ptr<Ref<Indi_WPR>>("Ref<Indi_WPR>")
      .constructor(&make_ref<Indi_WPR, IndiWPRParams &>)
      .constructor(&make_ref<Indi_WPR, IndiWPRParams &, ENUM_IDATA_SOURCE_TYPE>)
      .constructor(&make_ref<Indi_WPR, IndiWPRParams &, ENUM_IDATA_SOURCE_TYPE, IndicatorData *>,
                   emscripten::allow_raw_pointer<emscripten::arg<2>>())
      .constructor(&make_ref<Indi_WPR, IndiWPRParams &, ENUM_IDATA_SOURCE_TYPE, IndicatorData *, int>,
                   emscripten::allow_raw_pointer<emscripten::arg<2>>());
}

#endif
