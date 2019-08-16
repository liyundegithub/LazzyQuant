#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "message_handler.h"
#include "strategy_status.h"
#include "future_arbitrageur.h"
#include "connection_manager.h"

#include "tick_replayer_interface.h"
#include "market_watcher_interface.h"
#include "trade_executer_interface.h"

com::lazzyquant::trade_executer *pExecuter = nullptr;
StrategyStatusManager *pStatusManager = nullptr;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("future_arbitrageur");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Find arbitrageur opportunity and call executer automatically.");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({
        {{"r", "replay"},
            QCoreApplication::translate("main", "Replay Mode"), "ReplayDate"},
        {{"f", "logtofile"},
            QCoreApplication::translate("main", "Save log to a file")},
    });

    parser.process(a);
    bool replayMode = parser.isSet("replay");
    QString replayDate = QString();
    if (replayMode) {
        replayDate = parser.value("replay");
    }
    bool log2File = parser.isSet("logtofile");
    setupMessageHandler(true, log2File, "future_arbitrageur", !replayMode);

    com::lazzyquant::tick_replayer *pReplayer = nullptr;
    com::lazzyquant::market_watcher *pWatcher = nullptr;
    if (replayMode) {
        pReplayer = new com::lazzyquant::tick_replayer(REPLAYER_DBUS_SERVICE, REPLAYER_DBUS_OBJECT, QDBusConnection::sessionBus());
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
    delete pReplayer;
    delete pWatcher;
    restoreMessageHandler();
    return ret;
}
