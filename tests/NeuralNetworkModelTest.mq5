//+------------------------------------------------------------------+
//|                                                EA31337 framework |
//|                       Copyright 2016-2020, 31337 Investments Ltd |
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
 * Test functionality of network model's Model class.
 */

// Includes.
#include "../Test.mqh"
#include "../NeuralNetwork/Model.mqh"
#include "../NeuralNetwork/Layers/Linear.mqh"
#include "../Matrix.mqh"

/**
 * Implements Init event handler.
 */
int OnInit() {
  Matrix<double> x("[4, 3, 2, 1, 0]");
  Matrix<double> y("[0, 1, 2, 3, 4]");
  
  double learning_rate = 0.0001;
  
  Model model;
  
  model.Add(new Linear(6, 2));
//  model.Add(new Layer(new LayerActivationReLU()));
  model.Add(new Linear(2, 6));
  
  model.layers[0].weights.FillIdentity();
  model.layers[2].weights.FillIdentity();
  
  for (int i = 0; i < 10; ++i) {
    Matrix<double>* y_pred = model.Forward(&y);
    
    double loss = y_pred.MeanSquared(MATRIX_OPERATION_SUM, &y);
  
    model.Backward(loss);
    
    for (int k = 0; k < net.NumWeights(); ++k)
    {
      model.ShiftWeight(k, -learning_ratemodel.WeightGradient(k));
    }
  
    delete y_pred;
    delete loss;
  }
  
  return INIT_SUCCEEDED;
}
