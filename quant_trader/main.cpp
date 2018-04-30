#include <QList>
#include <QTime>
#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "quant_trader.h"
#include "strategy_status.h"
#include "connection_manager.h"
#include "trading_calendar.h"
#include "multiple_timer.h"
#include "message_handler.h"

#include "market_watcher_interface.h"
#include "sinyee_replayer_interface.h"
#include "trade_executer_interface.h"

using std::placeholders::_1;
using std::placeholders::_2;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("quant_trader");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Collect K lines, calculate indicators/strategies and call TradeExecuter automatically.");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({
        {{"r", "replay"},
            QCoreApplication::translate("main", "Replay Mode")},
        {{"s", "start"},
            QCoreApplication::translate("main", "Replay start date"), "yyyyMMdd"},
        {{"e", "end"},
            QCoreApplication::translate("main", "Replay end date"), "yyyyMMdd"},
        {{"a", "save"},
            QCoreApplication::translate("main", "Force saving collected bars to DB even in replay mode.")},
        {{"x", "executer"},
            QCoreApplication::translate("main", "Connect to TradeExecuter, should not be used with -n")},
        {{"n", "noexecuter"},
            QCoreApplication::translate("main", "Don't connect to TradeExecuter, should not be used with -x")},
        {{"f", "logtofile"},
            QCoreApplication::translate("main", "Save log to a file")},
    });

    parser.process(a);
    bool replayMode = parser.isSet("replay");
    QString replayStartDate;
    QString replayEndDate;
    if (replayMode) {
        replayStartDate = parser.value("start");
        replayEndDate = parser.value("end");
    }
    bool explicitSave = parser.isSet("save");
    bool saveBarsToDB = explicitSave || (!replayMode);

    bool explicitConnectToExecuter = parser.isSet("executer");
    bool explicitNoConnectToExecuter = parser.isSet("noexecuter");

    bool log2File = parser.isSet("logtofile");
    setupMessageHandler(true, log2File, "quant_trader");

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

    QuantTrader quantTrader(saveBarsToDB);
    MultipleTimer *marketOpenTimer = nullptr;
    MultipleTimer *marketCloseTimer = nullptr;

    ConnectionManager *pConnManager = new ConnectionManager({pWatcherOrReplayer}, {&quantTrader});

    com::lazzyquant::trade_executer *pExecuter = nullptr;
    if ((!replayMode && !explicitNoConnectToExecuter) || explicitConnectToExecuter) {
        pExecuter = new com::lazzyquant::trade_executer(EXECUTER_DBUS_SERVICE, EXECUTER_DBUS_OBJECT, QDBusConnection::sessionBus());
        quantTrader.cancelAllOrders = std::bind(&com::lazzyquant::trade_executer::cancelAllOrders, pExecuter, _1);
        quantTrader.setPosition = std::bind(&com::lazzyquant::trade_executer::setPosition, pExecuter, _1, _2);
    }

    if (replayMode) {
        TradingCalendar tc;
        QStringList replayDates;
        QDate startDate = QDate::fromString(replayStartDate, QStringLiteral("yyyyMMdd"));
        QDate endDate = QDate::fromString(replayEndDate, QStringLiteral("yyyyMMdd"));
        for (QDate date = startDate; date <= endDate; date = date.addDays(1)) {
            if (tc.isTradingDay(date)) {
                replayDates << date.toString(QStringLiteral("yyyyMMdd"));
            }
        }

        QObject::connect(pWatcherOrReplayer, SIGNAL(endOfReplay(QString)), &quantTrader, SLOT(onMarketClose()));

        QTimer::singleShot(500, [pWatcherOrReplayer, replayDates]() -> void {
                               for (const auto& date : qAsConst(replayDates)) {
                                   pWatcherOrReplayer->metaObject()->invokeMethod(pWatcherOrReplayer, "startReplay", Q_ARG(QString, date));
                               }
                           });
    } else {
        marketOpenTimer = new MultipleTimer({{8, 50}, {20, 50}});
        QObject::connect(marketOpenTimer, &MultipleTimer::timesUp, [pWatcher, &quantTrader]() -> void {
                             if (pWatcher->isValid() && pWatcher->getStatus() == "Ready") {
                                 QString tradingDay = pWatcher->getTradingDay();
                                 quantTrader.setTradingDay(tradingDay);
                             } else {
                                 qWarning() << "Market Watcher Not Ready!";
                             }
                             quantTrader.checkDataBaseStatus();
                         });

        QList<QTime> timePoints({{2, 35}, {11, 35}, {15, 5}});
        int tradingTimeSegments = timePoints.size();
        marketCloseTimer = new MultipleTimer(timePoints);
        QObject::connect(marketCloseTimer, &MultipleTimer::timesUp, [&quantTrader, tradingTimeSegments](int idx) -> void {
            if (idx == (tradingTimeSegments - 1)) {
                quantTrader.onMarketClose();
            } else {
                quantTrader.onMarketPause();
            }
        });
    }

    int ret = a.exec();
    delete pConnManager;
    if (marketOpenTimer) {
        marketOpenTimer->disconnect();
        delete marketOpenTimer;
    }
    if (marketCloseTimer) {
        marketCloseTimer->disconnect();
        delete marketCloseTimer;
    }
    delete pWatcherOrReplayer;
    if (pExecuter) {
        delete pExecuter;
    }
    restoreMessageHandler();
    return ret;
}
