#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "market.h"
#include "market_watcher.h"
#include "ctp_executer.h"
#include "option_arbitrageur.h"

MarketWatcher *pWatcher = nullptr;
CtpExecuter *pExecuter = nullptr;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("option_arbitrageur");
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

    if (!replayMode)
        loadCommonMarketData();

    pWatcher = new MarketWatcher(watcherConfigs[0], replayMode);
    pExecuter = new CtpExecuter(executerConfigs[0]);
    OptionArbitrageur arbitrageur(replayMode, replayDate);

    return a.exec();
}
