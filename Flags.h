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
 * Flags manager.
 */

#ifndef __MQL__
// Allows the preprocessor to include a header file when it is needed.
#pragma once
#endif

#define FLAGS_DEFAULT 0

template<typename T>
struct Flags
{
  T value;
  T default_flags;
  bool default_flags_set;

  Flags(T _flags = FLAGS_DEFAULT, T _default_flags = FLAGS_DEFAULT) {
    if (_default_flags != FLAGS_DEFAULT) {
      default_flags = _default_flags;
      default_flags_set = true;
    }
    else {
      default_flags_set = false;
    }
    
    if (_flags == FLAGS_DEFAULT) {
      SetFlags(default_flags_set ? default_flags : (T)0);
    }
    else {
      SetFlags(_flags);
    }
  }

  bool HasFlag(T _flag) {
    return (value & _flag) != (T)0;
  }

  bool HasExactFlags(T _flags) {
    return (value & _flags) == _flags;
  }  

  void AddFlag(T _flag_or_flags) {
    value |= _flag_or_flags;
  }  

  void RemoveFlag(T _flag_or_flags) {
    value &= ~_flag_or_flags;
  }  

  void SetFlags(T _flags) {
    value = _flags;
  }

  void Clear() {
    value = default_flags_set ? default_flags : (T)0;
  }  

  bool IsDefault() {
    return default_flags_set && value == default_flags;
  }

  void operator=(T _value) {
    SetFlags(_value);
  }  

  void operator+=(T _value) {
    AddFlag(_value);
  }  

  void operator-=(T _value) {
    RemoveFlag(_value);
  }  
};
