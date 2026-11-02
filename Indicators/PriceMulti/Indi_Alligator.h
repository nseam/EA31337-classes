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
#include "../Price/Indi_MA.h"

#ifndef __MQLBUILD__
// Defines.
// Indicator constants.
// Identifiers of indicator lines permissible when copying values of iAlligator().
#define GATORJAW_LINE 0    // Jaw line.
#define GATORTEETH_LINE 1  // Teeth line.
#define GATORLIPS_LINE 2   // Lips line.
#endif

// Enums.
// Indicator line identifiers used in Gator and Alligator indicators.
// @docs
// - https://www.mql5.com/en/docs/constants/indicatorconstants/lines
enum ENUM_ALLIGATOR_LINE {
#ifdef __MQL4__
  LINE_JAW = MODE_GATORJAW,      // Jaw line.
  LINE_TEETH = MODE_GATORTEETH,  // Teeth line.
  LINE_LIPS = MODE_GATORLIPS,    // Lips line.
#else
  LINE_JAW = GATORJAW_LINE,      // Jaw line.
  LINE_TEETH = GATORTEETH_LINE,  // Teeth line.
  LINE_LIPS = GATORLIPS_LINE,    // Lips line.
#endif
  FINAL_ALLIGATOR_LINE_ENTRY,
};

// Structs.
struct IndiAlligatorParams : IndicatorParams {
  int jaw_period;                    // Jaw line averaging period.
  int jaw_shift;                     // Jaw line shift.
  int teeth_period;                  // Teeth line averaging period.
  int teeth_shift;                   // Teeth line shift.
  int lips_period;                   // Lips line averaging period.
  int lips_shift;                    // Lips line shift.
  ENUM_MA_METHOD ma_method;          // Averaging method.
  ENUM_APPLIED_PRICE applied_price;  // Applied price.
  // Struct constructors.
  IndiAlligatorParams(int _jp = 13, int _js = 8, int _tp = 8, int _ts = 5, int _lp = 5, int _ls = 3,
                      ENUM_MA_METHOD _mm = MODE_SMMA, ENUM_APPLIED_PRICE _ap = PRICE_MEDIAN, int _shift = 0)
      : IndicatorParams(INDI_ALLIGATOR),
        jaw_period(_jp),
        jaw_shift(_js),
        teeth_period(_tp),
        teeth_shift(_ts),
        lips_period(_lp),
        lips_shift(_ls),
        ma_method(_mm),
        applied_price(_ap) {
    shift = _shift;
    SetCustomIndicatorName("Examples\\Alligator");
  };
  IndiAlligatorParams(IndiAlligatorParams &_params) { THIS_REF = _params; };
};

/**
 * Implements the Alligator indicator.
 */
#ifdef __MQL__
#define INDI_ALLIGATOR_DEFAULT_SOURCE_MODE IDATA_BUILTIN
#else
#define INDI_ALLIGATOR_DEFAULT_SOURCE_MODE IDATA_INDICATOR
#endif

class Indi_Alligator : public Indicator<IndiAlligatorParams> {
 public:
  /**
   * Class constructor.
   */
  Indi_Alligator(IndiAlligatorParams &_p, ENUM_IDATA_SOURCE_TYPE _idstype = INDI_ALLIGATOR_DEFAULT_SOURCE_MODE,
                 IndicatorData *_indi_src = NULL, int _indi_src_mode = 0)
      : Indicator(
            _p, IndicatorDataParams::GetInstance(FINAL_ALLIGATOR_LINE_ENTRY, TYPE_DOUBLE, _idstype, IDATA_RANGE_PRICE),
            _indi_src, _indi_src_mode) {}
  Indi_Alligator(int _shift = 0, ENUM_IDATA_SOURCE_TYPE _idstype = INDI_ALLIGATOR_DEFAULT_SOURCE_MODE,
                 IndicatorData *_indi_src = NULL,
                 int _indi_src_mode = 0)
      : Indicator(
            IndiAlligatorParams(),
            IndicatorDataParams::GetInstance(FINAL_ALLIGATOR_LINE_ENTRY, TYPE_DOUBLE, _idstype, IDATA_RANGE_PRICE),
            _indi_src, _indi_src_mode){};
  /**
   * Returns possible data source types. It is a bit mask of ENUM_INDI_SUITABLE_DS_TYPE.
   */
  unsigned int GetSuitableDataSourceTypes() override { return INDI_SUITABLE_DS_TYPE_AP; }

  /**
   * Returns possible data source modes. It is a bit mask of ENUM_IDATA_SOURCE_TYPE.
   */
  unsigned int GetPossibleDataModes() override { return IDATA_BUILTIN | IDATA_ICUSTOM | IDATA_INDICATOR; }

  /**
   * Returns the indicator value.
   *
   * @param
   * _ma_method ENUM_MA_METHOD
   * - MT4/MT5: MODE_SMA, MODE_EMA, MODE_SMMA, MODE_LWMA
   * _applied_price ENUM_APPLIED_PRICE
   * - MT4: MODE_SMA, MODE_EMA, MODE_SMMA, MODE_LWMA
   * - MT5: PRICE_CLOSE, PRICE_OPEN, PRICE_HIGH, PRICE_LOW, PRICE_MEDIAN, PRICE_TYPICAL, PRICE_WEIGHTED
   * _mode ENUM_ALLIGATOR_LINE
   * - EA: LINE_JAW, LINE_TEETH, LINE_LIPS
   * - MT4: 1 - MODE_GATORJAW, 2 - MODE_GATORTEETH, 3 - MODE_GATORLIPS
   * - MT5: 0 - GATORJAW_LINE, 1 - GATORTEETH_LINE, 2 - GATORLIPS_LINE
   *
   * @docs
   * - https://docs.mql4.com/indicators/ialligator
   * - https://www.mql5.com/en/docs/indicators/ialligator
   */
  static double iAlligator(string _symbol, ENUM_TIMEFRAMES _tf, int _jaw_period, int _jaw_shift, int _teeth_period,
                           int _teeth_shift, int _lips_period, int _lips_shift, ENUM_MA_METHOD _ma_method,
                           ENUM_APPLIED_PRICE _applied_price, ENUM_ALLIGATOR_LINE _mode, int _shift = 0,
                           IndicatorData *_obj = NULL) {
#ifdef __MQL__
#ifdef __MQL4__
    return ::iAlligator(_symbol, _tf, _jaw_period, _jaw_shift, _teeth_period, _teeth_shift, _lips_period, _lips_shift,
                        _ma_method, _applied_price, _mode, _shift);
#else  // __MQL5__
    INDICATOR_BUILTIN_CALL_AND_RETURN(::iAlligator(_symbol, _tf, _jaw_period, _jaw_shift, _teeth_period, _teeth_shift,
                                                   _lips_period, _lips_shift, _ma_method, _applied_price),
                                      _mode, _shift);
#endif
#else  // Non-MQL.
    if (_obj != NULL && _obj PTR_DEREF HasDataSource()) {
      return iAlligatorOnIndicator((Indi_Alligator *)_obj, _obj PTR_DEREF GetDataSource(), _jaw_period, _jaw_shift,
                                  _teeth_period, _teeth_shift, _lips_period, _lips_shift, _ma_method,
                                  _applied_price, _mode, _shift);
    }
    RUNTIME_ERROR("Alligator requires an applied-price data source.");
    return EMPTY_VALUE;
#endif
  }

  /** Calculates one Alligator line on its applied-price source using an independent MA cache. */
  static double iAlligatorOnIndicator(Indi_Alligator *_target, IndicatorData *_source, int _jaw_period,
                                      int _jaw_shift, int _teeth_period, int _teeth_shift, int _lips_period,
                                      int _lips_shift, ENUM_MA_METHOD _ma_method, ENUM_APPLIED_PRICE _applied_price,
                                      ENUM_ALLIGATOR_LINE _mode, int _shift) {
    int _period;
    int _line_shift;
    switch (_mode) {
      case LINE_JAW:
        _period = _jaw_period;
        _line_shift = _jaw_shift;
        break;
      case LINE_TEETH:
        _period = _teeth_period;
        _line_shift = _teeth_shift;
        break;
      case LINE_LIPS:
        _period = _lips_period;
        _line_shift = _lips_shift;
        break;
      default:
        return EMPTY_VALUE;
    }
    if (_period <= 0 || _line_shift < 0 || _shift < 0) return EMPTY_VALUE;

    ValueStorage<double> *_price = (ValueStorage<double> *)_source PTR_DEREF
        GetSpecificAppliedPriceValueStorage(_applied_price, _target);
    if (_price == NULL) return EMPTY_VALUE;
    const int _total = _price PTR_DEREF Size();
    if (_shift + _line_shift + _period > _total) return EMPTY_VALUE;

    IndiBufferCache<double> *_cache = _target PTR_DEREF GetCache() PTR_DEREF GetSubCache((int)_mode);
    return Indi_MA::iMAOnArray(PTR_TO_REF(_price), 0, _period, _line_shift, _ma_method, _shift, _cache);
  }

  /**
   * Returns the indicator's value.
   */
  virtual IndicatorDataEntryValue GetEntryValue(int _mode, int _abs_shift = 0) override {
    double _value = EMPTY_VALUE;
#ifdef __MQL4__
    if (_mode == 0) {
      // In MQL4 mode 0 should be treated as mode 1 as Alligator buffers starts from index 1.
      return GetEntryValue((ENUM_ALLIGATOR_LINE)1, ToRelShift(_abs_shift));
    }
#endif
    switch (Get<ENUM_IDATA_SOURCE_TYPE>(STRUCT_ENUM(IndicatorDataParams, IDATA_PARAM_IDSTYPE))) {
      case IDATA_BUILTIN:
        _value =
            Indi_Alligator::iAlligator(GetSymbol(), GetTf(), GetJawPeriod(), GetJawShift(), GetTeethPeriod(),
                                       GetTeethShift(), GetLipsPeriod(), GetLipsShift(), GetMAMethod(),
                                       GetAppliedPrice(), (ENUM_ALLIGATOR_LINE)_mode, ToRelShift(_abs_shift), THIS_PTR);
        break;
      case IDATA_INDICATOR:
        _value = Indi_Alligator::iAlligatorOnIndicator(
            THIS_PTR, GetDataSource(), GetJawPeriod(), GetJawShift(), GetTeethPeriod(), GetTeethShift(),
            GetLipsPeriod(), GetLipsShift(), GetMAMethod(), GetAppliedPrice(), (ENUM_ALLIGATOR_LINE)_mode,
            ToRelShift(_abs_shift));
        break;
      case IDATA_ICUSTOM:
        _value = iCustom(istate.handle, GetSymbol(), GetTf(), iparams.GetCustomIndicatorName(), /*[*/
                         GetJawPeriod(), GetJawShift(), GetTeethPeriod(), GetTeethShift(), GetLipsPeriod(),
                         GetLipsShift(), GetMAMethod(),
                         GetAppliedPrice()
                         /*]*/,
                         _mode, ToRelShift(_abs_shift));
        break;
      default:
        SetUserError(ERR_INVALID_PARAMETER);
    }
    return _value;
  }

  /**
   * Checks if indicator entry values are valid.
   */
  virtual bool IsValidEntry(IndicatorDataEntry &_entry) override {
    return !_entry.HasValue<double>(NULL) && !_entry.HasValue<double>(EMPTY_VALUE) && _entry.IsGt<double>(0);
  }

  /* Class getters */

  /**
   * Get jaw period value.
   */
  int GetJawPeriod() { return iparams.jaw_period; }

  /**
   * Get jaw shift value.
   */
  int GetJawShift() { return iparams.jaw_shift; }

  /**
   * Get teeth period value.
   */
  int GetTeethPeriod() { return iparams.teeth_period; }

  /**
   * Get teeth shift value.
   */
  int GetTeethShift() { return iparams.teeth_shift; }

  /**
   * Get lips period value.
   */
  int GetLipsPeriod() { return iparams.lips_period; }

  /**
   * Get lips shift value.
   */
  int GetLipsShift() { return iparams.lips_shift; }

  /**
   * Get MA method.
   */
  ENUM_MA_METHOD GetMAMethod() { return iparams.ma_method; }

  /**
   * Get applied price value.
   */
  ENUM_APPLIED_PRICE GetAppliedPrice() override { return iparams.applied_price; }

  /* Class setters */

  /**
   * Set jaw period value.
   */
  void SetJawPeriod(int _jaw_period) {
    istate.is_changed = true;
    iparams.jaw_period = _jaw_period;
  }

  /**
   * Set jaw shift value.
   */
  void SetJawShift(int _jaw_shift) {
    istate.is_changed = true;
    iparams.jaw_shift = _jaw_shift;
  }

  /**
   * Set teeth period value.
   */
  void SetTeethPeriod(int _teeth_period) {
    istate.is_changed = true;
    iparams.teeth_period = _teeth_period;
  }

  /**
   * Set teeth shift value.
   */
  void SetTeethShift(int _teeth_shift) {
    istate.is_changed = true;
    iparams.teeth_period = _teeth_shift;
  }

  /**
   * Set lips period value.
   */
  void SetLipsPeriod(int _lips_period) {
    istate.is_changed = true;
    iparams.lips_period = _lips_period;
  }

  /**
   * Set lips shift value.
   */
  void SetLipsShift(int _lips_shift) {
    istate.is_changed = true;
    iparams.lips_period = _lips_shift;
  }

  /**
   * Set MA method.
   */
  void SetMAMethod(ENUM_MA_METHOD _ma_method) {
    istate.is_changed = true;
    iparams.ma_method = _ma_method;
  }

  /**
   * Set applied price value.
   */
  void SetAppliedPrice(ENUM_APPLIED_PRICE _applied_price) {
    istate.is_changed = true;
    iparams.applied_price = _applied_price;
  }
};

#undef INDI_ALLIGATOR_DEFAULT_SOURCE_MODE

#ifndef __MQL4__
// Defines global functions (for MQL4 backward compability).
double iAlligator(string _symbol, int _tf, int _jp, int _js, int _tp, int _ts, int _lp, int _ls, int _ma_method,
                  int _ap, int _mode, int _shift) {
  ResetLastError();
  return Indi_Alligator::iAlligator(_symbol, (ENUM_TIMEFRAMES)_tf, _jp, _js, _tp, _ts, _lp, _ls,
                                    (ENUM_MA_METHOD)_ma_method, (ENUM_APPLIED_PRICE)_ap, (ENUM_ALLIGATOR_LINE)_mode,
                                    _shift);
}
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>

EMSCRIPTEN_BINDINGS(Indi_Alligator_Params) {
  emscripten::value_object<IndiAlligatorParams>("indicators.AlligatorParams")
      .field("jaw_period", &IndiAlligatorParams::jaw_period)
      .field("jaw_shift", &IndiAlligatorParams::jaw_shift)
      .field("teeth_period", &IndiAlligatorParams::teeth_period)
      .field("teeth_shift", &IndiAlligatorParams::teeth_shift)
      .field("lips_period", &IndiAlligatorParams::lips_period)
      .field("lips_shift", &IndiAlligatorParams::lips_shift)
      .field("ma_method", &IndiAlligatorParams::ma_method)
      .field("applied_price", &IndiAlligatorParams::applied_price)
      .field("shift", &IndiAlligatorParams::shift);
}

EMSCRIPTEN_BINDINGS(Indi_AlligatorBase) {
  emscripten::class_<Indicator<IndiAlligatorParams>, emscripten::base<IndicatorData>>("Indi_AlligatorBase");
}

EMSCRIPTEN_BINDINGS(Indi_Alligator) {
  emscripten::class_<Indi_Alligator, emscripten::base<Indicator<IndiAlligatorParams>>>("indicators.Alligator")
      .smart_ptr<Ref<Indi_Alligator>>("Ref<Indi_Alligator>")
      .constructor(&make_ref<Indi_Alligator, IndiAlligatorParams &>)
      .constructor(&make_ref<Indi_Alligator, IndiAlligatorParams &, ENUM_IDATA_SOURCE_TYPE>)
      .constructor(&make_ref<Indi_Alligator, IndiAlligatorParams &, ENUM_IDATA_SOURCE_TYPE, IndicatorData *>,
                   emscripten::allow_raw_pointer<emscripten::arg<2>>())
      .constructor(&make_ref<Indi_Alligator, IndiAlligatorParams &, ENUM_IDATA_SOURCE_TYPE, IndicatorData *, int>,
                   emscripten::allow_raw_pointer<emscripten::arg<2>>());
}

#endif
