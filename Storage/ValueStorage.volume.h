//+------------------------------------------------------------------+
//|                                                EA31337 framework |
//|                                 Copyright 2016-2021, EA31337 Ltd |
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
 * Volume getter version of ValueStorage.
 */

// Includes.
#include "ObjectsCache.h"
#include "ValueStorage.history.h"

/**
 * Storage to retrieve volume.
 */
class VolumeValueStorage : public HistoryValueStorage<long> {
 public:
  /**
   * Constructor.
   */
  VolumeValueStorage(ChartBase *_chart) : HistoryValueStorage(_chart) {}

  /**
   * Copy constructor.
   */
  VolumeValueStorage(const VolumeValueStorage &_r) : HistoryValueStorage(_r.chart.Ptr()) {}

  /**
   * Returns pointer to VolumeValueStorage of a given symbol and time-frame.
   */
  static VolumeValueStorage *GetInstance(ChartBase *_chart) {
    VolumeValueStorage *_storage;
    string _key = Util::MakeKey(_chart PTR_DEREF GetId());
    if (!ObjectsCache<VolumeValueStorage>::TryGet(_key, _storage)) {
      _storage = ObjectsCache<VolumeValueStorage>::Set(_key, new VolumeValueStorage(_chart));
    }
    return _storage;
  }

  /**
   * Fetches value from a given shift. Takes into consideration as-series flag.
   */
  virtual long Fetch(int _shift) {
    ResetLastError();
    long _volume = chart REF_DEREF GetVolume(RealShift(_shift));
    if (_LastError != ERR_NO_ERROR) {
      Print("Cannot fetch iVolume! Error: ", _LastError);
      DebugBreak();
    }
    return _volume;
  }
};
