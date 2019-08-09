#include <functional>
#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "connection_manager.h"
#include "multiple_timer.h"
#include "message_handler.h"
#include "option_arbitrageur_options.h"
#include "option_helper.h"
#include "option_arbitrageur.h"

#include "market_watcher_interface.h"
#include "tick_replayer_interface.h"
#include "trade_executer_interface.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

using BUY_SELL_LIMIT_4_PARAM = QDBusPendingReply<> (com::lazzyquant::trade_executer::*)(const QString &, int, double, int);

std::function<void(const QString&, int, double, int)>  buyLimit = [](auto, auto, auto, auto) -> void {};
std::function<void(const QString&, int, double, int)> sellLimit = [](auto, auto, auto, auto) -> void {};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("option_arbitrageur");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Find arbitrageur opportunity and call executer automatically.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions(optionArbitrageurOptions);
    parser.process(a);

    auto options = getOptionArbitrageurOptions(parser);
    if (options.replayMode && options.update) {
        qCritical().noquote() << "Can not do update in replay mode!";
        return -1;
    }
    setupMessageHandler(true, options.log2File, "option_arbitrageur");

    com::lazzyquant::market_watcher *pWatcher = nullptr;
    com::lazzyquant::tick_replayer *pReplayer = nullptr;
    com::lazzyquant::trade_executer *pExecuter = nullptr;
    QObject *pSource = nullptr;
    QStringList instruments;
    if (options.replayMode) {
        pReplayer = new com::lazzyquant::tick_replayer(REPLAYER_DBUS_SERVICE, REPLAYER_DBUS_OBJECT, QDBusConnection::sessionBus());
        instruments = pReplayer->getReplayList();
        pSource = pReplayer;
    } else {
        pWatcher = new com::lazzyquant::market_watcher(WATCHER_DBUS_SERVICE, WATCHER_DBUS_OBJECT, QDBusConnection::sessionBus());
        pExecuter = new com::lazzyquant::trade_executer(EXECUTER_DBUS_SERVICE, EXECUTER_DBUS_OBJECT, QDBusConnection::sessionBus());
        instruments = pExecuter->getCachedInstruments();
        pSource = pWatcher;
    }
    OptionHelper helper(pExecuter);
    OptionArbitrageur arbitrageur(instruments, &helper);
    MultipleTimer *marketOpenTimer = nullptr;
    MultipleTimer *marketCloseTimer = nullptr;

    if (!options.replayMode) {
        if (options.update) {
            marketOpenTimer = new MultipleTimer({{8, 30}, {20, 30}});
            QObject::connect(marketOpenTimer, &MultipleTimer::timesUp, [pWatcher, pExecuter, &arbitrageur]() -> void {
                if ((pWatcher->getStatus() != "Ready") || (pExecuter->getStatus() != "Ready")) {
                    qCritical().noquote() << "Market watcher or trade executer is not ready!";
                    return;
                }
                const QStringList subscribedInstruments = pWatcher->getSubscribeList();
                const QStringList cachedInstruments = pExecuter->getCachedInstruments();
                const QSet<QString> underlyingIDs = arbitrageur.getUnderlyingIDs();
                QStringList instrumentsToSubscribe;
                for (const auto &item : cachedInstruments) {
                    if (!subscribedInstruments.contains(item)) {
                        for (const auto &underlyingID : qAsConst(underlyingIDs)) {
                            if (item.startsWith(underlyingID)) {
                                instrumentsToSubscribe << item;
                                break;
                            }
                        }
                    }
                }
                if (!instrumentsToSubscribe.empty()) {
                    pWatcher->subscribeInstruments(instrumentsToSubscribe);
                }
            });
        } else {
            buyLimit = std::bind((BUY_SELL_LIMIT_4_PARAM)&com::lazzyquant::trade_executer::buyLimit, pExecuter, _1, _2, _3, _4);
            sellLimit = std::bind((BUY_SELL_LIMIT_4_PARAM)&com::lazzyquant::trade_executer::sellLimit, pExecuter, _1, _2, _3, _4);
            marketOpenTimer = new MultipleTimer({{8, 30}, {20, 30}});
            QObject::connect(marketOpenTimer, &MultipleTimer::timesUp, [pWatcher, &arbitrageur]() -> void {
                if (pWatcher->getStatus() != "Ready") {
                    qCritical().noquote() << "Market watcher is not ready!";
                    return;
                }
                QString tradingDay = pWatcher->getTradingDay();
                arbitrageur.setTradingDay(tradingDay);
            });
            marketCloseTimer = new MultipleTimer({{15, 5}});
            QObject::connect(marketCloseTimer, &MultipleTimer::timesUp, &arbitrageur, &OptionArbitrageur::onMarketClose);
        }
    }
    ConnectionManager *pConnManager = nullptr;
    if (!options.update) {
        pConnManager = new ConnectionManager({pSource}, {&arbitrageur});
    }
    if (options.replayMode) {
        pReplayer->startReplay(options.replayDate);
    }

    int ret = a.exec();
    if (marketCloseTimer) {
        marketCloseTimer->disconnect();
        delete marketCloseTimer;
    }
    if (marketOpenTimer) {
        marketOpenTimer->disconnect();
        delete marketOpenTimer;
    }
    delete pConnManager;
    delete pExecuter;
    delete pReplayer;
    delete pWatcher;
    restoreMessageHandler();
    return ret;
}
