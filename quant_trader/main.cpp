#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "quant_trader.h"
#include "strategy_status.h"
#include "connection_manager.h"
#include "trading_calendar.h"
#include "multiple_timer.h"

#include "market_watcher_interface.h"
#include "sinyee_replayer_interface.h"
#include "trade_executer_interface.h"

using std::placeholders::_1;
using std::placeholders::_2;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("quant_trader");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Collect K lines, calculate indicators/strategies and call executer automatically.");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({
        // replay mode (-r, --replay)
        {{"r", "replay"},
            QCoreApplication::translate("main", "Replay Mode")},
        {{"s", "start"},
            QCoreApplication::translate("main", "Start Date"), "ReplayStartDate"},
        {{"e", "end"},
            QCoreApplication::translate("main", "End Date"), "ReplayEndDate"},
    });

    parser.process(a);
    bool replayMode = parser.isSet("replay");
    QString replayStartDate;
    QString replayEndDate;
    if (replayMode) {
        replayStartDate = parser.value("start");
        replayEndDate = parser.value("end");
    }

    com::lazzyquant::market_watcher *pWatcher = new com::lazzyquant::market_watcher(WATCHER_DBUS_SERVICE, WATCHER_DBUS_OBJECT, QDBusConnection::sessionBus());

    QObject* pWatcherOrReplayer = nullptr;
    if (replayMode) {
        if (pWatcher->isValid() && pWatcher->isReplayMode()) {
            pWatcherOrReplayer = pWatcher;
        } else {
            delete pWatcher;
            com::lazzyquant::sinyee_replayer *pReplayer = new com::lazzyquant::sinyee_replayer(REPLAYER_DBUS_SERVICE, REPLAYER_DBUS_OBJECT, QDBusConnection::sessionBus());
            if (pReplayer->isValid()) {
                pWatcherOrReplayer = pReplayer;
            } else {
                delete pReplayer;
                qWarning() << "No valid data source!";
                return -1;
            }
        }
    } else {
        pWatcherOrReplayer = pWatcher;
    }

    com::lazzyquant::trade_executer *pExecuter = nullptr;
    QuantTrader quantTrader;
    MultipleTimer *multiTimer = nullptr;

    ConnectionManager *pConnManager = new ConnectionManager({pWatcherOrReplayer}, {&quantTrader});

    if (replayMode) {
        TradingCalendar tc;
        QStringList replayDates;
        QDate startDate = QDate::fromString(replayStartDate, "yyyyMMdd");
        QDate endDate = QDate::fromString(replayEndDate, "yyyyMMdd");
        for (QDate date = startDate; date <= endDate; date = date.addDays(1)) {
            if (tc.isTradingDay(date)) {
                replayDates << date.toString("yyyyMMdd");
            }
        }

        QTimer::singleShot(500, [pWatcherOrReplayer, replayDates]() -> void {
                               for (const auto& date : qAsConst(replayDates)) {
                                   pWatcherOrReplayer->metaObject()->invokeMethod(pWatcherOrReplayer, "startReplay", Q_ARG(QString, date));
                               }
                           });
    } else {
        pExecuter = new com::lazzyquant::trade_executer(EXECUTER_DBUS_SERVICE, EXECUTER_DBUS_OBJECT, QDBusConnection::sessionBus());
        quantTrader.cancelAllOrders = std::bind(&com::lazzyquant::trade_executer::cancelAllOrders, pExecuter, _1);
        quantTrader.setPosition = std::bind(&com::lazzyquant::trade_executer::setPosition, pExecuter, _1, _2);

        multiTimer = new MultipleTimer({{8, 50}, {20, 50}});
        QObject::connect(multiTimer, &MultipleTimer::timesUp, [pWatcher, &quantTrader]() -> void {
                             if (pWatcher->isValid() && pWatcher->getStatus() == "Ready") {
                                 QString tradingDay = pWatcher->getTradingDay();
                                 quantTrader.setTradingDay(tradingDay);
                             }
                         });
    }

    int ret = a.exec();
    delete pConnManager;
    if (multiTimer) {
        multiTimer->disconnect();
        delete multiTimer;
    }
    delete pWatcherOrReplayer;
    if (pExecuter) {
        delete pExecuter;
    }
    return ret;
}
