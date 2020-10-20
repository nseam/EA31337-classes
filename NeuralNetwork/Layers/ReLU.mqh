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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Prevents processing this includes file for the second time.
#ifndef NEURAL_NETWORK_LAYER_RELU_MQH
#define NEURAL_NETWORK_LAYER_RELU_MQH

// Includes.
#include "../../Math.mqh"
#include "../Layer.mqh"

/**
 * Neural network layer.
 */
class ReLU : public Layer {
  /**
   * Transfers data between previous and this layer, taking weights into consideration.
   */
  virtual void Forward() {
    Output() = Input();

    for (int i = 0; i < Output().GetSize(); ++i) outputs[i] = Math::ReLU(outputs[i].Val());
  }
};

#endif  // NEURAL_NETWORK_LAYER_RELU_MQH
