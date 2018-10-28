#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "market.h"
#include "message_handler.h"
#include "multiple_timer.h"
#include "market_watcher.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("market_watcher");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Receive and save market data, emit signal via D-Bus when market changes");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({
        {{"r", "replay"},
            QCoreApplication::translate("main", "Replay Mode")},
        {{"f", "logtofile"},
            QCoreApplication::translate("main", "Save log to a file")},
    });

    parser.process(a);
    bool replayMode = parser.isSet("replay");
    bool log2File = parser.isSet("logtofile");
    setupMessageHandler(true, log2File, "market_watcher");

    if (!replayMode)
        loadCommonMarketData();

    QList<MarketWatcher*> watcherList;
    for (const auto & config : watcherConfigs) {
        MarketWatcher *pWatcher = new MarketWatcher(config, replayMode);
        watcherList.append(pWatcher);
    }
    MultipleTimer *multiTimer = new MultipleTimer({{8, 45}, {8, 50}, {20, 45}, {20, 50}});
    QObject::connect(multiTimer, &MultipleTimer::timesUp, [=]() -> void {
        for (const auto & pWatcher : qAsConst(watcherList)) {
            if (pWatcher->isLoggedIn()) {
                QString tradingDay = pWatcher->getTradingDay();
                pWatcher->setTradingDay(tradingDay);
            } else {
                qWarning("Market Watcher not login!");
            }
        }
    });

    int ret = a.exec();

    delete multiTimer;
    for (const auto & pWatcher : qAsConst(watcherList)) {
        delete pWatcher;
    }
    restoreMessageHandler();
    return ret;
}
