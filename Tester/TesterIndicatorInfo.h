//+------------------------------------------------------------------+
//|                                                     FX31337 wasm |
//|                                 Copyright 2022-2022, EA31337 Ltd |
//|                          https://github.com/FX31337/FX31337-wasm |
//+------------------------------------------------------------------+

/*
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * Details about given indicator. Retrieved from Tester class.
 */

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#endif

// Local includes.
#include "../Std.h"

/**
 * Details about given indicator. Retrieved from Tester class.
 *
 * We can later user Tester::GetIndicatorData(int indi_index, int abs_shift, int mode = 0) to retrieve data from the
 * indicator.
 */
struct TesterIndicatorInfo {
  // Name of the indicator.
  string name;

  // Index of the indicator in the Platform.
  int index;

  // Number of values calculated for this indicator.
  int num_values;

  // Symbol pair indicator works on.
  string symbol;

  // Number of digits for the symbol.
  int symbol_digits;

  // Time-frame indicator works on.
  ENUM_TIMEFRAMES tf;

  // Data type of the indicator's values.
  ENUM_DATATYPE data_type;

  // Indicator's range value data type. E.g., unknown, fixed, dynamic.
  ENUM_IDATA_VALUE_RANGE value_range;

  // Indicator color.
  color indi_color;

  // Constructor.
  TesterIndicatorInfo(string _name = "", int _index = -1, int _num_values = 0, string _symbol = "",
                      ENUM_TIMEFRAMES _tf = PERIOD_CURRENT, ENUM_DATATYPE _data_type = TYPE_DOUBLE,
                      ENUM_IDATA_VALUE_RANGE _value_range = IDATA_RANGE_UNKNOWN, color _indi_color = clrNONE,
                      int _symbol_digits = 0)
      : name(_name), index(_index), num_values(_num_values), symbol(_symbol), symbol_digits(_symbol_digits),
        tf(_tf), data_type(_data_type), value_range(_value_range), indi_color(_indi_color) {}
};
