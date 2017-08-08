#include <QCoreApplication>

#include "config.h"
#include "quant_trader.h"
#include "strategy_status.h"
#include "connection_manager.h"

#include "market_watcher_interface.h"
#include "trade_executer_interface.h"

com::lazzyquant::market_watcher *pWatcher = nullptr;
com::lazzyquant::trade_executer *pExecuter = nullptr;
StrategyStatusManager *pStatusManager = nullptr;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("quant_trader");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    pWatcher = new com::lazzyquant::market_watcher(WATCHER_DBUS_SERVICE, WATCHER_DBUS_OBJECT, QDBusConnection::sessionBus());
    pExecuter = new com::lazzyquant::trade_executer(EXECUTER_DBUS_SERVICE, EXECUTER_DBUS_OBJECT, QDBusConnection::sessionBus());
    pStatusManager = new StrategyStatusManager();
    QuantTrader quantTrader;
    ConnectionManager *manager = new ConnectionManager({pWatcher}, {&quantTrader});

    int ret = a.exec();
    delete manager;
    delete pStatusManager;
    delete pExecuter;
    delete pWatcher;
    return ret;
}
