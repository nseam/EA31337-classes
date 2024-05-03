//+------------------------------------------------------------------+
//|                                                EA31337 framework |
//|                                 Copyright 2016-2023, EA31337 Ltd |
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
#ifndef BUFFER_CANDLE_H
#define BUFFER_CANDLE_H

// Includes.
#include "../BufferStruct.mqh"
#include "../Candle.struct.h"
#include "../Serializer/SerializerConverter.h"
#include "../Serializer/SerializerJson.h"

/**
 * Class to store struct data.
 */
template <typename TV>
class BufferCandle : public BufferStruct<CandleOCTOHLC<TV>> {
 protected:
  /* Protected methods */

  /**
   * Initialize class.
   *
   * Called on constructor.
   */
  void Init() { SetOverflowListener(BufferStructOverflowListener, 10); }

 public:
  /* Constructors */

  /**
   * Constructor.
   */
  BufferCandle() { Init(); }
  BufferCandle(BufferCandle& _right) {
    THIS_REF = _right;
    Init();
  }

  /**
   * Returns JSON representation of the buffer.
   */
  string ToJSON() { return SerializerConverter::FromObject(THIS_REF).ToString<SerializerJson>(); }
};

#endif  // BUFFER_CANDLE_H
