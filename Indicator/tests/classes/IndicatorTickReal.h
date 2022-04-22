//+------------------------------------------------------------------+
//|                                                EA31337 framework |
//|                                 Copyright 2016-2021, EA31337 Ltd |
//|                                       https://github.com/EA31337 |
//+------------------------------------------------------------------+

/*
 *  This file is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * Real tick-based indicator.
 */

#ifndef __MQL__
// Allows the preprocessor to include a header file when it is needed.
#pragma once
#endif

// Includes.
#include "../../../Chart.struct.static.h"
#include "../../IndicatorTick.h"

// Params for real tick-based indicator.
struct IndicatorTickRealParams : IndicatorParams {
  IndicatorTickRealParams() : IndicatorParams(INDI_TICK, 3, TYPE_DOUBLE) {}
};

// Real tick-based indicator.
class IndicatorTickReal : public IndicatorTick<IndicatorTickRealParams, double> {
 public:
  IndicatorTickReal(int _shift = 0, string _name = "") : IndicatorTick(INDI_TICK, _shift, _name) {}

  string GetName() override { return "IndicatorTickReal"; }

  void OnBecomeDataSourceFor(IndicatorBase* _base_indi) override {
    // Feeding base indicator with historic entries of this indicator.
#ifdef __debug__
    Print(GetFullName(), " became a data source for ", _base_indi.GetFullName());
#endif

#ifndef __MQL4__
    int _ticks_to_emit = 1000;

#ifdef __debug_verbose__
    Print(_base_indi.GetFullName(), " will be now filled with ", _ticks_to_emit,
          " historical entries generated by " + GetFullName());
#endif

    static MqlTick _tmp_ticks[];
    ArrayResize(_tmp_ticks, 0);

    int _tries = 10;
    int _num_copied = -1;

    while (_tries-- > 0) {
      _num_copied = CopyTicks(GetSymbol(), _tmp_ticks, COPY_TICKS_ALL);

      if (_num_copied == -1) {
        Sleep(1000);
      } else {
        break;
      }
    }

    // Clearing possible error 4004.
    ResetLastError();

    for (int i = 0; i < _num_copied; ++i) {
      TickAB<double> _tick(_tmp_ticks[i].ask, _tmp_ticks[i].bid);
      // We can't call EmitEntry() here, as tick would go to multiple sources at the same time!
      _base_indi.OnDataSourceEntry(TickToEntry(_tmp_ticks[i].time, _tick));
    }
#endif
  }

  void OnTick() override {
#ifdef __MQL4__
    // Refreshes Ask/Bid constants.
    RefreshRates();
    double _ask = Ask;
    double _bid = Bid;
    long _time = TimeCurrent();
#else
    static MqlTick _tmp_ticks[];
    // Copying only the last tick.
    int _num_copied = CopyTicks(GetSymbol(), _tmp_ticks, COPY_TICKS_INFO, 0, 1);

    if (_num_copied < 1 || _LastError != 0) {
      Print("Error. Cannot copy MT ticks via CopyTicks(). Error " + IntegerToString(_LastError));
      // DebugBreak();
      // Just emitting zeroes in case of error.
      TickAB<double> _tick(0, 0);
      EmitEntry(TickToEntry(TimeCurrent(), _tick));
      return;
    }

#ifdef __debug_verbose__
    Print("TickReal: ", TimeToString(_tmp_ticks[0].time), " = ", _tmp_ticks[0].bid);
#endif

    double _ask = _tmp_ticks[0].ask;
    double _bid = _tmp_ticks[0].bid;
    long _time = _tmp_ticks[0].time;
#endif
    TickAB<double> _tick(_ask, _bid);
    EmitEntry(TickToEntry(_time, _tick));
  }
};
