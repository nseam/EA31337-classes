//+------------------------------------------------------------------+
//|                 EA31337 - multi-strategy advanced trading robot. |
//|                       Copyright 2016-2020, 31337 Investments Ltd |
//|                                       https://github.com/EA31337 |
//+------------------------------------------------------------------+

/*
    This file is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Prevents processing this includes file for the second time.
#ifndef MATH_MQH
#define MATH_MQH

// Includes.
#include "DictStruct.mqh"

#define NEAR_ZERO 0.00001

// Enums.
// Math conditions.
enum ENUM_MATH_CONDITION {
  MATH_COND_EQ  = 1, // Argument values are equal.
  MATH_COND_GT  = 2, // First value is greater than second.
  MATH_COND_LE  = 3, // First value is lesser than second.
  FINAL_MATH_ENTRY = 4
};

// Struct.
struct MathEquation {
  ENUM_MATH_CONDITION op;
  MqlParam args[2];
};

// Includes standard C++ library for non-MQL code.
#ifndef __MQLBUILD__
#include <cfloat>
#include <cmath>
using namespace std;
#endif

/**
 * Class to provide math related methods.
 */
class Math {

 protected:
  //DictStruct<short, MathEquation> *eq;
 public:

  Math(MqlParam &_arg1, MqlParam &_arg2) {
  }

  /* Conditions */

  /**
   * Checks for math condition.
   *
   * @param ENUM_MATH_CONDITION _cond
   *   Math condition.
   * @param MqlParam[] _args
   *   Condition arguments.
   * @return
   *   Returns true when the condition is met.
   */
  bool Condition(ENUM_MATH_CONDITION _cond, MqlParam &_args[]) {
    switch (_cond) {
      case MATH_COND_EQ:
        // @todo
        return false;
      case MATH_COND_GT:
        // @todo
        return false;
      case MATH_COND_LE:
        // @todo
        return false;
      default:
        //logger.Error(StringFormat("Invalid math condition: %s!", EnumToString(_cond), __FUNCTION_LINE__));
        return false;
    }
  }
  bool Condition(ENUM_MATH_CONDITION _cond) {
    MqlParam _args[] = {};
    return Math::Condition(_cond, _args);
  }
  
  template<typename X>
  static X ReLU(X _value) {
    return (X)MathMax(0, _value);
  }

};
#endif // MATH_MQH
