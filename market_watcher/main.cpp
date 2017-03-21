#include <QCoreApplication>

#include "config.h"
#include "market.h"
#include "market_watcher.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    loadCommonMarketData();

    QList<MarketWatcher*> watcherList;
    for (const auto & config : watcherConfigs) {
        MarketWatcher *pWatcher = new MarketWatcher(config);
        watcherList.append(pWatcher);
    }

    int ret = a.exec();

    for (const auto & pWatcher : watcherList) {
        delete pWatcher;
    }
    return ret;
}
