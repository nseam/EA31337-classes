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
 * Test functionality of Taskable class.
 */

// Includes.
#include "../../Test.mqh"
#include "../Taskable.h"

// Defines structure.
struct TaskableIntValue {
 protected:
  int value;  // Field to store a double type.
 public:
  TaskableIntValue() : value(0) {}
  void SetValue(int _value) { value = _value; }
};

// Defines class.
class TaskTest01 : public Taskable<TaskableIntValue> {
 protected:
  int sum;
  TaskableIntValue data;

 public:
  TaskTest01() : sum(0){};
  int GetSum() { return sum; }

  /* Taskable methods */

  /**
   * Checks a condition.
   */
  bool Check(const TaskConditionEntry &_entry) {
    sum += _entry.GetId();
    PrintFormat("%s; sum=%d", __FUNCSIG__, sum);
    return sum > 0;
  }

  /**
   * Gets a copy of structure.
   */
  TaskableIntValue Get(const TaskGetterEntry &_entry) {
    sum += _entry.GetId();
    data.SetValue(sum);
    PrintFormat("%s; sum=%d", __FUNCSIG__, sum);
    return data;
  }

  /**
   * Runs an action.
   */
  bool Run(const TaskActionEntry &_entry) {
    sum += _entry.GetId();
    PrintFormat("%s; sum=%d", __FUNCSIG__, sum);
    return sum > 0;
  }
};

/**
 * Implements Init event handler.
 */
int OnInit() {
  bool _result = true;
  TaskTest01 _test01;
  // Runs a dummy action.
  TaskActionEntry _entry_action(2);
  _result &= _test01.Run(_entry_action);
  _result &= _entry_action.Get(STRUCT_ENUM(TaskActionEntry, TASK_ACTION_ENTRY_FLAG_IS_ACTIVE));
  // Checks a dummy condition.
  TaskConditionEntry _entry_cond(2);
  _result &= _test01.Check(_entry_cond);
  _result &= _entry_cond.Get(STRUCT_ENUM(TaskConditionEntry, TASK_CONDITION_ENTRY_FLAG_IS_ACTIVE));
  // Gets a dummy value.
  TaskGetterEntry _entry_get(2);
  TaskableIntValue _value = _test01.Get(_entry_get);
  _result &= _entry_get.Get(STRUCT_ENUM(TaskGetterEntry, TASK_GETTER_ENTRY_FLAG_IS_ACTIVE));
  // Checks the results.
  assertTrueOrFail(_result && _test01.GetSum() == 6, "Fail!");
  _result &= GetLastError() == ERR_NO_ERROR;
  return (_result ? INIT_SUCCEEDED : INIT_FAILED);
}

/**
 * Implements Tick event handler.
 */
void OnTick() {}

/**
 * Implements Deinit event handler.
 */
void OnDeinit(const int reason) {}
