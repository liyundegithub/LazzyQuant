#include "quant_trader_manager_dbus.h"

#include "market_watcher_interface.h"
#include "sinyee_replayer_interface.h"
#include "trade_executer_interface.h"
#include "quant_trader_interface.h"

#include "config.h"

QuantTraderManagerDbus::QuantTraderManagerDbus(bool replayMode, ReplaySource replaySource) :
    QuantTraderManager(replayMode)
{
    if (replayMode && replaySource == SINYEE_REPLAYER) {
        pDataSource = new com::lazzyquant::sinyee_replayer(REPLAYER_DBUS_SERVICE, REPLAYER_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
    } else {
        pDataSource = new com::lazzyquant::market_watcher(WATCHER_DBUS_SERVICE, WATCHER_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
    }
    pTrader = new com::lazzyquant::quant_trader(TRADER_DBUS_SERVICE, TRADER_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
    pExecuter = new com::lazzyquant::trade_executer(EXECUTER_DBUS_SERVICE, EXECUTER_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
}

QuantTraderManagerDbus::~QuantTraderManagerDbus()
{
    //
}
