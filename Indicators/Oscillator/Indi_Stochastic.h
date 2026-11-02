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
 * Implements Stochastic indicator.
 */

#ifndef __MQL__
// Allows the preprocessor to include a header file when it is needed.
#pragma once
#endif

// Includes.
#include "../../Indicator/Indicator.h"
#include "../Price/Indi_MA.h"

#ifndef __MQL__
// Enums.
// @see: https://www.mql5.com/en/docs/constants/indicatorconstants/prices
enum ENUM_STO_PRICE {
  STO_LOWHIGH = 0,  // Calculation is based on Low/High prices.
  STO_CLOSECLOSE,   // Calculation is based on Close/Close prices.
};
#endif

// Structs.
struct IndiStochParams : IndicatorParams {
  int kperiod;
  int dperiod;
  int slowing;
  ENUM_MA_METHOD ma_method;
  ENUM_STO_PRICE price_field;
  // Struct constructors.
  IndiStochParams(int _kperiod = 5, int _dperiod = 3, int _slowing = 3, ENUM_MA_METHOD _ma_method = MODE_SMA,
                  ENUM_STO_PRICE _pf = STO_LOWHIGH, int _shift = 0)
      : IndicatorParams(INDI_STOCHASTIC),
        kperiod(_kperiod),
        dperiod(_dperiod),
        slowing(_slowing),
        ma_method(_ma_method),
        price_field(_pf) {
    shift = _shift;
    SetCustomIndicatorName("Examples\\Stochastic");
  };
  IndiStochParams(IndiStochParams &_params) { THIS_REF = _params; };
};

/**
 * Implements the Stochastic Oscillator.
 */
#ifdef __MQL__
#define INDI_STOCHASTIC_DEFAULT_SOURCE_MODE IDATA_BUILTIN
#else
#define INDI_STOCHASTIC_DEFAULT_SOURCE_MODE IDATA_INDICATOR
#endif

class Indi_Stochastic : public Indicator<IndiStochParams> {
 public:
  /**
   * Class constructor.
   */
  Indi_Stochastic(IndiStochParams &_p, ENUM_IDATA_SOURCE_TYPE _idstype = INDI_STOCHASTIC_DEFAULT_SOURCE_MODE,
                  IndicatorData *_indi_src = NULL,
                  int _indi_src_mode = 0)
      : Indicator(_p,
                  IndicatorDataParams::GetInstance(FINAL_SIGNAL_LINE_ENTRY, TYPE_DOUBLE, _idstype, IDATA_RANGE_RANGE,
                                                   _indi_src_mode),
                  _indi_src) {}
  Indi_Stochastic(int _shift = 0, ENUM_IDATA_SOURCE_TYPE _idstype = INDI_STOCHASTIC_DEFAULT_SOURCE_MODE,
                  IndicatorData *_indi_src = NULL,
                  int _indi_src_mode = 0)
      : Indicator(IndiStochParams(),
                  IndicatorDataParams::GetInstance(FINAL_SIGNAL_LINE_ENTRY, TYPE_DOUBLE, _idstype, IDATA_RANGE_RANGE,
                                                   _indi_src_mode),
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
   * Calculates the Stochastic Oscillator and returns its value.
   *
   * @docs
   * - https://docs.mql4.com/indicators/istochastic
   * - https://www.mql5.com/en/docs/indicators/istochastic
   */
  static double iStochastic(
      string _symbol, ENUM_TIMEFRAMES _tf, unsigned int _kperiod, unsigned int _dperiod, unsigned int _slowing,
      ENUM_MA_METHOD _ma_method,    // (MT4/MT5): MODE_SMA, MODE_EMA, MODE_SMMA, MODE_LWMA
      ENUM_STO_PRICE _price_field,  // (MT4 _price_field):      0      - Low/High,       1        - Close/Close
                                    // (MT5 _price_field): STO_LOWHIGH - Low/High, STO_CLOSECLOSE - Close/Close
      int _mode,                    // (MT4): 0 - MODE_MAIN/MAIN_LINE, 1 - MODE_SIGNAL/SIGNAL_LINE
      int _shift = 0, IndicatorData *_obj = NULL) {
#ifdef __MQL__
#ifdef __MQL4__
    return ::iStochastic(_symbol, _tf, _kperiod, _dperiod, _slowing, _ma_method, _price_field, _mode, _shift);
#else  // __MQL5__
    INDICATOR_BUILTIN_CALL_AND_RETURN(
        ::iStochastic(_symbol, _tf, _kperiod, _dperiod, _slowing, _ma_method, _price_field), _mode, _shift);
#endif
#else  // Non-MQL.
    if (_obj != NULL && _obj PTR_DEREF HasDataSource()) {
      return iStochasticOnIndicator((Indi_Stochastic *)_obj, _obj PTR_DEREF GetCandle(), _kperiod, _dperiod,
                                    _slowing, _ma_method, _price_field, (ENUM_SIGNAL_LINE)_mode, _shift);
    }
    RUNTIME_ERROR("Stochastic requires a candle data source.");
    return EMPTY_VALUE;
#endif
  }

  /**
   * Calculates Stochastic on OHLC arrays and caches raw %K, smoothed %K, and %D.
   */
  static double iStochasticOnArray(int rates_total, int prev_calculated, ValueStorage<double> &_open,
                                   ValueStorage<double> &_high, ValueStorage<double> &_low,
                                   ValueStorage<double> &_close, int _kperiod, int _dperiod, int _slowing,
                                   ENUM_MA_METHOD _ma_method, ENUM_STO_PRICE _price_field, ENUM_SIGNAL_LINE _mode,
                                   int _shift, IndiBufferCache<double> *_cache) {
    if (_kperiod <= 0 || _dperiod <= 0 || _slowing <= 0 || rates_total <= 0) return EMPTY_VALUE;

    _cache PTR_DEREF SetPriceBuffer(_open, _high, _low, _close, rates_total);
    if (!_cache PTR_DEREF HasBuffers()) {
      _cache PTR_DEREF AddBuffer<NativeValueStorage<double>>(3);
    }

    ValueStorage<double> &_raw_k = _cache PTR_DEREF GetBuffer<double>(0);
    ValueStorage<double> &_main = _cache PTR_DEREF GetBuffer<double>(1);
    ValueStorage<double> &_signal = _cache PTR_DEREF GetBuffer<double>(2);
    const int _begin = _kperiod - 1;
    const int _raw_start = prev_calculated == 0 || prev_calculated - 1 < _begin ? _begin : prev_calculated - 1;

    for (int i = _raw_start; i < rates_total && !IsStopped(); ++i) {
      double _highest = -DBL_MAX;
      double _lowest = DBL_MAX;
      const int _window_start = i - _kperiod + 1;
      for (int j = _window_start; j <= i; ++j) {
        const double _high_value = _price_field == STO_LOWHIGH ? _high[j].Get() : _close[j].Get();
        const double _low_value = _price_field == STO_LOWHIGH ? _low[j].Get() : _close[j].Get();
        _highest = fmax(_highest, _high_value);
        _lowest = fmin(_lowest, _low_value);
      }
      const double _close_value = _close[i].Get();
      _raw_k[i] = _highest == _lowest ? 0.0 : 100.0 * (_close_value - _lowest) / (_highest - _lowest);
    }

    const int _prev = _cache PTR_DEREF GetPrevCalculated();
    Indi_MA::Calculate(rates_total, _prev, _begin, _raw_k, _main, _ma_method, _slowing);
    const int _main_begin = _begin + _slowing - 1;
    Indi_MA::Calculate(rates_total, _prev, _main_begin, _main, _signal, _ma_method, _dperiod);
    _cache PTR_DEREF SetPrevCalculated(rates_total);

    if (_mode == LINE_MAIN) return _cache PTR_DEREF GetTailValue<double>(1, _shift);
    if (_mode == LINE_SIGNAL) return _cache PTR_DEREF GetTailValue<double>(2, _shift);
    return EMPTY_VALUE;
  }

  /** Calculates Stochastic using cached OHLC buffers from the source candle. */
  static double iStochasticOnIndicator(Indi_Stochastic *_target, IndicatorData *_source, int _kperiod, int _dperiod,
                                       int _slowing, ENUM_MA_METHOD _ma_method, ENUM_STO_PRICE _price_field,
                                       ENUM_SIGNAL_LINE _mode, int _shift) {
    if (_source == NULL) return EMPTY_VALUE;
    ValueStorage<double> *_open = (ValueStorage<double> *)_source PTR_DEREF
        GetSpecificValueStorage(INDI_DATA_VS_TYPE_PRICE_OPEN);
    ValueStorage<double> *_high = (ValueStorage<double> *)_source PTR_DEREF
        GetSpecificValueStorage(INDI_DATA_VS_TYPE_PRICE_HIGH);
    ValueStorage<double> *_low = (ValueStorage<double> *)_source PTR_DEREF
        GetSpecificValueStorage(INDI_DATA_VS_TYPE_PRICE_LOW);
    ValueStorage<double> *_close = (ValueStorage<double> *)_source PTR_DEREF
        GetSpecificValueStorage(INDI_DATA_VS_TYPE_PRICE_CLOSE);
    IndiBufferCache<double> *_cache = _target PTR_DEREF GetCache();

    return iStochasticOnArray(_open PTR_DEREF Size(), _cache PTR_DEREF GetPrevCalculated(), PTR_TO_REF(_open),
                              PTR_TO_REF(_high), PTR_TO_REF(_low), PTR_TO_REF(_close), _kperiod, _dperiod, _slowing,
                              _ma_method, _price_field, _mode, _shift, _cache);
  }

  /**
   * Returns the indicator's value.
   */
  virtual IndicatorDataEntryValue GetEntryValue(int _mode = LINE_MAIN, int _abs_shift = 0) override {
    double _value = EMPTY_VALUE;
    switch (Get<ENUM_IDATA_SOURCE_TYPE>(STRUCT_ENUM(IndicatorDataParams, IDATA_PARAM_IDSTYPE))) {
      case IDATA_BUILTIN:
        _value = Indi_Stochastic::iStochastic(GetSymbol(), GetTf(), GetKPeriod(), GetDPeriod(), GetSlowing(),
                                              GetMAMethod(), GetPriceField(), _mode, ToRelShift(_abs_shift), THIS_PTR);
        break;
      case IDATA_INDICATOR:
        _value = Indi_Stochastic::iStochasticOnIndicator(THIS_PTR, GetCandle(), GetKPeriod(), GetDPeriod(),
                                                        GetSlowing(), GetMAMethod(), GetPriceField(),
                                                        (ENUM_SIGNAL_LINE)_mode, ToRelShift(_abs_shift));
        break;
      case IDATA_ICUSTOM:
        _value = iCustom(istate.handle, GetSymbol(), GetTf(), iparams.GetCustomIndicatorName(), /*[*/ GetKPeriod(),
                         GetDPeriod(), GetSlowing() /*]*/, _mode, ToRelShift(_abs_shift));
        break;
      default:
        SetUserError(ERR_INVALID_PARAMETER);
    }
    return _value;
  }

  /**
   * Checks if indicator entry values are valid.
   */
  virtual bool IsValidEntry(IndicatorDataEntry &_entry) override { return _entry.IsWithinRange<double>(0, 101); }

  /* Getters */

  /**
   * Get period of the %K line.
   */
  int GetKPeriod() { return iparams.kperiod; }

  /**
   * Get period of the %D line.
   */
  int GetDPeriod() { return iparams.dperiod; }

  /**
   * Get slowing value.
   */
  int GetSlowing() { return iparams.slowing; }

  /**
   * Set MA method.
   */
  ENUM_MA_METHOD GetMAMethod() { return iparams.ma_method; }

  /**
   * Get price field parameter.
   */
  ENUM_STO_PRICE GetPriceField() { return iparams.price_field; }

  /* Setters */

  /**
   * Set period of the %K line.
   */
  void SetKPeriod(int _kperiod) {
    istate.is_changed = true;
    iparams.kperiod = _kperiod;
  }

  /**
   * Set period of the %D line.
   */
  void SetDPeriod(int _dperiod) {
    istate.is_changed = true;
    iparams.dperiod = _dperiod;
  }

  /**
   * Set slowing value.
   */
  void SetSlowing(int _slowing) {
    istate.is_changed = true;
    iparams.slowing = _slowing;
  }

  /**
   * Set MA method.
   */
  void SetMAMethod(ENUM_MA_METHOD _ma_method) {
    istate.is_changed = true;
    iparams.ma_method = _ma_method;
  }

  /**
   * Set price field parameter.
   */
  void SetPriceField(ENUM_STO_PRICE _price_field) {
    istate.is_changed = true;
    iparams.price_field = _price_field;
  }
};

#undef INDI_STOCHASTIC_DEFAULT_SOURCE_MODE

#ifndef __MQL4__
// Defines global functions (for MQL4 backward compability).
double iStochastic(string _symbol, int _tf, int _kperiod, int _dperiod, int _slowing, int _ma_method, int _pf,
                   int _mode, int _shift) {
  ResetLastError();
  return Indi_Stochastic::iStochastic(_symbol, (ENUM_TIMEFRAMES)_tf, _kperiod, _dperiod, _slowing,
                                      (ENUM_MA_METHOD)_ma_method, (ENUM_STO_PRICE)_pf, _mode, _shift);
}
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>

EMSCRIPTEN_BINDINGS(ENUM_STO_PRICE) {
  emscripten::enum_<ENUM_STO_PRICE>("sto_prices")
      .value("LOWHIGH", STO_LOWHIGH)
      .value("CLOSECLOSE", STO_CLOSECLOSE);
}

EMSCRIPTEN_BINDINGS(Indi_Stochastic_Params) {
  emscripten::value_object<IndiStochParams>("indicators.StochasticParams")
      .field("kperiod", &IndiStochParams::kperiod)
      .field("dperiod", &IndiStochParams::dperiod)
      .field("slowing", &IndiStochParams::slowing)
      .field("ma_method", &IndiStochParams::ma_method)
      .field("price_field", &IndiStochParams::price_field)
      .field("shift", &IndiStochParams::shift);
}

EMSCRIPTEN_BINDINGS(Indi_StochasticBase) {
  emscripten::class_<Indicator<IndiStochParams>, emscripten::base<IndicatorData>>("Indi_StochasticBase");
}

EMSCRIPTEN_BINDINGS(Indi_Stochastic) {
  emscripten::class_<Indi_Stochastic, emscripten::base<Indicator<IndiStochParams>>>("indicators.Stochastic")
      .smart_ptr<Ref<Indi_Stochastic>>("Ref<Indi_Stochastic>")
      .constructor(&make_ref<Indi_Stochastic, IndiStochParams &>)
      .constructor(&make_ref<Indi_Stochastic, IndiStochParams &, ENUM_IDATA_SOURCE_TYPE>)
      .constructor(&make_ref<Indi_Stochastic, IndiStochParams &, ENUM_IDATA_SOURCE_TYPE, IndicatorData *>,
                   emscripten::allow_raw_pointer<emscripten::arg<2>>())
      .constructor(&make_ref<Indi_Stochastic, IndiStochParams &, ENUM_IDATA_SOURCE_TYPE, IndicatorData *, int>,
                   emscripten::allow_raw_pointer<emscripten::arg<2>>());
}

#endif
