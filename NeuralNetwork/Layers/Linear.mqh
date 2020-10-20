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
#ifndef NEURAL_NETWORK_LAYER_LINEAR_MQH
#define NEURAL_NETWORK_LAYER_LINEAR_MQH

// Includes.
#include "../Layer.mqh"

/**
 * Neural network layer.
 */
class Linear : public Layer {
 protected:
  // Output weights. Shape of (num_outputs, num_inputs).
  Matrix<double> weight;

  bool biased;

  // Biases.
  Matrix<double> bias;

  // Loss gradients.
  Matrix<double> gradient;

 public:
  virtual Matrix<double>* Weight() { return &weight; }

  virtual Matrix<double>* Bias() { return &bias; }

  virtual Matrix<double>* Gradient() { return &gradient; }

  Linear(int _num_inputs, int _num_outputs, bool _biased = true) {
    biased = _biased;
    inputs.SetShape(_num_inputs);
    outputs.SetShape(_num_outputs);
    weight.SetShape(_num_outputs, _num_inputs);
    bias.SetShape(_num_outputs);
    gradient.SetShape(_num_outputs, _num_inputs);
  }

 protected:
  /**
   * Transfers data between previous and this layer, taking weights into consideration.
   */
  virtual void Forward() {
    if (weight.GetSize() != Input().GetSize() * Output().GetSize()) {
      Alert("Layer::Forward(): Weights and inputs/outputs mimatch!");
      return;
    }

    for (int output_idx = 0; output_idx < Output().GetSize(); ++output_idx) {
      outputs[output_idx] = 0;
      for (int input_idx = 0; input_idx < Input().GetSize(); ++input_idx) {
        outputs[output_idx] = outputs[output_idx].Val() + inputs[input_idx].Val() * weight[output_idx][input_idx].Val();
      }
      outputs[output_idx] = outputs[output_idx].Val() + bias[output_idx].Val();
    }
  }
};

#endif  // NEURAL_NETWORK_LAYER_LINEAR_MQH
