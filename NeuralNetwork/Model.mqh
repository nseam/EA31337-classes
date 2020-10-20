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
#include "Layers/Input.mqh"

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
  
  // Layer used as input.
  Layer* input_layer;
  
  void Add(Layer* layer) {
    ArrayResize(layers, ArraySize(layers) + 1, 10);
    layers[ArraySize(layers) - 1] = layer;
  }
  
  Matrix<double>* Forward(Matrix<double>* _input) {
    // Evaluating layers.
    for (int i = 0; i < ArraySize(layers); ++i) {
      // Initializing inputs from previous layer's outputs.
      if (i == 0)
        layers[i].inputs = _input;
      else
        layers[i].inputs = layers[i - 1].outputs;

      layers[i].Forward();
    }
  
    return &layers[ArraySize(layers) - 1].outputs;
  }

  void Backward(double _loss) {
    
  }

};

#endif // NEURAL_NETWORK_MODEL_MQH
