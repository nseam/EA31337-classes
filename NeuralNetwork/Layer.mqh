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
#ifndef NEURAL_NETWORK_LAYER_MQH
#define NEURAL_NETWORK_LAYER_MQH

// Includes.
#include "../Matrix.mqh"

/**
 * Neural network layer.
 *
 * Matrix<double> x("[4, 3, 2, 1, 0]");
 * Matrix<double> y("[0, 1, 2, 3, 4]");
 *
 * double learning_rate = 0.0001;
 *
 * Model model;
 *
 * model.Add(new Layer(new LayerLinear(6, 2, true)));
 * model.Add(new Layer(new LayerActivationReLU()));
 * model.Add(new Layer(new LayerLinear(2, 6, true)));
 *
 * model.layers[0].weights.FillIdentity();
 * model.layers[2].weights.FillIdentity();
 *
 * for (int i = 0; i < 1000; ++i) {
 *   Matrix<double>* y_pred = model.Forward(y).
 *   Matrix<double>* loss = y_pred.MeanSquared(MATRIX_OPERATION_SUM, y);
 *
 *   model.Backward(loss);
 *   
 *   for (int k = 0; k < net.NumWeights(); ++k)
 *   {
 *     model.ShiftWeight(k, -learning_rate * model.WeightGradient(k));
 *   }
 *
 *   delete y_pred;
 *   delete loss;
 * }
 *
 */
class Layer
{
public:

  Matrix<double> data;
  Matrix<double> weights;
  Matrix<double> biases;
  
  Layer(int _num_inputs = 0) {
    if (_num_inputs > 0) {
      data.SetShape(_num_inputs);
      weights.SetShape(_num_inputs);
      biases.SetShape(_num_inputs);
    }
  }
  
  int Size() {
    return data.GetSize();
  }
};

#endif // NEURAL_NETWORK_LAYER_MQH
