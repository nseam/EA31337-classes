//+------------------------------------------------------------------+
//|                                                EA31337 framework |
//|                                 Copyright 2016-2021, EA31337 Ltd |
//|                                       https://github.com/EA31337 |
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
#include "Math.define.h"
#endif

// Data types.
#ifdef __cplusplus
#include <iomanip>
#include <locale>
#include <sstream>
#include <vector>
#endif

#ifndef __MQL__
#define __FUNCSIG__ __FUNCTION__
#endif

#ifdef __MQL__
#define ASSIGN_TO_THIS(TYPE, VALUE) ((TYPE)this) = ((TYPE)VALUE)
#else
#define ASSIGN_TO_THIS(TYPE, VALUE) ((TYPE&)*this) = ((TYPE&)VALUE)
#endif

// Pointers.
#ifdef __MQL__
#define THIS_ATTR
#define THIS_PTR (&this)
#define THIS_REF this
#define PTR_DEREF .
#define PTR_ATTRIB(O, A) O.A
#define PTR_ATTRIB2(O, A, B) O.A.B
#define PTR_TO_REF(PTR) PTR
#define MAKE_REF_FROM_PTR(TYPE, NAME, PTR) TYPE* NAME = PTR
#define nullptr NULL
#define REF_DEREF .Ptr().
#define int64 long
#else
#define THIS_ATTR this->
#define THIS_PTR (this)
#define THIS_REF (*this)
#define PTR_DEREF ->
#define PTR_ATTRIB(O, A) O->A
#define PTR_ATTRIB2(O, A, B) O->A->B
#define PTR_TO_REF(PTR) (*PTR)
#define MAKE_REF_FROM_PTR(TYPE, NAME, PTR) TYPE& NAME = PTR
#define REF_DEREF .Ptr()->
#define int64 long long
#endif

// References.
#ifdef __cplusplus
#define REF(X) (&X)
#else
#define REF(X) X&
#endif

// Arrays and references to arrays.
#define _COMMA ,
#ifdef __MQL__
#define ARRAY_DECLARATION_BRACKETS []
#else
// C++'s _cpp_array is an object, so no brackets are needed.
#define ARRAY_DECLARATION_BRACKETS
#endif

#ifdef __MQL__
/**
 * Reference to object.
 */
#define CONST_REF_TO(T) const T

/**
 * Reference to the array.
 *
 * @usage
 *   ARRAY_REF(<type of the array items>, <name of the variable>)
 */
#define ARRAY_REF(T, N) REF(T) N ARRAY_DECLARATION_BRACKETS
#define FIXED_ARRAY_REF(T, N, S) ARRAY_REF(T, N)

#define CONST_ARRAY_REF(T, N) const N ARRAY_DECLARATION_BRACKETS

/**
 * Array definition.
 *
 * @usage
 *   ARRAY(<type of the array items>, <name of the variable>)
 */
#define ARRAY(T, N) T N[]

#else
/**
 * Reference to object.
 */
#define CONST_REF_TO(T) const T&

/**

 * Reference to the array.
 *
 * @usage
 *   ARRAY_REF(<type of the array items>, <name of the variable>)
 */
#define ARRAY_REF(T, N) _cpp_array<T>& N
#define FIXED_ARRAY_REF(T, N, S) T(&N)[S]

#define CONST_ARRAY_REF(T, N) const _cpp_array<T>& N

/**
 * Array definition.
 *
 * @usage
 *   ARRAY(<type of the array items>, <name of the variable>)
 */
#define ARRAY(T, N) ::_cpp_array<T> N
#endif

// typename(T)
#ifndef __MQL__
#define typename(T) typeid(T).name()
#endif

// C++ array class.
#ifndef __MQL__
/**
 * Custom array template to be used as a replacement of dynamic array in MQL.
 */
template <typename T>
class _cpp_array {
  // List of items.
  std::vector<T> m_data;

  // IsSeries flag.
  bool m_isSeries = false;

 public:
  _cpp_array() {}

  template <int size>
  _cpp_array(const T REF(_arr)[size]) {
    for (const auto& _item : _arr) m_data.push_back(_item);
  }

  _cpp_array(const _cpp_array& r) {
    m_data = r.m_data;
    m_isSeries = r.m_isSeries;
  }

  _cpp_array(_cpp_array& r) {
    m_data.assign(r.m_data.begin(), r.m_data.end());
    m_isSeries = r.m_isSeries;
  }

  std::vector<T>& str() { return m_data; }

  void push(const T& value) { m_data.push_back(value); }

  void operator=(const _cpp_array& r) {
    m_data = r.m_data;
    m_isSeries = r.m_isSeries;
  }

  void operator=(_cpp_array& r) {
    m_data.assign(r.m_data.begin(), r.m_data.end());
    m_isSeries = r.m_isSeries;
  }

  /**
   * Returns pointer of first element (provides a way to iterate over array elements).
   */
  // operator T*() { return &m_data.first(); }

  /**
   * Index operator. Takes care of IsSeries flag.
   */
  T& operator[](int index) { return m_data[m_isSeries ? (size() - index - 1) : index]; }

  /**
   * Index operator. Takes care of IsSeries flag.
   */
  const T& operator[](int index) const { return m_data[m_isSeries ? (size() - index - 1) : index]; }

  /**
   * Returns number of elements in the array.
   */
  int size() const { return (int)m_data.size(); }

  void resize(int new_size, int reserve_size = 0) {
    // E.g., size = 10, new_size = 90, reserve_size = 50
    // thus: new_reserve_size = new_size + reserve_size - (new_size % reserve_size)
    // which is: 90 + reserve_size - (90 % reserve_size) = 90 + 50 - 40 = 100.
    if (reserve_size > 0) {
      new_size = reserve_size - (new_size % reserve_size);
    }
    m_data.reserve(new_size);
    m_data.resize(new_size);
  }

  /**
   * Checks whether
   */
  int getIsSeries() const { return m_isSeries; }

  /**
   * Sets IsSeries flag for an array.
   * Array indexing is from 0 without IsSeries flag or from last-element
   * with IsSeries flag.
   */
  void setIsSeries(bool _isSeries) { m_isSeries = _isSeries; }
};

#ifdef EMSCRIPTEN
#include <emscripten/bind.h>

#define REGISTER_ARRAY_OF(N, T, D)               \
  EMSCRIPTEN_BINDINGS(N) {                       \
    emscripten::class_<_cpp_array<T>>(D)         \
        .constructor()                           \
        .function("Push", &_cpp_array<T>::push)  \
        .function("Size", &_cpp_array<T>::size); \
  }

#endif

template <typename T>
class _cpp_array;
#endif

// Mql's color class.
#ifndef __MQL__
class color {
  unsigned int value;

 public:
  color(unsigned int _color = 0) { value = _color; }
  color& operator=(unsigned int _color) {
    value = _color;
    return *this;
  }
  operator unsigned int() const { return value; }
};
#endif

// MQL defines.
#ifndef __MQL__
#define WHOLE_ARRAY -1  // For processing the entire array.
#endif

// Converts string into C++-style string pointer.
#ifdef __MQL__
#define C_STR(S) S
#else
#define C_STR(S) cstring_from(S)

inline const char* cstring_from(const std::string& _value) { return _value.c_str(); }
#endif

#ifdef __cplusplus
using std::string;
#endif

inline bool IsNull(const string& str) { return str == ""; }

/**
 * Referencing struct's enum.
 *
 * @usage
 *   STRUCT_ENUM(<struct_name>, <enum_name>)
 */
#ifdef __MQL4__
#define STRUCT_ENUM(S, E) E
#else
#define STRUCT_ENUM(S, E) S::E
#endif

#ifndef __MQL__
// Additional enum values for ENUM_SYMBOL_INFO_DOUBLE
#define SYMBOL_MARGIN_LIMIT ((ENUM_SYMBOL_INFO_DOUBLE)46)
#define SYMBOL_MARGIN_MAINTENANCE ((ENUM_SYMBOL_INFO_DOUBLE)43)
#define SYMBOL_MARGIN_LONG ((ENUM_SYMBOL_INFO_DOUBLE)44)
#define SYMBOL_MARGIN_SHORT ((ENUM_SYMBOL_INFO_DOUBLE)45)
#define SYMBOL_MARGIN_STOP ((ENUM_SYMBOL_INFO_DOUBLE)47)
#define SYMBOL_MARGIN_STOPLIMIT ((ENUM_SYMBOL_INFO_DOUBLE)48)
#endif

template <typename T>
class InvalidEnumValue {
 public:
#ifdef __cplusplus
  constexpr
#endif
      static const T
      value() {
    return (T)INT_MAX;
  }
};

#ifndef __MQL__
struct _WRONG_VALUE {
  template <typename T>
  operator T() {
    return (T)-1;
  }
} WRONG_VALUE;

const char* _empty_string_c = "";
const string _empty_string = "";

// Converter of NULL_VALUE into expected type. e.g., "int x = NULL_VALUE" will end up with "x = 0".
struct _NULL_VALUE {
  template <typename T>
  operator T() const {
    return (T)0;
  }

} NULL_VALUE;

/**
 * Converting an enumeration value of any type to a text form.
 *
 * @docs
 * - https://www.mql5.com/en/docs/convert/enumtostring
 */
string EnumToString(int _value) {
  std::stringstream ss;
  // We really don't want to mess with type reflection here (if possible at all). So we are outputting the input
  // integer.
  ss << _value;
  return ss.str();
}

template <>
_NULL_VALUE::operator string() const {
  return _empty_string;
}
#define NULL_STRING ""
#else
#define NULL_VALUE NULL
#define NULL_STRING NULL
#endif

#ifndef __MQL__
#include "Chart.enum.h"
/**
 * Returns currently selected period for platform.
 */
// @fixit Should fetch selected period from somewhere.
extern ENUM_TIMEFRAMES Period();

#endif

#define RUNTIME_ERROR(MSG) \
  Print(MSG);              \
  DebugBreak();
