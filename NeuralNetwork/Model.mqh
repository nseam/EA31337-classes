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
#ifndef NEURAL_NETWORK_MODEL_MQH
#define NEURAL_NETWORK_MODEL_MQH

// Includes.
#include "Layer.mqh"

/**
 * Neural network model.
 */
class Model
{
public:

  ~Model() {
    for (int i = 0; i < ArraySize(layers); ++i)
      delete layers[i];
  }

  Layer* layers[];
  
  void Add(Layer* layer) {
    layers[ArraySize(layers)] = layer;
  }
  
  Matrix<double>* Forward(Matrix<double>* _pred) {
    return NULL;
  }

  void Backward(double _loss) {
    return NULL;
  }

};

#endif // NEURAL_NETWORK_MODEL_MQH
