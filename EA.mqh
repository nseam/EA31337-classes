//+------------------------------------------------------------------+
//|                                                EA31337 framework |
//|                                 Copyright 2016-2021, EA31337 Ltd |
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
 * Implements Expert Advisor class for writing custom trading robots.
 */

// Prevents processing this includes file for the second time.
#ifndef EA_MQH
#define EA_MQH

// Includes.
#include "Action.enum.h"
#include "Chart.mqh"
#include "Condition.enum.h"
#include "Data.struct.h"
#include "Dict.mqh"
#include "DictObject.mqh"
#include "EA.enum.h"
#include "EA.struct.h"
#include "Market.mqh"
#include "Refs.struct.h"
#include "SerializerConverter.mqh"
#include "SerializerCsv.mqh"
#include "SerializerJson.mqh"
#include "SerializerSqlite.mqh"
#include "Strategy.mqh"
#include "SummaryReport.mqh"
#include "Task.mqh"
#include "Terminal.mqh"
#include "Trade.mqh"

class EA {
 public:
 protected:
  // Class variables.
  Account *account;
  DictObject<ENUM_TIMEFRAMES, DictStruct<long, Ref<Strategy>>> strats;
  DictStruct<short, TaskEntry> tasks;
  Ref<Market> market;
  Ref<Log> logger;
  SummaryReport *report;
  Ref<Terminal> terminal;

  // Data variables.
  BufferStruct<ChartEntry> data_chart;
  BufferStruct<SymbolInfoEntry> data_symbol;
  Dict<string, double> ddata;  // Custom user data.
  Dict<string, int> idata;     // Custom user data.
  DictObject<ENUM_TIMEFRAMES, BufferStruct<IndicatorDataEntry>> data_indi;
  DictObject<ENUM_TIMEFRAMES, BufferStruct<StgEntry>> data_stg;
  // DictObject<string, Trade> trade;  // @todo
  EAParams eparams;
  EAProcessResult eresults;
  EAState estate;

 public:
  /**
   * Class constructor.
   */
  EA(EAParams &_params)
      : account(new Account),
        logger(new Log(_params.Get<ENUM_LOG_LEVEL>(STRUCT_ENUM(EAParams, EA_PARAM_PROP_LOG_LEVEL)))),
        market(new Market(_params.Get<string>(STRUCT_ENUM(EAParams, EA_PARAM_PROP_SYMBOL)), logger.Ptr())),
        report(new SummaryReport),
        terminal(new Terminal) {
    eparams = _params;
    estate.SetFlag(EA_STATE_FLAG_ON_INIT, true);
    UpdateStateFlags();
    // Add and process tasks.
    AddTask(eparams.GetStruct<TaskEntry>(STRUCT_ENUM(EAParams, EA_PARAM_STRUCT_TASK_ENTRY)));
    ProcessTasks();
    estate.SetFlag(EA_STATE_FLAG_ON_INIT, false);
  }

  /**
   * Class deconstructor.
   */
  ~EA() {
    // Process tasks on quit.
    estate.SetFlag(EA_STATE_FLAG_ON_QUIT, true);
    ProcessTasks();
    // Deinitialize classes.
    Object::Delete(account);
    Object::Delete(report);
  }

  Log *Logger() { return logger.Ptr(); }

  /* Getters */

  /**
   * Gets a strategy parameter value.
   */
  template <typename T>
  T Get(STRUCT_ENUM(EAParams, ENUM_EA_PARAM_PROP) _param) {
    return eparams.Get<T>(_param);
  }

  /* Setters */

  /**
   * Sets an EA parameter value.
   */
  template <typename T>
  void Set(STRUCT_ENUM(EAParams, ENUM_EA_PARAM_PROP) _param, T _value) {
    return eparams.Set<T>(_param, _value);
  }

  /**
   * Sets an strategy parameter value for all strategies in the given timeframe.
   */
  template <typename T>
  void Set(ENUM_STRATEGY_PARAM _param, T _value, ENUM_TIMEFRAMES _tf) {
    for (DictStructIterator<long, Ref<Strategy>> iter = strats[_tf].Begin(); iter.IsValid(); ++iter) {
      Strategy *_strat = iter.Value().Ptr();
      _strat.Set<T>(_param, _value);
    }
  }

  /**
   * Sets an strategy parameter value for the given timeframe.
   */
  template <typename T>
  void Set(ENUM_STRATEGY_PARAM _param, T _value) {
    for (DictObjectIterator<ENUM_TIMEFRAMES, DictStruct<long, Ref<Strategy>>> itf = strats.Begin(); itf.IsValid();
         ++itf) {
      Set(_param, _value, itf.Key());
    }
  }

  /**
   * Sets an strategy parameter value for all strategies in the given timeframe.
   */
  template <typename T>
  void Set(ENUM_TRADE_PARAM _param, T _value, ENUM_TIMEFRAMES _tf) {
    for (DictStructIterator<long, Ref<Strategy>> iter = strats[_tf].Begin(); iter.IsValid(); ++iter) {
      Strategy *_strat = iter.Value().Ptr();
      _strat.Set<T>(_param, _value);
    }
  }

  /**
   * Sets an strategy parameter value for the given timeframe.
   */
  template <typename T>
  void Set(ENUM_TRADE_PARAM _param, T _value) {
    for (DictObjectIterator<ENUM_TIMEFRAMES, DictStruct<long, Ref<Strategy>>> itf = strats.Begin(); itf.IsValid();
         ++itf) {
      Set(_param, _value, itf.Key());
    }
  }

  /* Processing methods */

  /**
   * Process strategy signals.
   */
  bool ProcessSignals(Strategy *_strat, StrategySignal &_signal, bool _trade_allowed = true) {
    bool _result = true;
    int _last_error = ERR_NO_ERROR;
    ResetLastError();
    if (_strat.CheckCondition(STRAT_COND_TRADE_COND, TRADE_COND_HAS_STATE, TRADE_STATE_ORDERS_ACTIVE)) {
      // Check if we should open and/or close the orders.
      if (_signal.CheckSignalsAll(STRAT_SIGNAL_BUY_CLOSE | STRAT_SIGNAL_BUY_CLOSE_PASS)) {
        _result &= _strat.ExecuteAction(STRAT_ACTION_TRADE_EXE, TRADE_ACTION_ORDERS_CLOSE_BY_TYPE, ORDER_TYPE_BUY);
        // Buy orders closed.
      }
      if (_signal.CheckSignalsAll(STRAT_SIGNAL_SELL_CLOSE | STRAT_SIGNAL_SELL_CLOSE_PASS)) {
        _result &= _strat.ExecuteAction(STRAT_ACTION_TRADE_EXE, TRADE_ACTION_ORDERS_CLOSE_BY_TYPE, ORDER_TYPE_SELL);
        // Sell orders closed.
      }
    }
    if (_trade_allowed) {
      // Open orders on signals.
      if (_signal.CheckSignalsAll(STRAT_SIGNAL_BUY_OPEN | STRAT_SIGNAL_BUY_OPEN_PASS | STRAT_SIGNAL_TIME_PASS)) {
        _strat.Set(TRADE_PARAM_ORDER_COMMENT, _strat.GetOrderOpenComment("B:"));
        // Buy order open.
        _result &= _strat.ExecuteAction(STRAT_ACTION_TRADE_EXE, TRADE_ACTION_ORDER_OPEN, ORDER_TYPE_BUY);
      }
      if (_signal.CheckSignalsAll(STRAT_SIGNAL_SELL_OPEN | STRAT_SIGNAL_SELL_OPEN_PASS | STRAT_SIGNAL_TIME_PASS)) {
        _strat.Set(TRADE_PARAM_ORDER_COMMENT, _strat.GetOrderOpenComment("S:"));
        // Sell order open.
        _result &= _strat.ExecuteAction(STRAT_ACTION_TRADE_EXE, TRADE_ACTION_ORDER_OPEN, ORDER_TYPE_SELL);
      }
      if (!_result) {
        _last_error = GetLastError();
        switch (_last_error) {
          case ERR_NOT_ENOUGH_MEMORY:
            logger.Ptr().Error(StringFormat("Not enough money to open trades! Code: %d", _last_error),
                               __FUNCTION_LINE__, _strat.GetName());
            logger.Ptr().Warning(StringFormat("Suspending strategy.", _last_error), __FUNCTION_LINE__,
                                 _strat.GetName());
            _strat.Suspended(true);
            break;
        }
      }
    }
    _last_error = GetLastError();
    if (_last_error > 0) {
      logger.Ptr().Warning(StringFormat("Processing signals failed! Code: %d", _last_error), __FUNCTION_LINE__,
                           _strat.GetName());
    }
    return _last_error == 0;
  }

  /**
   * Process strategy on tick event for the given timeframe.
   *
   * @return
   *   Returns struct with the processed results.
   */
  virtual EAProcessResult ProcessTickByTf(const ENUM_TIMEFRAMES _tf, const MqlTick &_tick) {
    for (DictStructIterator<long, Ref<Strategy>> iter = strats[_tf].Begin(); iter.IsValid(); ++iter) {
      bool _can_trade = true;
      Strategy *_strat = iter.Value().Ptr();
      if (_strat.IsEnabled()) {
        if (estate.new_periods != DATETIME_NONE) {
          // Process when new periods started.
          _strat.OnPeriod(estate.new_periods);
          eresults.stg_processed_periods++;
        }
        if (_strat.TickFilter(_tick)) {
          _can_trade &= _can_trade && !_strat.IsSuspended();
          _can_trade &= _can_trade &&
                        !_strat.CheckCondition(STRAT_COND_TRADE_COND, TRADE_COND_HAS_STATE, TRADE_STATE_TRADE_CANNOT);
          StrategySignal _signal = _strat.ProcessSignals(_can_trade);
          //_signal.Set();
          ProcessSignals(_strat, _signal, _can_trade);
          if (estate.new_periods != DATETIME_NONE) {
            _strat.ProcessOrders();
            _strat.ProcessTasks();
          }
          StgProcessResult _strat_result = _strat.GetProcessResult();
          eresults.last_error = fmax(eresults.last_error, _strat_result.last_error);
          eresults.stg_errored += (int)_strat_result.last_error > ERR_NO_ERROR;
          eresults.stg_processed++;
        }
      }
    }
    return eresults;
  }

  /**
   * Process strategy signals on tick event.
   *
   * Note: Call this method for every tick bar.
   *
   * @return
   *   Returns struct with the processed results.
   */
  virtual EAProcessResult ProcessTick() {
    if (estate.IsEnabled()) {
      eresults.Reset();
      if (estate.IsActive()) {
        GetMarket().SetTick(SymbolInfoStatic::GetTick(_Symbol));
        ProcessPeriods();
        for (DictObjectIterator<ENUM_TIMEFRAMES, DictStruct<long, Ref<Strategy>>> iter_tf = strats.Begin();
             iter_tf.IsValid(); ++iter_tf) {
          ENUM_TIMEFRAMES _tf = iter_tf.Key();
          ProcessTickByTf(_tf, GetMarket().GetLastTick());
        }
        if (eresults.last_error > ERR_NO_ERROR) {
          logger.Ptr().Flush();
        }
      }
      estate.last_updated.Update();
      if (estate.new_periods > 0) {
        // Process data and tasks on new periods.
        ProcessData();
        ProcessTasks();
      }
    }
    return eresults;
  }

  /**
   * Process data to store.
   */
  void ProcessData() {
    long _timestamp = estate.last_updated.GetEntry().GetTimestamp();
    if (eparams.CheckFlagDataStore(EA_DATA_STORE_CHART)) {
      ChartEntry _entry = Chart().GetEntry();
      data_chart.Add(_entry, _entry.bar.ohlc.time);
    }
    if (eparams.CheckFlagDataStore(EA_DATA_STORE_INDICATOR)) {
      for (DictObjectIterator<ENUM_TIMEFRAMES, DictStruct<long, Ref<Strategy>>> iter_tf = strats.Begin();
           iter_tf.IsValid(); ++iter_tf) {
        ENUM_TIMEFRAMES _itf = iter_tf.Key();
        for (DictStructIterator<long, Ref<Strategy>> iter = strats[_itf].Begin(); iter.IsValid(); ++iter) {
          Strategy *_strati = iter.Value().Ptr();
          Indicator *_indi = _strati.GetParams().GetIndicator();
          if (_indi != NULL) {
            IndicatorDataEntry _ientry = _indi.GetEntry();
            if (!data_indi.KeyExists(_itf)) {
              // Create new timeframe buffer if does not exist.
              BufferStruct<IndicatorDataEntry> *_ide = new BufferStruct<IndicatorDataEntry>;
              data_indi.Set(_itf, _ide);
            }
            // Save entry into data_indi.
            data_indi[_itf].Add(_ientry);
          }
        }
      }
    }
    if (eparams.CheckFlagDataStore(EA_DATA_STORE_STRATEGY)) {
      for (DictObjectIterator<ENUM_TIMEFRAMES, DictStruct<long, Ref<Strategy>>> iter_tf = strats.Begin();
           iter_tf.IsValid(); ++iter_tf) {
        ENUM_TIMEFRAMES _stf = iter_tf.Key();
        for (DictStructIterator<long, Ref<Strategy>> iter = strats[_stf].Begin(); iter.IsValid(); ++iter) {
          Strategy *_strat = iter.Value().Ptr();
          StgEntry _sentry = _strat.GetEntry();
          if (!data_stg.KeyExists(_stf)) {
            // Create new timeframe buffer if does not exist.
            BufferStruct<StgEntry> *_se = new BufferStruct<StgEntry>;
            data_stg.Set(_stf, _se);
          }
          // Save data into data_stg.
          data_stg[_stf].Add(_sentry);
        }
      }
    }
    if (eparams.CheckFlagDataStore(EA_DATA_STORE_SYMBOL)) {
      data_symbol.Add(SymbolInfo().GetEntryLast(), _timestamp);
    }
    if (eparams.CheckFlagDataStore(EA_DATA_STORE_TRADE)) {
      // @todo
    }
  }

  /**
   * Checks for new starting periods.
   */
  unsigned int ProcessPeriods() {
    estate.new_periods = estate.last_updated.GetStartedPeriods();
    OnPeriod();
    return estate.new_periods;
  }

  /**
   * Export data.
   */
  void DataExport(unsigned short _methods) {
    long _timestamp = estate.last_updated.GetEntry().GetTimestamp();
    int _serializer_flags = SERIALIZER_FLAG_SKIP_HIDDEN | SERIALIZER_FLAG_INCLUDE_DEFAULT |
                            SERIALIZER_FLAG_INCLUDE_DYNAMIC | SERIALIZER_FLAG_REUSE_STUB | SERIALIZER_FLAG_REUSE_OBJECT;

    if (eparams.CheckFlagDataStore(EA_DATA_STORE_CHART)) {
      string _key_chart = "Chart";
      _key_chart += StringFormat("-%d-%d", data_chart.GetMin(), data_chart.GetMax());

      SerializerConverter _stub = Serializer::MakeStubObject<BufferStruct<ChartEntry>>(_serializer_flags);
      SerializerConverter _obj = SerializerConverter::FromObject(data_chart, _serializer_flags);

      if ((_methods & EA_DATA_EXPORT_CSV) != 0) {
        _obj.ToFile<SerializerCsv>(_key_chart + ".csv", _serializer_flags, &_stub);
      }
      if ((_methods & EA_DATA_EXPORT_DB) != 0) {
        SerializerSqlite::ConvertToFile(_obj, _key_chart + ".sqlite", "chart", _serializer_flags, &_stub);
      }
      if ((_methods & EA_DATA_EXPORT_JSON) != 0) {
        _obj.ToFile<SerializerJson>(_key_chart + ".json", _serializer_flags, &_stub);
      }

      // Required because of SERIALIZER_FLAG_REUSE_STUB flag.
      _stub.Clean();

      // Required because of SERIALIZER_FLAG_REUSE_OBJECT flag.
      _obj.Clean();
    }
    if (eparams.CheckFlagDataStore(EA_DATA_STORE_INDICATOR)) {
      SerializerConverter _stub = Serializer::MakeStubObject<BufferStruct<IndicatorDataEntry>>(_serializer_flags);

      for (DictObjectIterator<ENUM_TIMEFRAMES, DictStruct<long, Ref<Strategy>>> iter_tf = strats.Begin();
           iter_tf.IsValid(); ++iter_tf) {
        ENUM_TIMEFRAMES _itf = iter_tf.Key();
        if (data_indi.KeyExists(_itf)) {
          BufferStruct<IndicatorDataEntry> _indi_buff = data_indi.GetByKey(_itf);

          SerializerConverter _obj = SerializerConverter::FromObject(_indi_buff, _serializer_flags);

          for (DictStructIterator<long, Ref<Strategy>> iter = strats[_itf].Begin(); iter.IsValid(); ++iter) {
            string _key_indi = "Indicator";
            _key_indi += StringFormat("-%d-%d-%d", _itf, _indi_buff.GetMin(), _indi_buff.GetMax());

            if ((_methods & EA_DATA_EXPORT_CSV) != 0) {
              _obj.ToFile<SerializerCsv>(_key_indi + ".csv", _serializer_flags, &_stub);
            }
            if ((_methods & EA_DATA_EXPORT_DB) != 0) {
              SerializerSqlite::ConvertToFile(_obj, _key_indi + ".sqlite", "indicator", _serializer_flags, &_stub);
            }
            if ((_methods & EA_DATA_EXPORT_JSON) != 0) {
              _obj.ToFile<SerializerJson>(_key_indi + ".json", _serializer_flags, &_stub);
            }
          }  // for

          // Required because of SERIALIZER_FLAG_REUSE_OBJECT flag.
          _obj.Clean();
        }  // if
      }

      // Required because of SERIALIZER_FLAG_REUSE_STUB flag.
      _stub.Clean();
    }
    if (eparams.CheckFlagDataStore(EA_DATA_STORE_STRATEGY)) {
      SerializerConverter _stub = Serializer::MakeStubObject<BufferStruct<StgEntry>>(_serializer_flags);

      for (DictObjectIterator<ENUM_TIMEFRAMES, DictStruct<long, Ref<Strategy>>> iter_tf = strats.Begin();
           iter_tf.IsValid(); ++iter_tf) {
        ENUM_TIMEFRAMES _stf = iter_tf.Key();
        for (DictStructIterator<long, Ref<Strategy>> iter = strats[_stf].Begin(); iter.IsValid(); ++iter) {
          if (data_stg.KeyExists(_stf)) {
            string _key_stg = StringFormat("Strategy-%d", _stf);
            BufferStruct<StgEntry> _stg_buff = data_stg.GetByKey(_stf);
            SerializerConverter _obj = SerializerConverter::FromObject(_stg_buff, _serializer_flags);

            _key_stg += StringFormat("-%d-%d-%d", _stf, _stg_buff.GetMin(), _stg_buff.GetMax());
            if ((_methods & EA_DATA_EXPORT_CSV) != 0) {
              _obj.ToFile<SerializerCsv>(_key_stg + ".csv", _serializer_flags, &_stub);
            }
            if ((_methods & EA_DATA_EXPORT_DB) != 0) {
              SerializerSqlite::ConvertToFile(_obj, _key_stg + ".sqlite", "strategy", _serializer_flags, &_stub);
            }
            if ((_methods & EA_DATA_EXPORT_JSON) != 0) {
              _obj.ToFile<SerializerJson>(_key_stg + ".json", _serializer_flags, &_stub);
            }

            // Required because of SERIALIZER_FLAG_REUSE_OBJECT flag.
            _obj.Clean();
          }
        }
      }
      // Required because of SERIALIZER_FLAG_REUSE_STUB flag.
      _stub.Clean();
    }
    if (eparams.CheckFlagDataStore(EA_DATA_STORE_SYMBOL)) {
      SerializerConverter _stub = Serializer::MakeStubObject<BufferStruct<SymbolInfoEntry>>(_serializer_flags);
      SerializerConverter _obj = SerializerConverter::FromObject(data_symbol, _serializer_flags);

      string _key_sym = "Symbol";
      _key_sym += StringFormat("-%d-%d", data_symbol.GetMin(), data_symbol.GetMax());
      if ((_methods & EA_DATA_EXPORT_CSV) != 0) {
        _obj.ToFile<SerializerCsv>(_key_sym + ".csv", _serializer_flags, &_stub);
      }
      if ((_methods & EA_DATA_EXPORT_DB) != 0) {
        SerializerSqlite::ConvertToFile(_obj, _key_sym + ".sqlite", "symbol", _serializer_flags, &_stub);
      }
      if ((_methods & EA_DATA_EXPORT_JSON) != 0) {
        _obj.ToFile<SerializerJson>(_key_sym + ".json", _serializer_flags, &_stub);
      }

      // Required because of SERIALIZER_FLAG_REUSE_STUB flag.
      _stub.Clean();

      // Required because of SERIALIZER_FLAG_REUSE_OBJECT flag.
      _obj.Clean();
    }
    if (eparams.CheckFlagDataStore(EA_DATA_STORE_TRADE)) {
      string _key_trade = "Trade";
      // _key_sym += StringFormat("-%d-%d", data_trade.GetMin(), data_trade.GetMax());
      if ((_methods & EA_DATA_EXPORT_CSV) != 0) {
        // @todo
        // SerializerConverter _stub_trade =
        // Serializer::MakeStubObject<BufferStruct<TradeEntry>>(SERIALIZER_FLAG_SKIP_HIDDEN);
        // SerializerConverter::FromObject(data_trade, SERIALIZER_FLAG_SKIP_HIDDEN).ToFile<SerializerCsv>(_key + ".csv",
        // SERIALIZER_FLAG_SKIP_HIDDEN, &_stub_trade);
      }
      if ((_methods & EA_DATA_EXPORT_DB) != 0) {
        // @todo: Use Database class.
      }
      if ((_methods & EA_DATA_EXPORT_JSON) != 0) {
        // @todo
        // SerializerConverter _stub_trade =
        // Serializer::MakeStubObject<BufferStruct<TradeEntry>>(SERIALIZER_FLAG_SKIP_HIDDEN);
        // SerializerConverter::FromObject(data_trade, SERIALIZER_FLAG_SKIP_HIDDEN).ToFile<SerializerJson>(_key +
        // ".json", SERIALIZER_FLAG_SKIP_HIDDEN, &_stub_trade);
      }
    }
  }

  /**
   * Export data using default methods.
   */
  void DataExport() { DataExport(eparams.Get<unsigned short>(STRUCT_ENUM(EAParams, EA_PARAM_PROP_DATA_EXPORT))); }

  /* Tasks */

  /**
   * Add task.
   */
  void AddTask(TaskEntry &_entry) {
    if (_entry.IsValid()) {
      if (_entry.GetAction().GetType() == ACTION_TYPE_EA) {
        _entry.SetActionObject(GetPointer(this));
      }
      if (_entry.GetCondition().GetType() == COND_TYPE_EA) {
        _entry.SetConditionObject(GetPointer(this));
      }
      tasks.Push(_entry);
    }
  }

  /**
   * Process EA tasks.
   */
  unsigned int ProcessTasks() {
    unsigned int _counter = 0;
    for (DictStructIterator<short, TaskEntry> iter = tasks.Begin(); iter.IsValid(); ++iter) {
      bool _is_processed = false;
      TaskEntry _entry = iter.Value();
      switch (_entry.GetConditionType()) {
        case COND_TYPE_ACCOUNT:
          _entry.SetConditionObject(account);
          break;
        case COND_TYPE_EA:
          _entry.SetConditionObject(THIS_PTR);
          break;
        case COND_TYPE_TRADE:
          // Not supported (yet).
          // _entry.SetConditionObject(trade);
          break;
      }
      switch (_entry.GetActionType()) {
        case ACTION_TYPE_EA:
          _entry.SetActionObject(THIS_PTR);
          break;
        case ACTION_TYPE_TRADE:
          if (Condition::Test(_entry.GetCondition())) {
            for (DictObjectIterator<ENUM_TIMEFRAMES, DictStruct<long, Ref<Strategy>>> iter_tf = strats.Begin();
                 iter_tf.IsValid(); ++iter_tf) {
              ENUM_TIMEFRAMES _tf = iter_tf.Key();
              for (DictStructIterator<long, Ref<Strategy>> iter_strat = strats[_tf].Begin(); iter_strat.IsValid();
                   ++iter_strat) {
                Strategy *_strat = iter_strat.Value().Ptr();
                _is_processed |= _strat.ExecuteAction(STRAT_ACTION_TRADE_EXE, _entry.GetActionId());
              }
            }
          }
          break;
      }
      _is_processed = _is_processed || Task::Process(_entry);
      _counter += (unsigned short)_is_processed;
    }
    return _counter;
  }

  /* Strategy methods */

  /**
   * Adds strategy to specific timeframe.
   *
   * @param
   * _tf - timeframe to add the strategy.
   *
   * @return
   * Returns true if the strategy has been initialized correctly,
   * otherwise false.
   */
  template <typename SClass>
  bool StrategyAdd(ENUM_TIMEFRAMES _tf, long _sid = 0, long _magic_no = 0) {
    bool _result = true;
    int _tfi = ChartTf::TfToIndex(_tf);
    Ref<Strategy> _strat = ((SClass *)NULL).Init(_tf, _magic_no + _tfi);
    if (!strats.KeyExists(_tf)) {
      DictStruct<long, Ref<Strategy>> _new_strat_dict;
      _result &= strats.Set(_tf, _new_strat_dict);
    }
    OnStrategyAdd(_strat.Ptr());
    if (_sid > 0) {
      _result &= strats.GetByKey(_tf).Set(_sid, _strat);
    } else {
      _result &= strats.GetByKey(_tf).Push(_strat);
    }
    return _result;
  }

  /**
   * Adds strategy to multiple timeframes.
   *
   * @param
   * _tfs - timeframes to add strategy (using bitwise operation).
   *
   * @return
   * Returns true if all strategies has been initialized correctly, otherwise
   * false.
   */
  template <typename SClass>
  bool StrategyAdd(unsigned int _tfs, long _sid = 0, long _magic = 0) {
    bool _result = _tfs == 0;
    if ((_tfs & M1B) == M1B) _result = StrategyAdd<SClass>(PERIOD_M1, _sid, _magic);
    if ((_tfs & M5B) == M5B) _result = StrategyAdd<SClass>(PERIOD_M5, _sid, _magic);
    if ((_tfs & M15B) == M15B) _result = StrategyAdd<SClass>(PERIOD_M15, _sid, _magic);
    if ((_tfs & M30B) == M30B) _result = StrategyAdd<SClass>(PERIOD_M30, _sid, _magic);
    if ((_tfs & H1B) == H1B) _result = StrategyAdd<SClass>(PERIOD_H1, _sid, _magic);
    if ((_tfs & H4B) == H4B) _result = StrategyAdd<SClass>(PERIOD_H4, _sid, _magic);
    if ((_tfs & D1B) == D1B) _result = StrategyAdd<SClass>(PERIOD_D1, _sid, _magic);
    if ((_tfs & W1B) == W1B) _result = StrategyAdd<SClass>(PERIOD_W1, _sid, _magic);
    if ((_tfs & MN1B) == MN1B) _result = StrategyAdd<SClass>(PERIOD_MN1, _sid, _magic);
    return _result;
  }

  /* Update methods */

  /**
   * Update EA state flags.
   */
  void UpdateStateFlags() {
    estate.SetFlag(EA_STATE_FLAG_CONNECTED, GetTerminal().IsConnected());
    estate.SetFlag(EA_STATE_FLAG_LIBS_ALLOWED, GetTerminal().IsLibrariesAllowed());
    estate.SetFlag(EA_STATE_FLAG_OPTIMIZATION, GetTerminal().IsOptimization());
    estate.SetFlag(EA_STATE_FLAG_TESTING, GetTerminal().IsTesting());
    estate.SetFlag(EA_STATE_FLAG_TRADE_ALLOWED, GetTerminal().IsTradeAllowed());
    estate.SetFlag(EA_STATE_FLAG_VISUAL_MODE, GetTerminal().IsVisualMode());
  }

  /**
   * Updates info on chart.
   */
  bool UpdateInfoOnChart() {
    bool _result = false;
    if (eparams.Get<int>(STRUCT_ENUM(EAParams, EA_PARAM_PROP_CHART_INFO_FREQ)) > 0) {
      static datetime _last_update = 0;
      if (_last_update + eparams.Get<int>(STRUCT_ENUM(EAParams, EA_PARAM_PROP_CHART_INFO_FREQ)) < TimeCurrent()) {
        _last_update = TimeCurrent();
        // @todo
        _result = true;
      }
    }
    return _result;
  }

  /**
   * Updates strategy lot size.
   */
  bool UpdateLotSize() {
    if (eparams.CheckFlag(EA_PARAM_FLAG_LOTSIZE_AUTO)) {
      // Auto calculate lot size for each strategy.
      return ExecuteAction(EA_ACTION_STRATS_EXE_ACTION, STRAT_ACTION_TRADE_EXE, TRADE_ACTION_CALC_LOT_SIZE);
    }
    return false;
  }

  /* Conditions and actions */

  /**
   * Checks for EA condition.
   *
   * @param ENUM_EA_CONDITION _cond
   *   EA condition.
   * @return
   *   Returns true when the condition is met.
   */
  bool CheckCondition(ENUM_EA_CONDITION _cond, DataParamEntry &_args[]) {
    switch (_cond) {
      case EA_COND_IS_ACTIVE:
        return estate.IsActive();
      case EA_COND_IS_ENABLED:
        return estate.IsEnabled();
      case EA_COND_IS_NOT_CONNECTED:
        estate.SetFlag(EA_STATE_FLAG_CONNECTED, GetTerminal().IsConnected());
        return !estate.IsConnected();
      case EA_COND_ON_NEW_MINUTE:  // On new minute.
        return (estate.new_periods & DATETIME_MINUTE) != 0;
      case EA_COND_ON_NEW_HOUR:  // On new hour.
        return (estate.new_periods & DATETIME_HOUR) != 0;
      case EA_COND_ON_NEW_DAY:  // On new day.
        return (estate.new_periods & DATETIME_DAY) != 0;
      case EA_COND_ON_NEW_WEEK:  // On new week.
        return (estate.new_periods & DATETIME_WEEK) != 0;
      case EA_COND_ON_NEW_MONTH:  // On new month.
        return (estate.new_periods & DATETIME_MONTH) != 0;
      case EA_COND_ON_NEW_YEAR:  // On new year.
        return (estate.new_periods & DATETIME_YEAR) != 0;
      case EA_COND_ON_INIT:
        return estate.IsOnInit();
      case EA_COND_ON_QUIT:
        return estate.IsOnQuit();
      default:
        Logger().Error(StringFormat("Invalid EA condition: %s!", EnumToString(_cond), __FUNCTION_LINE__));
        return false;
    }
  }
  bool CheckCondition(ENUM_EA_CONDITION _cond) {
    ARRAY(DataParamEntry, _args);
    return EA::CheckCondition(_cond, _args);
  }

  /**
   * Execute EA action.
   *
   * @param ENUM_EA_ACTION _action
   *   EA action to execute.
   * @return
   *   Returns true when the action has been executed successfully.
   */
  bool ExecuteAction(ENUM_EA_ACTION _action, DataParamEntry &_args[]) {
    bool _result = true;
    long arg_size = ArraySize(_args);
    switch (_action) {
      case EA_ACTION_DISABLE:
        estate.Enable(false);
        return true;
      case EA_ACTION_ENABLE:
        estate.Enable();
        return true;
      case EA_ACTION_EXPORT_DATA:
        DataExport();
        return true;
      case EA_ACTION_STRATS_EXE_ACTION:
        // Args:
        // 1st (i:0) - Strategy's enum action to execute.
        // 2nd (i:1) - Strategy's argument to pass.
        for (DictObjectIterator<ENUM_TIMEFRAMES, DictStruct<long, Ref<Strategy>>> iter_tf = strats.Begin();
             iter_tf.IsValid(); ++iter_tf) {
          ENUM_TIMEFRAMES _tf = iter_tf.Key();
          if (arg_size > 0) {
            DataParamEntry _sargs[];
            ArrayResize(_sargs, ArraySize(_args) - 1);
            for (int i = 0; i < ArraySize(_sargs); i++) {
              _sargs[i] = _args[i + 1];
            }
            for (DictStructIterator<long, Ref<Strategy>> iter = strats[_tf].Begin(); iter.IsValid(); ++iter) {
              Strategy *_strat = iter.Value().Ptr();
              _result &= _strat.ExecuteAction((ENUM_STRATEGY_ACTION)_args[0].integer_value, _sargs);
            }
          }
        }
        return _result;
      case EA_ACTION_TASKS_CLEAN:
        // @todo
        return tasks.Size() == 0;
      default:
        Logger().Error(StringFormat("Invalid EA action: %s!", EnumToString(_action), __FUNCTION_LINE__));
        return false;
    }
    return _result;
  }
  bool ExecuteAction(ENUM_EA_ACTION _action) {
    ARRAY(DataParamEntry, _args);
    return EA::ExecuteAction(_action, _args);
  }
  bool ExecuteAction(ENUM_EA_ACTION _action, long _arg1) {
    ARRAY(DataParamEntry, _args);
    DataParamEntry _param1 = _arg1;
    ArrayPushObject(_args, _param1);
    return EA::ExecuteAction(_action, _args);
  }
  bool ExecuteAction(ENUM_EA_ACTION _action, long _arg1, long _arg2) {
    ARRAY(DataParamEntry, _args);
    DataParamEntry _param1 = _arg1;
    DataParamEntry _param2 = _arg2;
    ArrayPushObject(_args, _param1);
    ArrayPushObject(_args, _param2);
    return EA::ExecuteAction(_action, _args);
  }

  /* Getters */

  /**
   * Returns pointer to Terminal object.
   */
  Market *GetMarket() { return market.Ptr(); }

  /**
   * Returns pointer to Market object.
   */
  Terminal *GetTerminal() { return terminal.Ptr(); }

  /**
   * Gets EA's name.
   */
  EAParams GetParams() const { return eparams; }

  /**
   * Gets object to strategies.
   */
  DictObject<ENUM_TIMEFRAMES, DictStruct<long, Ref<Strategy>>> *GetStrategies() { return GetPointer(strats); }

  /**
   * Gets Strategy instance by the timeframe and ID.
   */
  Strategy *GetStrategy(ENUM_TIMEFRAMES _tf, int _sid) {
    Strategy *_strat = NULL;
    DictStruct<long, Ref<Strategy>> *_strats_tf = GetStrategiesByTf(_tf);
    if (GetPointer(_strats_tf) != NULL) {
      if (_strats_tf.KeyExists(_sid)) {
        _strat = _strats_tf.GetByKey(_sid).Ptr();
      }
    }
    return _strat;
  }

  /**
   * Gets object to strategies for the given timeframe.
   */
  DictStruct<long, Ref<Strategy>> *GetStrategiesByTf(ENUM_TIMEFRAMES _tf) { return strats.GetByKey(_tf); }

  /* State getters */

  /**
   * Checks if trading is allowed.
   */
  bool IsTradeAllowed() { return estate.IsTradeAllowed(); }

  /**
   * Checks if using libraries is allowed.
   */
  bool IsLibsAllowed() { return estate.IsLibsAllowed(); }

  /* Struct getters */

  /**
   * Gets EA params.
   */
  EAParams GetParams() { return eparams; }

  /**
   * Gets EA state.
   */
  EAState GetState() { return estate; }

  /* Class getters */

  /**
   * Gets pointer to account details.
   */
  Account *Account() { return account; }

  /**
   * Gets pointer to log instance.
   */
  Log *Log() { return logger.Ptr(); }

  /**
   * Gets pointer to market details.
   */
  Market *Market() { return market.Ptr(); }

  /**
   * Gets pointer to strategies.
   */
  DictObject<ENUM_TIMEFRAMES, DictStruct<long, Ref<Strategy>>> *Strategies() { return &strats; }

  /**
   * Gets pointer to symbol details.
   */
  SymbolInfo *SymbolInfo() { return (SymbolInfo *)GetMarket(); }

  /* Setters */

  /* Virtual methods */

  /**
   * Executed when new time is started (like each minute).
   */
  virtual void OnPeriod() {
    if ((estate.new_periods & DATETIME_MINUTE) != 0) {
      // New minute started.
    }
    if ((estate.new_periods & DATETIME_HOUR) != 0) {
      // New hour started.
    }
    if ((estate.new_periods & DATETIME_DAY) != 0) {
      // New day started.
      UpdateLotSize();
    }
    if ((estate.new_periods & DATETIME_WEEK) != 0) {
      // New week started.
    }
    if ((estate.new_periods & DATETIME_MONTH) != 0) {
      // New month started.
    }
    if ((estate.new_periods & DATETIME_YEAR) != 0) {
      // New year started.
    }
  }

  /**
   * Executed on strategy being added.
   *
   * @param _strat Strategy instance.
   * @see StrategyAdd()
   *
   */
  virtual void OnStrategyAdd(Strategy *_strat) {
    float _margin_risk = eparams.Get<float>(STRUCT_ENUM(EAParams, EA_PARAM_PROP_RISK_MARGIN_MAX));
    _strat.Set<float>(TRADE_PARAM_RISK_MARGIN, _margin_risk);
  }

  /* Printer methods */

  /**
   * Returns EA data in textual representation.
   */
  string ToString(string _dlm = "; ") {
    string _output = "";
    _output += eparams.ToString() + _dlm;
    //_output += StringFormat("Strategies: %d", strats.Size());
    return _output;
  }

  /* Serializers */

  /**
   * Returns serialized representation of the object instance.
   */
  SerializerNodeType Serialize(Serializer &_s) {
    _s.Pass(THIS_REF, "account", account, SERIALIZER_FIELD_FLAG_DYNAMIC);

    // In MQL it will be: Market* _market = GetMarket();
    // In C++ it will be: Market& _market = GetMarket();
    // It is needed as PassObject() expects reference to object instead of its pointer.
    MAKE_REF_FROM_PTR(Market, _market, GetMarket());
    _s.PassObject(THIS_REF, "market", _market, SERIALIZER_FIELD_FLAG_DYNAMIC);

    for (DictObjectIterator<ENUM_TIMEFRAMES, DictStruct<long, Ref<Strategy>>> _iter_tf = GetStrategies().Begin();
         _iter_tf.IsValid(); ++_iter_tf) {
      ENUM_TIMEFRAMES _tf = _iter_tf.Key();
      for (DictStructIterator<long, Ref<Strategy>> _iter = GetStrategiesByTf(_tf).Begin(); _iter.IsValid(); ++_iter) {
        Strategy *_strat = _iter.Value().Ptr();
        // @fixme: GH-422
        // _s.PassWriteOnly(this, "strat:" + _strat.GetName(), _strat);
        string _sname = _strat.GetName();  // + "@" + Chart::TfToString(_strat.GetTf()); // @todo
        string _sparams = _strat.GetParams().ToString();
        string _sresults = _strat.GetProcessResult().ToString();
        _s.Pass(THIS_REF, "strat:params:" + _sname, _sparams);
        _s.Pass(THIS_REF, "strat:results:" + _sname, _sresults);
      }
    }
    return SerializerNodeObject;
  }
};
#endif  // EA_MQH
