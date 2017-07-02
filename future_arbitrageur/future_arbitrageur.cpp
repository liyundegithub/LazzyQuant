#include "config.h"
#include "depth_market.h"
#include "pair_trade.h"
#include "butterfly.h"
#include "future_arbitrageur.h"
#include "sinyee_replayer_interface.h"

#include <QSettings>

com::lazzyquant::sinyee_replayer *pReplayer = nullptr;

FutureArbitrageur::FutureArbitrageur(QObject *parent) : QObject(parent)
{
    setupStrategies();
    pReplayer = new com::lazzyquant::sinyee_replayer(REPLAYER_DBUS_SERVICE, REPLAYER_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
    connect(pReplayer, SIGNAL(newMarketData(QString,uint,double,int,double,int,double,int)), this, SLOT(onMarketData(QString,uint,double,int,double,int,double,int)));
}

FutureArbitrageur::~FutureArbitrageur()
{
    for (auto *pStrategy : qAsConst(pStrategyList)) {
        delete pStrategy;
    }
    delete pMarketCollection;
}

void FutureArbitrageur::setupStrategies()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, ORGANIZATION, "future_arbitrageur");
    const auto strategyIDs = settings.childGroups();

    QStringList allInstruments;
    for (const auto &strategyID : strategyIDs) {
        settings.beginGroup(strategyID);

        QString first = settings.value("First").toString();
        allInstruments << first;
        QString second = settings.value("Second").toString();
        allInstruments << second;

        if (settings.contains("Third")) {
            QString third = settings.value("Third").toString();
            allInstruments << third;
        }
        if (settings.contains("Fourth")) {
            QString fourth = settings.value("Fourth").toString();
            allInstruments << fourth;
        }

        settings.endGroup();
    }

    allInstruments.removeDuplicates();
    std::sort(allInstruments.begin(), allInstruments.end());
    qDebug() << "allInstruments:" << allInstruments;
    pMarketCollection = new DepthMarketCollection(allInstruments);

    for (const auto &strategyID : strategyIDs) {
        settings.beginGroup(strategyID);

        QStringList instruments;
        QString first = settings.value("First").toString();
        QString second = settings.value("Second").toString();
        instruments << first << second;

        int position = settings.value("Position").toInt();
        int maxPosition = settings.value("MaxPosition").toInt();
        int minPosition = settings.value("MinPosition").toInt();
        int openThreshold = settings.value("OpenThreshold").toDouble();
        int closeThreshold = settings.value("CloseThreshold").toDouble();

        QString strategyName = settings.value("Strategy").toString();
        BaseStrategy *pStrategy = nullptr;
        if (strategyName == "Butterfly") {
            QString third = settings.value("Third").toString();
            instruments << third;
            pStrategy = new Butterfly(position, instruments, maxPosition, minPosition, openThreshold, closeThreshold, pMarketCollection);
        } else if (strategyName == "PairTrade") {
            pStrategy = new PairTrade(position, instruments, maxPosition, minPosition, openThreshold, closeThreshold, pMarketCollection);
        }
        if (pStrategy != nullptr) {
            pStrategyList.append(pStrategy);
        }

        settings.endGroup();
    }
}

void FutureArbitrageur::onMarketData(const QString& instrumentID, uint time, double lastPrice, int volume,
                                     double askPrice1, int askVolume1, double bidPrice1, int bidVolume1)
{
    Q_UNUSED(volume)

    static bool limited = false;
    if (limited) {
        return;
    }
    if (askVolume1 <= 0 || bidVolume1 <= 0) {
        qWarning() << "Limited!!!";
        limited = true;
        return;
    }

    const DepthMarket latest_market_data = {
        time,
        lastPrice,
        askPrice1,
        askVolume1,
        bidPrice1,
        bidVolume1
    };

    DepthMarket * pDM = nullptr;

    int idx = pMarketCollection->getIdxByInstrument(instrumentID);
    if (idx == -1) {
        return;
    } else {
        pDM = &(pMarketCollection->pMarket[idx]);
    }

    const bool significantChange = pDM->significantChange(latest_market_data);
    *pDM = latest_market_data;
    if (significantChange) {
        for (auto * pStrategy : qAsConst(pStrategyList)) {
            pStrategy->onInstrumentChanged(idx);
        }
    } else {
        // Market does not change much, do nothing
    }
}
