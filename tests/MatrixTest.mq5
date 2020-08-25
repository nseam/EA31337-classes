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
 * Test functionality of Matrix class.
 */

// Includes.
#include "../Matrix.mqh"
#include "../Test.mqh"

/**
 * Implements Init event handler.
 */
int OnInit() {
  int a, b, c;

  Matrix<double> matrix(2, 3, 20);

  assertTrueOrFail(matrix.GetRange(0) == 2, "1st dimension's length is not valid!");
  assertTrueOrFail(matrix.GetRange(1) == 3, "2nd dimension's length iś not valid!");
  assertTrueOrFail(matrix.GetRange(2) == 20, "3rd dimension's length is not valid!");

  assertTrueOrFail(matrix.GetDimensions() == 3, "Number of matrix dimensions isn't valid!");

  matrix.Fill(1);

  for (a = 0; a < matrix.GetRange(0); ++a) {
    for (b = 0; b < matrix.GetRange(1); ++b) {
      for (c = 0; c < matrix.GetRange(2); ++c) {
        assertTrueOrFail(matrix[a][b][c].Val() == 1, "Fill() didn't fill the whole matrix!");
      }
    }
  }

  matrix.Add(2);

  for (a = 0; a < matrix.GetRange(0); ++a) {
    for (b = 0; b < matrix.GetRange(1); ++b) {
      for (c = 0; c < matrix.GetRange(2); ++c) {
        assertTrueOrFail(matrix[a][b][c].Val() == 3, "Add() didn't add value to the whole matrix!");
      }
    }
  }

  matrix.Sub(2);

  for (a = 0; a < matrix.GetRange(0); ++a) {
    for (b = 0; b < matrix.GetRange(1); ++b) {
      for (c = 0; c < matrix.GetRange(2); ++c) {
        assertTrueOrFail(matrix[a][b][c].Val() == 1, "Sub() didn't subtract value from the whole matrix!");
      }
    }
  }

  matrix.Mul(4);

  for (a = 0; a < matrix.GetRange(0); ++a) {
    for (b = 0; b < matrix.GetRange(1); ++b) {
      for (c = 0; c < matrix.GetRange(2); ++c) {
        assertTrueOrFail(matrix[a][b][c].Val() == 4, "Mul() didn't multiply value for the whole matrix!");
      }
    }
  }

  matrix.Div(4);

  for (a = 0; a < matrix.GetRange(0); ++a) {
    for (b = 0; b < matrix.GetRange(1); ++b) {
      for (c = 0; c < matrix.GetRange(2); ++c) {
        assertTrueOrFail(matrix[a][b][c].Val() == 1, "Div() didn't divide value for the whole matrix!");
      }
    }
  }

  assertTrueOrFail((int)matrix.Sum() == matrix.GetSize(), "Sum() didn't sum values for the whole matrix!");

  matrix.FillRandom();

  assertTrueOrFail((int)matrix.Sum() != matrix.GetSize(), "FillRandom() should replace 1's with another values!");

  matrix.FillRandom(-0.1, 0.1);

  for (a = 0; a < matrix.GetRange(0); ++a) {
    for (b = 0; b < matrix.GetRange(1); ++b) {
      for (c = 0; c < matrix.GetRange(2); ++c) {
        assertTrueOrFail(matrix[a][b][c].Val() >= -0.1 && matrix[a][b][c].Val() <= 0.1,
                         "FillRandom() didn't fill random values properly for the whole matrix!");
      }
    }
  }

  Matrix<double> matrix2(1, 5);

  matrix2[0][0] = 1;
  matrix2[0][1] = 2;
  matrix2[0][2] = 4;
  matrix2[0][3] = 7;
  matrix2[0][4] = 12;

  assertTrueOrFail(matrix2.Avg() == 5.2, "Avg() didn't calculate valid average for matrix values!");

  assertTrueOrFail(matrix2.Min() == 1, "Min() didn't find the lowest matrix value!");

  assertTrueOrFail(matrix2.Max() == 12, "Max() didn't find the highest matrix value!");

  assertTrueOrFail(matrix2.Med() == 4, "Med() didn't find median of the matrix values!");

  return INIT_SUCCEEDED;
}