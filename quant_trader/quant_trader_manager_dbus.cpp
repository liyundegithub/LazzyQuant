#include "config.h"
#include "quant_trader.h"
#include "quant_trader_manager.h"
#include "quant_trader_manager_dbus.h"

#include "market_watcher_interface.h"
#include "sinyee_replayer_interface.h"
#include "trade_executer_interface.h"

using namespace com::lazzyquant;

QuantTraderManagerDbus::QuantTraderManagerDbus(QuantTrader *pTrader,
                                               bool replayMode,
                                               const QPair<QString, QString> &replayRange,
                                               MarketSource source)
{
    if (replayMode) {
        QString startDay = replayRange.first;
        QString endDay = replayRange.second;
        bool autoStartReplay = !startDay.isEmpty() && !endDay.isEmpty();

        auto pReplayer = new sinyee_replayer(REPLAYER_DBUS_SERVICE, REPLAYER_DBUS_OBJECT, QDBusConnection::sessionBus());
        if (source == SINYEE_REPLAYER || (pReplayer->isValid() && source == AUTO_SELECT)) {
            marketSource = SINYEE_REPLAYER;
            auto manager = new QuantTraderManagerReplay<sinyee_replayer, QuantTrader, trade_executer>(pReplayer, pTrader, nullptr);
            if (autoStartReplay) {
                manager->setAutoReplayDate(startDay, endDay);
            }
            d = manager;
        } else {
            marketSource = MARKET_WATCHER;
            delete pReplayer;
            auto pWatcher = new market_watcher(WATCHER_DBUS_SERVICE, WATCHER_DBUS_OBJECT, QDBusConnection::sessionBus());
            auto manager = new QuantTraderManagerReplay<market_watcher, QuantTrader, trade_executer>(pWatcher, pTrader, nullptr);
            if (autoStartReplay) {
                manager->setAutoReplayDate(startDay, endDay);
            }
            d = manager;
        }
    } else {
        marketSource = MARKET_WATCHER;
        auto pWatcher = new market_watcher(WATCHER_DBUS_SERVICE, WATCHER_DBUS_OBJECT, QDBusConnection::sessionBus());
        d = new QuantTraderManagerReal<market_watcher, QuantTrader, trade_executer>(pWatcher, pTrader, nullptr);
    }
    d->init();
}

QuantTraderManagerDbus::~QuantTraderManagerDbus()
{
    if (d) {
        auto *pSource = d->getSource();
        auto *pExecuter = d->getExecuter();
        delete d;
        if (pSource) {
            delete pSource;
        }
        if (pExecuter) {
            delete pExecuter;
        }
    }
}
