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
 * Implements Moving Average Convergence Divergence (MACD) indicator.
 */

#ifndef __MQL__
// Allows the preprocessor to include a header file when it is needed.
#pragma once
#endif

// Includes.
#include "../../Indicator/Indicator.h"
#include "../Price/Indi_MA.h"

// Structs.
struct IndiMACDParams : IndicatorParams {
  unsigned int ema_fast_period;
  unsigned int ema_slow_period;
  unsigned int signal_period;
  ENUM_APPLIED_PRICE applied_price;
  // Struct constructors.
  IndiMACDParams(unsigned int _efp = 12, unsigned int _esp = 26, unsigned int _sp = 9,
                 ENUM_APPLIED_PRICE _ap = PRICE_CLOSE, int _shift = 0)
      : IndicatorParams(INDI_MACD),
        ema_fast_period(_efp),
        ema_slow_period(_esp),
        signal_period(_sp),
        applied_price(_ap) {
    shift = _shift;
    SetCustomIndicatorName("Examples\\MACD");
  };
  IndiMACDParams(IndiMACDParams &_params) { THIS_REF = _params; };
};

/**
 * Implements the Moving Averages Convergence/Divergence indicator.
 */
#ifdef __MQL__
#define INDI_MACD_DEFAULT_SOURCE_MODE IDATA_BUILTIN
#else
#define INDI_MACD_DEFAULT_SOURCE_MODE IDATA_INDICATOR
#endif

class Indi_MACD : public Indicator<IndiMACDParams> {
 public:
  /**
   * Class constructor.
   */
  Indi_MACD(IndiMACDParams &_p, ENUM_IDATA_SOURCE_TYPE _idstype = INDI_MACD_DEFAULT_SOURCE_MODE, IndicatorData *_indi_src = NULL,
            int _indi_src_mode = 0)
      : Indicator(_p,
                  IndicatorDataParams::GetInstance(FINAL_SIGNAL_LINE_ENTRY, TYPE_DOUBLE, _idstype, IDATA_RANGE_RANGE,
                                                   _indi_src_mode),
                  _indi_src) {}
  Indi_MACD(int _shift = 0, ENUM_IDATA_SOURCE_TYPE _idstype = INDI_MACD_DEFAULT_SOURCE_MODE, IndicatorData *_indi_src = NULL,
            int _indi_src_mode = 0)
      : Indicator(IndiMACDParams(),
                  IndicatorDataParams::GetInstance(FINAL_SIGNAL_LINE_ENTRY, TYPE_DOUBLE, _idstype, IDATA_RANGE_RANGE,
                                                   _indi_src_mode),
                  _indi_src) {}
  /**
   * Returns possible data source types. It is a bit mask of ENUM_INDI_SUITABLE_DS_TYPE.
   */
  unsigned int GetSuitableDataSourceTypes() override { return INDI_SUITABLE_DS_TYPE_AP; }

  /**
   * Returns possible data source modes. It is a bit mask of ENUM_IDATA_SOURCE_TYPE.
   */
  unsigned int GetPossibleDataModes() override { return IDATA_BUILTIN | IDATA_ICUSTOM; }

  /**
   * Returns the indicator value.
   *
   * @docs
   * - https://docs.mql4.com/indicators/imacd
   * - https://www.mql5.com/en/docs/indicators/imacd
   */
  static double iMACD(
      string _symbol, ENUM_TIMEFRAMES _tf, unsigned int _ema_fast_period, unsigned int _ema_slow_period,
      unsigned int _signal_period, ENUM_APPLIED_PRICE _applied_price,
      ENUM_SIGNAL_LINE _mode = LINE_MAIN,  // (MT4/MT5 _mode): 0 - MODE_MAIN/MAIN_LINE, 1 - MODE_SIGNAL/SIGNAL_LINE
      int _shift = 0, IndicatorData *_obj = NULL) {
#ifdef __MQL__
#ifdef __MQL4__
    return ::iMACD(_symbol, _tf, _ema_fast_period, _ema_slow_period, _signal_period, _applied_price, _mode, _shift);
#else  // __MQL5__
    INDICATOR_BUILTIN_CALL_AND_RETURN(
        ::iMACD(_symbol, _tf, _ema_fast_period, _ema_slow_period, _signal_period, _applied_price), _mode, _shift);
#endif
#else  // Non-MQL.
    if (_obj != NULL && _obj PTR_DEREF HasDataSource()) {
      IndicatorData *_source = _obj PTR_DEREF GetDataSource();
      auto ema = [_source](unsigned int _period, ENUM_APPLIED_PRICE _ap, int _at_shift) {
        if (_period == 0) return DBL_MAX;

        const int _seed_shift = _source PTR_DEREF GetBars() - 1;
        if (_seed_shift < _at_shift || _seed_shift - (int)_period + 1 < 0) return DBL_MAX;

        double _sum = 0;
        for (unsigned int i = 0; i < _period; ++i)
          _sum += _source PTR_DEREF GetPrice(_ap, _seed_shift - (int)i);

        double _value = _sum / _period;
        const double _alpha = 2.0 / ((double)_period + 1.0);
        for (int i = _seed_shift - (int)_period; i >= _at_shift; --i)
          _value = _alpha * _source PTR_DEREF GetPrice(_ap, i) + (1.0 - _alpha) * _value;
        return _value;
      };

      auto main_value = [&ema, _ema_fast_period, _ema_slow_period, _applied_price](int _at_shift) {
        return ema(_ema_fast_period, _applied_price, _at_shift) -
               ema(_ema_slow_period, _applied_price, _at_shift);
      };

      if (_mode == LINE_MAIN)
        return main_value(_shift);

      if (_mode == LINE_SIGNAL) {
        if (_signal_period == 0) return DBL_MAX;

        const int _seed_shift = _source PTR_DEREF GetBars() - 1;
        if (_seed_shift < _shift || _seed_shift - (int)_signal_period + 1 < 0) return DBL_MAX;

        double _sum = 0;
        for (unsigned int i = 0; i < _signal_period; ++i)
          _sum += main_value(_seed_shift - (int)i);

        double _value = _sum / _signal_period;
        const double _alpha = 2.0 / ((double)_signal_period + 1.0);
        for (int i = _seed_shift - (int)_signal_period; i >= _shift; --i)
          _value = _alpha * main_value(i) + (1.0 - _alpha) * _value;
        return _value;
      }
    }

    RUNTIME_ERROR("MACD requires an applied-price data source.");
    return DBL_MAX;
#endif
  }

  /**
   * Calculates MACD lines on a price array, caching both EMA inputs and outputs.
   */
  static double iMACDOnArray(const int rates_total, const int prev_calculated, const int begin,
                             ValueStorage<double> &_price, unsigned int _ema_fast_period,
                             unsigned int _ema_slow_period, unsigned int _signal_period, ENUM_SIGNAL_LINE _mode,
                             int _shift, IndiBufferCache<double> *_cache, bool _recalculate = false) {
    if (_ema_fast_period == 0 || _ema_slow_period == 0 || _signal_period == 0 ||
        rates_total < (int)fmax(_ema_fast_period, fmax(_ema_slow_period, _signal_period))) {
      return EMPTY_VALUE;
    }

    _cache PTR_DEREF SetPriceBuffer(_price, rates_total);

    if (!_cache PTR_DEREF HasBuffers()) {
      _cache PTR_DEREF AddBuffer<NativeValueStorage<double>>(4);
    }

    if (_recalculate) {
      _cache PTR_DEREF ResetPrevCalculated();
    }

    const int _prev_calculated = _cache PTR_DEREF GetPrevCalculated();
    ValueStorage<double> &_fast = _cache PTR_DEREF GetBuffer<double>(0);
    ValueStorage<double> &_slow = _cache PTR_DEREF GetBuffer<double>(1);
    ValueStorage<double> &_main = _cache PTR_DEREF GetBuffer<double>(2);
    ValueStorage<double> &_signal = _cache PTR_DEREF GetBuffer<double>(3);

    Indi_MA::CalculateEMA(rates_total, _prev_calculated, begin, _price, _fast, _ema_fast_period);
    Indi_MA::CalculateEMA(rates_total, _prev_calculated, begin, _price, _slow, _ema_slow_period);

    const int _start = _prev_calculated == 0 ? begin : _prev_calculated - 1;
    for (int i = _start; i < rates_total && !IsStopped(); ++i) {
      _main[i] = _fast[i].Get() - _slow[i].Get();
    }

    Indi_MA::CalculateEMA(rates_total, _prev_calculated, begin, _main, _signal, _signal_period);
    _cache PTR_DEREF SetPrevCalculated(rates_total);

    if (_mode == LINE_MAIN) {
      return _cache PTR_DEREF GetTailValue<double>(2, _shift);
    }
    if (_mode == LINE_SIGNAL) {
      return _cache PTR_DEREF GetTailValue<double>(3, _shift);
    }
    return EMPTY_VALUE;
  }

  /**
   * Calculates MACD on another indicator using cached EMA and output buffers.
   */
  static double iMACDOnIndicator(IndicatorData *_target, IndicatorData *_source, ENUM_APPLIED_PRICE _applied_price,
                                 ENUM_SIGNAL_LINE _mode = LINE_MAIN, int _shift = 0) {
    Indi_MACD *_macd = (Indi_MACD *)_target;
    ValueStorage<double> *_price = (ValueStorage<double> *)_source PTR_DEREF
        GetSpecificAppliedPriceValueStorage(_applied_price, _target);
    IndiBufferCache<double> *_cache = _target PTR_DEREF GetCache();

    return iMACDOnArray(_price PTR_DEREF Size(), _cache PTR_DEREF GetPrevCalculated(), 0, PTR_TO_REF(_price),
              _macd PTR_DEREF GetEmaFastPeriod(), _macd PTR_DEREF GetEmaSlowPeriod(),
              _macd PTR_DEREF GetSignalPeriod(), _mode, _shift, _cache);
  }

  /**
   * Returns the indicator's value.
   */
  virtual IndicatorDataEntryValue GetEntryValue(int _mode = LINE_MAIN, int _abs_shift = 0) override {
    double _value = EMPTY_VALUE;
    switch (Get<ENUM_IDATA_SOURCE_TYPE>(STRUCT_ENUM(IndicatorDataParams, IDATA_PARAM_IDSTYPE))) {
      case IDATA_BUILTIN:
        _value = Indi_MACD::iMACD(GetSymbol(), GetTf(), GetEmaFastPeriod(), GetEmaSlowPeriod(), GetSignalPeriod(),
                                  GetAppliedPrice(), (ENUM_SIGNAL_LINE)_mode, ToRelShift(_abs_shift), THIS_PTR);
        break;
      case IDATA_INDICATOR:
        _value = Indi_MACD::iMACDOnIndicator(THIS_PTR, GetDataSource(), GetAppliedPrice(),
                                             (ENUM_SIGNAL_LINE)_mode, ToRelShift(_abs_shift));
        break;
      case IDATA_ICUSTOM:
        _value =
            iCustom(istate.handle, GetSymbol(), GetTf(), iparams.GetCustomIndicatorName(), /*[*/ GetEmaFastPeriod(),
                    GetEmaSlowPeriod(), GetSignalPeriod(), GetAppliedPrice() /*]*/, _mode, ToRelShift(_abs_shift));
        break;
      default:
        SetUserError(ERR_INVALID_PARAMETER);
    }
    return _value;
  }

  /**
   * Checks if indicator entry values are valid.
   */
  virtual bool IsValidEntry(IndicatorDataEntry &_entry) override { return !_entry.HasValue<double>(DBL_MAX); }

  /* Getters */

  /**
   * Get fast EMA period value.
   *
   * Averaging period for the calculation of the moving average.
   */
  unsigned int GetEmaFastPeriod() { return iparams.ema_fast_period; }

  /**
   * Get slow EMA period value.
   *
   * Averaging period for the calculation of the moving average.
   */
  unsigned int GetEmaSlowPeriod() { return iparams.ema_slow_period; }

  /**
   * Get signal period value.
   *
   * Averaging period for the calculation of the moving average.
   */
  unsigned int GetSignalPeriod() { return iparams.signal_period; }

  /**
   * Get applied price value.
   *
   * The desired price base for calculations.
   */
  ENUM_APPLIED_PRICE GetAppliedPrice() override { return iparams.applied_price; }

  /* Setters */

  /**
   * Set fast EMA period value.
   *
   * Averaging period for the calculation of the moving average.
   */
  void SetEmaFastPeriod(unsigned int _ema_fast_period) {
    istate.is_changed = true;
    iparams.ema_fast_period = _ema_fast_period;
  }

  /**
   * Set slow EMA period value.
   *
   * Averaging period for the calculation of the moving average.
   */
  void SetEmaSlowPeriod(unsigned int _ema_slow_period) {
    istate.is_changed = true;
    iparams.ema_slow_period = _ema_slow_period;
  }

  /**
   * Set signal period value.
   *
   * Averaging period for the calculation of the moving average.
   */
  void SetSignalPeriod(unsigned int _signal_period) {
    istate.is_changed = true;
    iparams.signal_period = _signal_period;
  }

  /**
   * Set applied price value.
   *
   * The desired price base for calculations.
   * @docs
   * - https://docs.mql4.com/constants/indicatorconstants/prices#enum_applied_price_enum
   * - https://www.mql5.com/en/docs/constants/indicatorconstants/prices#enum_applied_price_enum
   */
  void SetAppliedPrice(ENUM_APPLIED_PRICE _applied_price) {
    istate.is_changed = true;
    iparams.applied_price = _applied_price;
  }
};

#undef INDI_MACD_DEFAULT_SOURCE_MODE

#ifndef __MQL4__
// Defines global functions (for MQL4 backward compability).
double iMACD(string _symbol, int _tf, int _ema_fp, int _ema_sp, int _signal_period, int _ap, int _mode, int _shift) {
  ResetLastError();
  return Indi_MACD::iMACD(_symbol, (ENUM_TIMEFRAMES)_tf, _ema_fp, _ema_sp, _signal_period, (ENUM_APPLIED_PRICE)_ap,
                          (ENUM_SIGNAL_LINE)_mode, _shift);
}
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>

EMSCRIPTEN_BINDINGS(Indi_MACD_Params) {
  emscripten::value_object<IndiMACDParams>("indicators.MACDParams")
      .field("ema_fast_period", &IndiMACDParams::ema_fast_period)
      .field("ema_slow_period", &IndiMACDParams::ema_slow_period)
      .field("signal_period", &IndiMACDParams::signal_period)
      .field("applied_price", &IndiMACDParams::applied_price)
      .field("shift", &IndiMACDParams::shift);
}

EMSCRIPTEN_BINDINGS(Indi_MACDBase) {
  emscripten::class_<Indicator<IndiMACDParams>, emscripten::base<IndicatorData>>("Indi_MACDBase");
}

EMSCRIPTEN_BINDINGS(Indi_MACD) {
  emscripten::class_<Indi_MACD, emscripten::base<Indicator<IndiMACDParams>>>("indicators.MACD")
      .smart_ptr<Ref<Indi_MACD>>("Ref<Indi_MACD>")
      .constructor(&make_ref<Indi_MACD, IndiMACDParams &>)
      .constructor(&make_ref<Indi_MACD, IndiMACDParams &, ENUM_IDATA_SOURCE_TYPE>)
      .constructor(&make_ref<Indi_MACD, IndiMACDParams &, ENUM_IDATA_SOURCE_TYPE, IndicatorData *>,
                   emscripten::allow_raw_pointer<emscripten::arg<2>>())
      .constructor(&make_ref<Indi_MACD, IndiMACDParams &, ENUM_IDATA_SOURCE_TYPE, IndicatorData *, int>,
                   emscripten::allow_raw_pointer<emscripten::arg<2>>());
}

#endif
