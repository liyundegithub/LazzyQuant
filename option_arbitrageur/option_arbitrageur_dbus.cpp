#include <functional>

#include "config.h"
#include "option_helper.h"
#include "option_arbitrageur.h"
#include "option_arbitrageur_manager.h"
#include "option_arbitrageur_options.h"
#include "option_arbitrageur_dbus.h"

#include "market_watcher_interface.h"
#include "tick_replayer_interface.h"
#include "trade_executer_interface.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

using namespace com::lazzyquant;

std::function<void(const QString&, int, double, int)>  buyLimit = [](auto, auto, auto, auto) -> void {};
std::function<void(const QString&, int, double, int)> sellLimit = [](auto, auto, auto, auto) -> void {};

OptionArbitrageurDbus::OptionArbitrageurDbus(const OptionArbitrageurOptions &options)
{
    market_watcher *pWatcher = nullptr;
    tick_replayer *pReplayer = nullptr;
    trade_executer *pExecuter = nullptr;
    QStringList instruments;
    if (options.replayMode) {
        pReplayer = new tick_replayer(REPLAYER_DBUS_SERVICE, REPLAYER_DBUS_OBJECT, QDBusConnection::sessionBus());
        instruments = pReplayer->getReplayList();
    } else {
        pWatcher = new market_watcher(WATCHER_DBUS_SERVICE, WATCHER_DBUS_OBJECT, QDBusConnection::sessionBus());
        pExecuter = new trade_executer(EXECUTER_DBUS_SERVICE, EXECUTER_DBUS_OBJECT, QDBusConnection::sessionBus());
        instruments = pExecuter->getCachedInstruments();
    }
    pHelper = new OptionHelper(pExecuter);
    OptionArbitrageur *pArbitrageur = new OptionArbitrageur(instruments, pHelper);

    if (options.replayMode) {
        auto managerReplay = new ReplayManager<tick_replayer, OptionArbitrageur, trade_executer>(pReplayer, pArbitrageur, pExecuter);
        if (options.isReplayReady()) {
            managerReplay->setAutoReplayDate(options.replayDate, options.replayDate);
        }
        pManager = managerReplay;
    } else {
        buyLimit = std::bind(qOverload<const QString&, int, double, int>(&trade_executer::buyLimit), pExecuter, _1, _2, _3, _4);
        sellLimit = std::bind(qOverload<const QString&, int, double, int>(&trade_executer::sellLimit), pExecuter, _1, _2, _3, _4);
        pManager = new OptionArbitrageurRealManager<market_watcher, OptionArbitrageur, trade_executer>(pWatcher, pArbitrageur, pExecuter, options.updateOnly);
    }
    pManager->init();
}

OptionArbitrageurDbus::~OptionArbitrageurDbus()
{
    auto pSource = pManager->getSource();
    auto pArbitrageur = pManager->getTrader();
    auto pExecuter = pManager->getExecuter();
    delete pManager;
    delete pSource;
    delete pArbitrageur;
    delete pExecuter;
    delete pHelper;
}
