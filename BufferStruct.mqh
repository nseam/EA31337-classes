//+------------------------------------------------------------------+
//|                                                EA31337 framework |
//|                       Copyright 2016-2020, 31337 Investments Ltd |
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

// Prevents processing this includes file for the second time.
#ifndef BUFFER_STRUCT_MQH
#define BUFFER_STRUCT_MQH

// Includes.
#include "DictStruct.mqh"
#include "Serializer.mqh"

// Structs.
struct BufferStructEntry : public MqlParam {
 public:
  /* Struct operators */

  bool operator==(const BufferStructEntry& _s) {
    return type == _s.type && double_value == _s.double_value && integer_value == _s.integer_value &&
           string_value == _s.string_value;
  }

  SerializerNodeType Serialize(Serializer& s) {
    s.PassEnum(this, "type", type, SERIALIZER_FIELD_FLAG_HIDDEN);

    string aux_string;

    switch (type) {
      case TYPE_BOOL:
      case TYPE_UCHAR:
      case TYPE_CHAR:
      case TYPE_USHORT:
      case TYPE_SHORT:
      case TYPE_UINT:
      case TYPE_INT:
      case TYPE_ULONG:
      case TYPE_LONG:
        s.Pass(this, "value", integer_value);
        break;

      case TYPE_DOUBLE:
        s.Pass(this, "value", double_value);
        break;

      case TYPE_STRING:
        s.Pass(this, "value", string_value);
        break;

      case TYPE_DATETIME:
        if (s.IsWriting()) {
          aux_string = TimeToString(integer_value);
          s.Pass(this, "value", aux_string);
        } else {
          s.Pass(this, "value", aux_string);
          integer_value = StringToTime(aux_string);
        }
        break;

      default:
        // Unknown type. Serializing anyway.
        s.Pass(this, "value", aux_string);
    }

    return SerializerNodeObject;
  }

  /**
   * Initializes object with given number of elements. Could be skipped for non-containers.
   */
  template <>
  void SerializeStub(int _n1 = 1, int _n2 = 1, int _n3 = 1, int _n4 = 1, int _n5 = 1) {
    type = TYPE_INT;
    integer_value = 0;
  }
};

/**
 * Class to store struct data.
 */
template <typename TStruct>
class BufferStruct : public DictStruct<long, TStruct> {
 public:
  /* Constructors */

  /**
   * Constructor.
   */
  BufferStruct() {}
  BufferStruct(BufferStruct& _right) { this = _right; }

  /**
   * Adds new value.
   */
  void Add(TStruct& _value, long _dt = 0) {
    _dt = _dt > 0 ? _dt : TimeCurrent();
    Set(_dt, _value);
  }

  /**
   * Clean entries older than given timestamp.
   */
  void Clean(long _dt = 0, bool _older = true) {
    for (DictStructIterator<long, TStruct> iter = Begin(); iter.IsValid(); ++iter) {
      long _time = iter.Key();
      if (_older && _time < _dt) {
        Unset(iter.Key());
      } else if (!_older && _time > _dt) {
        Unset(iter.Key());
      }
    }
  }
};

#endif  // BUFFER_STRUCT_MQH
