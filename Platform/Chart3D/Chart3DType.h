//+------------------------------------------------------------------+
//|                                                EA31337 framework |
//|                                 Copyright 2016-2023, EA31337 Ltd |
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
 * 3D chart type renderer.
 */

#ifndef __MQL__
// Allows the preprocessor to include a header file when it is needed.
#pragma once
#endif

#include "../../Refs.mqh"
#include "Device.h"

class Chart3D;
class Device;

/**
 * 3D chart type renderer.
 */
class Chart3DType : public Dynamic {
 protected:
  Chart3D* chart3d;
  Device* device;

 public:
  /**
   * Constructor.
   */
  Chart3DType(Chart3D* _chart3d, Device* _device) : chart3d(_chart3d), device(_device) {}

  Device* GetDevice() { return device; }

  /**
   * Renders chart.
   */
  virtual void Render(Device* _device) {}
};
