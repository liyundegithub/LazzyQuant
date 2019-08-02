#include "config.h"
#include "market_watcher.h"
#include "sinyee_replayer.h"
#include "quant_trader.h"
#include "ctp_executer.h"
#include "quant_trader_manager.h"
#include "quant_trader_manager_bundle.h"

QuantTraderManagerBundle::QuantTraderManagerBundle(MarketWatcher *pWatcher,
                                                   QuantTrader *pTrader,
                                                   CtpExecuter *pExecuter,
                                                   bool replayMode,
                                                   const QPair<QString, QString> &replayRange,
                                                   bool quitAfterReplay)
{
    if (replayMode) {
        QString startDay = replayRange.first;
        QString endDay = replayRange.second;
        bool autoStartReplay = !startDay.isEmpty() && !endDay.isEmpty();

        auto managerReply = new QuantTraderManagerReplay<MarketWatcher, QuantTrader, CtpExecuter>(pWatcher, pTrader, pExecuter);
        if (autoStartReplay) {
            managerReply->setAutoReplayDate(startDay, endDay, quitAfterReplay);
        }
        d = managerReply;
    } else {
        d = new QuantTraderManagerReal<MarketWatcher, QuantTrader, CtpExecuter>(pWatcher, pTrader, pExecuter);
    }
    d->init();
}

QuantTraderManagerBundle::QuantTraderManagerBundle(SinYeeReplayer *pReplayer,
                                                   QuantTrader *pTrader,
                                                   CtpExecuter *pExecuter,
                                                   const QPair<QString, QString> &replayRange,
                                                   bool quitAfterReplay)
{
    QString startDay = replayRange.first;
    QString endDay = replayRange.second;
    bool autoStartReplay = !startDay.isEmpty() && !endDay.isEmpty();

    auto managerReply = new QuantTraderManagerReplay<SinYeeReplayer, QuantTrader, CtpExecuter>(pReplayer, pTrader, pExecuter);
    if (autoStartReplay) {
        managerReply->setAutoReplayDate(startDay, endDay, quitAfterReplay);
    }
    managerReply->init();
}

QuantTraderManagerBundle::~QuantTraderManagerBundle()
{
    if (d) {
        auto *pSource = d->getSource();
        auto *pExecuter = d->getExecuter();
        delete d;
        delete pSource;
        delete pExecuter;
    }
}
