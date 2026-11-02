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

#ifndef __MQL__
// Allows the preprocessor to include a header file when it is needed.
#pragma once
#endif

// Includes.
#include "../../Indicator/Indicator.h"

#ifdef __MQL__
#define INDI_SAR_DEFAULT_SOURCE_MODE IDATA_BUILTIN
#else
#define INDI_SAR_DEFAULT_SOURCE_MODE IDATA_INDICATOR
#endif

// Structs.
struct IndiSARParams : IndicatorParams {
  double step;
  double max;
  // Struct constructors.
  IndiSARParams(double _step = 0.02, double _max = 0.2, int _shift = 0)
      : IndicatorParams(INDI_SAR), step(_step), max(_max) {
    shift = _shift;
    SetCustomIndicatorName("Examples\\ParabolicSAR");
  };
  IndiSARParams(IndiSARParams &_params) { THIS_REF = _params; };
};

/**
 * Implements the Parabolic Stop and Reverse system indicator.
 */
class Indi_SAR : public Indicator<IndiSARParams> {
 public:
  /**
   * Class constructor.
   */
  Indi_SAR(IndiSARParams &_p, ENUM_IDATA_SOURCE_TYPE _idstype = INDI_SAR_DEFAULT_SOURCE_MODE,
           IndicatorData *_indi_src = NULL,
           int _indi_src_mode = 0)
      : Indicator(_p, IndicatorDataParams::GetInstance(1, TYPE_DOUBLE, _idstype, IDATA_RANGE_PRICE, _indi_src_mode),
                  _indi_src) {}
  Indi_SAR(int _shift = 0, ENUM_IDATA_SOURCE_TYPE _idstype = INDI_SAR_DEFAULT_SOURCE_MODE,
           IndicatorData *_indi_src = NULL,
           int _indi_src_mode = 0)
      : Indicator(IndiSARParams(),
                  IndicatorDataParams::GetInstance(1, TYPE_DOUBLE, _idstype, IDATA_RANGE_PRICE, _indi_src_mode),
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
   * Returns the indicator value.
   *
   * @docs
   * - https://docs.mql4.com/indicators/isar
   * - https://www.mql5.com/en/docs/indicators/isar
   */
  static double iSAR(string _symbol = NULL_STRING, ENUM_TIMEFRAMES _tf = PERIOD_CURRENT, double _step = 0.02,
                     double _max = 0.2, int _shift = 0, IndicatorData *_obj = NULL) {
#ifdef __MQL__
#ifdef __MQL4__
    return ::iSAR(_symbol, _tf, _step, _max, _shift);
#else  // __MQL5__
    INDICATOR_BUILTIN_CALL_AND_RETURN(::iSAR(_symbol, _tf, _step, _max), 0, _shift);
#endif
#else  // Non-MQL.
    if (_obj != NULL && _obj PTR_DEREF HasDataSource()) {
      return iSAROnIndicator((Indi_SAR *)_obj, _obj PTR_DEREF GetCandle(), _step, _max, _shift);
    }
    RUNTIME_ERROR("SAR requires a candle data source.");
    return EMPTY_VALUE;
#endif
  }

  /** Calculates Parabolic SAR incrementally from the source candle history. */
  static double iSAROnIndicator(Indi_SAR *_target, IndicatorData *_source, double _step, double _max,
                                int _shift = 0) {
    if (_source == NULL || _shift < 0 || _step <= 0.0 || _max <= 0.0) return EMPTY_VALUE;

    ValueStorage<double> *_high = (ValueStorage<double> *)_source PTR_DEREF
        GetSpecificValueStorage(INDI_DATA_VS_TYPE_PRICE_HIGH);
    ValueStorage<double> *_low = (ValueStorage<double> *)_source PTR_DEREF
        GetSpecificValueStorage(INDI_DATA_VS_TYPE_PRICE_LOW);
    ValueStorage<double> *_close = (ValueStorage<double> *)_source PTR_DEREF
        GetSpecificValueStorage(INDI_DATA_VS_TYPE_PRICE_CLOSE);
    if (_high == NULL || _low == NULL || _close == NULL) return EMPTY_VALUE;

    IndiBufferCache<double> *_cache = _target PTR_DEREF GetCache();
    const int _rates_total = _close PTR_DEREF Size();
    if (_rates_total < 2 || _shift >= _rates_total) return EMPTY_VALUE;

    _cache PTR_DEREF SetPriceBuffer(PTR_TO_REF(_close), _rates_total);
    if (!_cache PTR_DEREF HasBuffers()) {
      _cache PTR_DEREF AddBuffer<NativeValueStorage<double>>(4);
    }

    ValueStorage<double> &_sar = _cache PTR_DEREF GetBuffer<double>(0);
    ValueStorage<double> &_ep = _cache PTR_DEREF GetBuffer<double>(1);
    ValueStorage<double> &_af = _cache PTR_DEREF GetBuffer<double>(2);
    ValueStorage<double> &_trend = _cache PTR_DEREF GetBuffer<double>(3);

    double _max_step = _max < _step ? _step : _max;
    const int _prev_calculated = _cache PTR_DEREF GetPrevCalculated();
    int _start;
    if (_prev_calculated == 0) {
      const bool _initial_uptrend = PTR_TO_REF(_close)[1].Get() >= PTR_TO_REF(_close)[0].Get();
      _sar[0] = _initial_uptrend ? PTR_TO_REF(_low)[0].Get() : PTR_TO_REF(_high)[0].Get();
      _ep[0] = _initial_uptrend ? PTR_TO_REF(_high)[0].Get() : PTR_TO_REF(_low)[0].Get();
      _af[0] = _step;
      _trend[0] = _initial_uptrend ? 1.0 : 0.0;
      _start = 1;
    } else {
      _start = _prev_calculated > 1 ? _prev_calculated - 1 : 1;
    }

    for (int i = _start; i < _rates_total && !IsStopped(); ++i) {
      bool _uptrend = _trend[i - 1].Get() != 0.0;
      double _sar_value = _sar[i - 1].Get() + _af[i - 1].Get() * (_ep[i - 1].Get() - _sar[i - 1].Get());
      double _extreme = _ep[i - 1].Get();
      double _acceleration = _af[i - 1].Get();

      if (_uptrend) {
        _sar_value = fmin(_sar_value, PTR_TO_REF(_low)[i - 1].Get());
        if (i > 1) _sar_value = fmin(_sar_value, PTR_TO_REF(_low)[i - 2].Get());

        if (PTR_TO_REF(_low)[i].Get() < _sar_value) {
          _uptrend = false;
          _sar_value = _extreme;
          _extreme = PTR_TO_REF(_low)[i].Get();
          _acceleration = _step;
        } else if (PTR_TO_REF(_high)[i].Get() > _extreme) {
          _extreme = PTR_TO_REF(_high)[i].Get();
          _acceleration = fmin(_acceleration + _step, _max_step);
        }
      } else {
        _sar_value = fmax(_sar_value, PTR_TO_REF(_high)[i - 1].Get());
        if (i > 1) _sar_value = fmax(_sar_value, PTR_TO_REF(_high)[i - 2].Get());

        if (PTR_TO_REF(_high)[i].Get() > _sar_value) {
          _uptrend = true;
          _sar_value = _extreme;
          _extreme = PTR_TO_REF(_high)[i].Get();
          _acceleration = _step;
        } else if (PTR_TO_REF(_low)[i].Get() < _extreme) {
          _extreme = PTR_TO_REF(_low)[i].Get();
          _acceleration = fmin(_acceleration + _step, _max_step);
        }
      }

      _sar[i] = _sar_value;
      _ep[i] = _extreme;
      _af[i] = _acceleration;
      _trend[i] = _uptrend ? 1.0 : 0.0;
    }

    _cache PTR_DEREF SetPrevCalculated(_rates_total);
    return _cache PTR_DEREF GetTailValue<double>(0, _shift);
  }

  /**
   * Returns the indicator's value.
   */
  virtual IndicatorDataEntryValue GetEntryValue(int _mode = 0, int _abs_shift = 0) override {
    double _value = EMPTY_VALUE;
    switch (Get<ENUM_IDATA_SOURCE_TYPE>(STRUCT_ENUM(IndicatorDataParams, IDATA_PARAM_IDSTYPE))) {
      case IDATA_BUILTIN:
        _value = Indi_SAR::iSAR(GetSymbol(), GetTf(), GetStep(), GetMax(), ToRelShift(_abs_shift), THIS_PTR);
        break;
      case IDATA_INDICATOR:
        _value = Indi_SAR::iSAROnIndicator(THIS_PTR, GetCandle(), GetStep(), GetMax(), ToRelShift(_abs_shift));
        break;
      case IDATA_ICUSTOM:
        _value = iCustom(istate.handle, GetSymbol(), GetTf(), iparams.GetCustomIndicatorName(), /*[*/ GetStep(),
                         GetMax() /*]*/, _mode, ToRelShift(_abs_shift));
        break;
      default:
        SetUserError(ERR_INVALID_PARAMETER);
    }
    return _value;
  }

  /* Getters */

  /**
   * Get step of price increment.
   */
  double GetStep() { return iparams.step; }

  /**
   * Get the maximum step.
   */
  double GetMax() { return iparams.max; }

  /* Setters */

  /**
   * Set step of price increment (usually 0.02).
   */
  void SetStep(double _step) {
    istate.is_changed = true;
    iparams.step = _step;
  }

  /**
   * Set the maximum step (usually 0.2).
   */
  void SetMax(double _max) {
    istate.is_changed = true;
    iparams.max = _max;
  }
};

#undef INDI_SAR_DEFAULT_SOURCE_MODE

#ifndef __MQL4__
// Defines global functions (for MQL4 backward compability).
double iSAR(string _symbol, int _tf, double _step, double _max, int _shift) {
  ResetLastError();
  return Indi_SAR::iSAR(_symbol, (ENUM_TIMEFRAMES)_tf, _step, _max, _shift);
}
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>

EMSCRIPTEN_BINDINGS(Indi_SAR_Params) {
  emscripten::value_object<IndiSARParams>("indicators.SARParams")
      .field("step", &IndiSARParams::step)
      .field("max", &IndiSARParams::max)
      .field("shift", &IndiSARParams::shift);
}

EMSCRIPTEN_BINDINGS(Indi_SARBase) {
  emscripten::class_<Indicator<IndiSARParams>, emscripten::base<IndicatorData>>("Indi_SARBase");
}

EMSCRIPTEN_BINDINGS(Indi_SAR) {
  emscripten::class_<Indi_SAR, emscripten::base<Indicator<IndiSARParams>>>("indicators.SAR")
      .smart_ptr<Ref<Indi_SAR>>("Ref<Indi_SAR>")
      .constructor(&make_ref<Indi_SAR, IndiSARParams &>)
      .constructor(&make_ref<Indi_SAR, IndiSARParams &, ENUM_IDATA_SOURCE_TYPE>)
      .constructor(&make_ref<Indi_SAR, IndiSARParams &, ENUM_IDATA_SOURCE_TYPE, IndicatorData *>,
                   emscripten::allow_raw_pointer<emscripten::arg<2>>())
      .constructor(&make_ref<Indi_SAR, IndiSARParams &, ENUM_IDATA_SOURCE_TYPE, IndicatorData *, int>,
                   emscripten::allow_raw_pointer<emscripten::arg<2>>());
}

#endif
