#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "future_arbitrageur.h"
#include "connection_manager.h"

#include "sinyee_replayer_interface.h"
#include "market_watcher_interface.h"
#include "trade_executer_interface.h"
#include "strategy_status.h"

com::lazzyquant::sinyee_replayer *pReplayer = nullptr;
com::lazzyquant::market_watcher *pWatcher = nullptr;
com::lazzyquant::trade_executer *pExecuter = nullptr;
StrategyStatusManager *pStatusManager = nullptr;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("future_arbitrageur");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Find arbitrageur opportunity and call executer automatically.");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({
        // replay mode (-r, --replay)
        {{"r", "replay"},
            QCoreApplication::translate("main", "Replay Mode"), "ReplayDate"},
    });

    parser.process(a);
    bool replayMode = parser.isSet("replay");
    QString replayDate = QString();
    if (replayMode) {
        replayDate = parser.value("replay");
    }

    if (replayMode) {
        pReplayer = new com::lazzyquant::sinyee_replayer(REPLAYER_DBUS_SERVICE, REPLAYER_DBUS_OBJECT, QDBusConnection::sessionBus());
    } else {
        pWatcher = new com::lazzyquant::market_watcher(WATCHER_DBUS_SERVICE, WATCHER_DBUS_OBJECT, QDBusConnection::sessionBus());
    }
    pExecuter = new com::lazzyquant::trade_executer(EXECUTER_DBUS_SERVICE, EXECUTER_DBUS_OBJECT, QDBusConnection::sessionBus());
    pStatusManager = new StrategyStatusManager();
    FutureArbitrageur arbitrageur;
    ConnectionManager manager({pReplayer, pWatcher}, {&arbitrageur});
    if (replayMode) {
        pReplayer->startReplay(replayDate);
    }
    int ret = a.exec();
    delete pStatusManager;
    delete pExecuter;
    if (pReplayer) {
        delete pReplayer;
    }
    if (pWatcher) {
        delete pWatcher;
    }
    return ret;
}
