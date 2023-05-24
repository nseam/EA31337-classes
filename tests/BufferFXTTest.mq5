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

/**
 * @file
 * Test functionality of Buffer class.
 */

// Includes
#include "../BufferFXT.mqh"
#include "../Platform.h"
#include "../Test.mqh"

BufferFXT *ticks;

/**
 * Implements OnInit().
 */
int OnInit() {
  Platform::Init();
  ticks = new BufferFXT(Platform::FetchDefaultCandleIndicator("EURUSD", PERIOD_M1));
  // Test 1.
  // @todo
  return (GetLastError() > 0 ? INIT_FAILED : INIT_SUCCEEDED);
}

/**
 * Implements OnTick().
 */
void OnTick() { Platform::Tick(); }

/**
 * Implements OnDeinit().
 */
void OnDeinit(const int reason) { delete ticks; }
