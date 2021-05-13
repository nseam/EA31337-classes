//+------------------------------------------------------------------+
//|                                                EA31337 framework |
//|                       Copyright 2016-2021, 31337 Investments Ltd |
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

/**
 * @file
 * 3D chart candles renderer.
 */

#include "Chart3DType.h"
#include "Cube.h"
#include "Device.h"
#include "Vertex.h"

class Chart3D;

/**
 * 3D chart candles renderer.
 */
class Chart3DCandles : public Chart3DType {
  Ref<Cube<Vertex>> cube1;
  Ref<Cube<Vertex>> cube2;
  Ref<Cube<Vertex>> cube3;

 public:
  /**
   * Constructor.
   */
  Chart3DCandles(Chart3D* _chart3d, Device* _device) : Chart3DType(_chart3d, _device) {
    cube1 = new Cube<Vertex>(1.0f, 1.0f, 1.0f);
    cube2 = new Cube<Vertex>(0.15f, 1.0f, 0.15f);
    cube3 = new Cube<Vertex>(1.0f, 0.15f, 0.15f);
  }

  /**
   * Renders chart.
   */
  virtual void Render(Device* _device) {
    TSR _tsr;

    for (int _shift = chart3d.GetBarsVisibleShiftStart(); _shift != chart3d.GetBarsVisibleShiftEnd(); --_shift) {
      BarOHLC _ohlc = chart3d.GetPrice(PERIOD_CURRENT, _shift);

      float _height = chart3d.GetPriceScale(_ohlc.GetMaxOC()) - chart3d.GetPriceScale(_ohlc.GetMinOC());
      float higher = _ohlc.IsBear();

      cube1.Ptr().GetTSR().translation.x = chart3d.GetBarPositionX(_shift);
      cube1.Ptr().GetTSR().translation.y = chart3d.GetPriceScale(_ohlc.GetMinOC()) + _height / 2;
      cube1.Ptr().GetTSR().scale.y = _height;
      
      //Print(cube1.Ptr().GetTSR().translation.y);
      
      cube1.Ptr().GetMaterial().SetColor(higher ? 0x00FF00 : 0xFF0000);
      _device.Render(cube1.Ptr());

      cube2.Ptr().GetTSR().translation.x = chart3d.GetBarPositionX(_shift);
      float _line_height = chart3d.GetPriceScale(_ohlc.GetHigh()) - chart3d.GetPriceScale(_ohlc.GetLow());
      cube2.Ptr().GetTSR().translation.y = chart3d.GetPriceScale(_ohlc.GetLow()) + _line_height / 2;
      cube2.Ptr().GetTSR().scale.y = _line_height;
      cube2.Ptr().GetMaterial().SetColor(0x8888888);
      _device.Render(cube2.Ptr());
    }
    
    // Rendering price lines.
    if (true)
    for (float _s = chart3d.GetMinBarsPrice(); _s <= chart3d.GetMaxBarsPrice(); _s += 5.55f) {
      float _y = chart3d.GetPriceScale(_s);

      cube3.Ptr().GetTSR().translation.y = _y;
      cube3.Ptr().GetTSR().scale.x = 200.0f;
      
      cube3.Ptr().GetMaterial().SetColor(0xFFFFFFFF);
      _device.Render(cube3.Ptr());
    }
  }
};