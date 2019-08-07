#include "config.h"
#include "market_watcher.h"
#include "ctp_replayer.h"
#include "sinyee_replayer.h"
#include "quant_trader.h"
#include "ctp_executer.h"
#include "quant_trader.h"
#include "quant_trader_options.h"
#include "quant_trader_manager.h"
#include "quant_trader_bundle.h"
#include "trade_logger.h"

#include <QDateTime>
#include <QTimeZone>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

QuantTraderBundle::QuantTraderBundle(const QuantTraderOptions &options, const QString &source)
{
    QuantTrader *pTrader = new QuantTrader(traderConfigs[0], options.saveBarsToDB());

    CtpExecuter *pExecuter = nullptr;
    if ((!options.replayMode && !options.explicitNoConnectToExecuter) || options.explicitConnectToExecuter) {
        pExecuter = new CtpExecuter(executerConfigs[0]);
        pTrader->cancelAllOrders = std::bind(&CtpExecuter::cancelAllOrders, pExecuter, _1);
        pTrader->setPosition = std::bind(&CtpExecuter::setPosition, pExecuter, _1, _2);
    }

    if (options.replayMode) {
        TickReplayer *pReplayer = nullptr;
        if (source.compare("ctp", Qt::CaseInsensitive) == 0) {
            pReplayer = new CtpReplayer(replayerConfigs[0]);
        } else {    // if (source.compare("sinyee", Qt::CaseInsensitive) == 0)
            pReplayer = new SinYeeReplayer(replayerConfigs[0]);
        }
        auto managerReplay = new QuantTraderManagerReplay<TickReplayer, QuantTrader, CtpExecuter>(pReplayer, pTrader, pExecuter);
        QString startDay = options.replayRange().first;
        QString endDay = options.replayRange().second;
        bool autoStartReplay = !startDay.isEmpty() && !endDay.isEmpty();
        if (autoStartReplay) {
            managerReplay->setAutoReplayDate(startDay, endDay, true);
        }
        pManager = managerReplay;
    } else {
        auto pWatcher = new MarketWatcher(watcherConfigs[0]);
        pManager = new QuantTraderManagerReal<MarketWatcher, QuantTrader, CtpExecuter>(pWatcher, pTrader, pExecuter);
    }

    pManager->init();

    if (options.saveTradeLogToDB) {
        pLogger = new TradeLogger("quant_trader_" + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMddhhmmss")));
        pTrader->logTrade = std::bind(&TradeLogger::positionChanged, pLogger, _1, _2, _3, _4);
    } else {
        pTrader->logTrade = [](qint64 time, const QString &instrumentID, int newPosition, double price) -> void {
            qInfo().noquote() << QDateTime::fromSecsSinceEpoch(time, QTimeZone::utc()).toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
                              << "New position for" << instrumentID << newPosition << ", price =" << price;
        };
    }
}

QuantTraderBundle::~QuantTraderBundle()
{
    delete pLogger;
    auto pSource = pManager->getSource();
    auto pTrader = pManager->getTrader();
    auto pExecuter = pManager->getExecuter();
    delete pManager;
    delete pSource;
    delete pTrader;
    delete pExecuter;
}
