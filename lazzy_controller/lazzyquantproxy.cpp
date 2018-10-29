#include <cfloat>
#include <QTimer>

#include "lazzyquantproxy.h"

#include "market_watcher_interface.h"
#include "sinyee_replayer_interface.h"
#include "trade_executer_interface.h"
#include "quant_trader_interface.h"

#include "config.h"

LazzyQuantProxy::LazzyQuantProxy(QObject *parent) :
    QObject(parent)
{
    pWatcher = new com::lazzyquant::market_watcher(WATCHER_DBUS_SERVICE, WATCHER_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
    pExecuter = new com::lazzyquant::trade_executer(EXECUTER_DBUS_SERVICE, EXECUTER_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
    pReplayer = new com::lazzyquant::sinyee_replayer(REPLAYER_DBUS_SERVICE, REPLAYER_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
    pTrader = new com::lazzyquant::quant_trader(TRADER_DBUS_SERVICE, TRADER_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
    connect(pTrader, SIGNAL(newBarFormed(QString,QString)), this, SIGNAL(newBarFormed(QString,QString)));

    QTimer *timer = new QTimer(this);
    timer->setInterval(1000);
    timer->setSingleShot(false);
    timer->setTimerType(Qt::CoarseTimer);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer->start();
}

LazzyQuantProxy::~LazzyQuantProxy()
{
}

void LazzyQuantProxy::onTimer()
{
    bool watcherStatus = pWatcher && pWatcher->isValid();
    bool replayerStatus = pReplayer && pReplayer->isValid();
    bool executerStatus = pExecuter && pExecuter->isValid();
    bool traderStatus = pTrader && pTrader->isValid();
    emit moduleStatus(watcherStatus, replayerStatus, executerStatus, traderStatus);
}

double LazzyQuantProxy::getAvailable()
{
    if (pExecuter->isValid()) {
        pExecuter->updateAccountInfo();
        return pExecuter->getAvailable();
    } else {
        return -DBL_MAX;
    }
}

void LazzyQuantProxy::buyMarket(const QString &instrument, int volume, const QString &strategyID)
{
    // TODO
}

void LazzyQuantProxy::sellMarket(const QString &instrument, int volume, const QString &strategyID)
{
    // TODO
}

void LazzyQuantProxy::onModified(const QString &name)
{
    pTrader->onModified(name);
}

int LazzyQuantProxy::getPositionByInstrumentId(const QString &instrument)
{
    if (pTrader->isValid()) {
        return pTrader->getPositionByInstrumentId(instrument);
    } else {
        return -INT_MAX;
    }
}

int LazzyQuantProxy::getPositionByStrategyId(const QString &id)
{
    if (pTrader->isValid()) {
        return pTrader->getPositionByStrategyId(id);
    } else {
        return -INT_MAX;
    }
}

QString LazzyQuantProxy::getInstrumentByStrategyId(const QString &id)
{
    if (pTrader->isValid()) {
        return pTrader->getInstrumentByStrategyId(id);
    } else {
        return QString();
    }
}

QStringList LazzyQuantProxy::getStrategyId(const QString &instrument)
{
    if (pTrader->isValid()) {
        return pTrader->getStrategyId(instrument);
    } else {
        return QStringList();
    }
}

bool LazzyQuantProxy::getStrategyEnabled(const QString &id)
{
    if (pTrader->isValid()) {
        return pTrader->getStrategyEnabled(id);
    } else {
        return false;
    }
}

void LazzyQuantProxy::setStrategyEnabled(const QString &id, bool state)
{
    if (pTrader->isValid()) {
        pTrader->setStrategyEnabled(id, state);
    }
}
