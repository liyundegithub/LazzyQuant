#include "depth_market.h"

#include <QDateTime>
#include <QTimeZone>
#include <QDebugStateSaver>

#define TOLERANCE   0.000001

DepthMarket::DepthMarket() :
    time(0),
    lastPrice(0.0),
    askPrice(0.0),
    askVolume(0),
    bidPrice(0.0),
    bidVolume(0)
{
    // default ctor
}

DepthMarket::DepthMarket(qint64 time, double lastPrice, double askPrice, int askVolume, double bidPrice, int bidVolume) :
    time(time),
    lastPrice(lastPrice),
    askPrice(askPrice),
    askVolume(askVolume),
    bidPrice(bidPrice),
    bidVolume(bidVolume)
{
    //
}

bool DepthMarket::significantChange(const DepthMarket &other) const
{
    bool asksChanged = qAbs(this->askPrice - other.askPrice) > TOLERANCE;
    bool bidsChanged = qAbs(this->bidPrice - other.bidPrice) > TOLERANCE;
    return (asksChanged || bidsChanged);
}

QDebug operator<<(QDebug dbg, const DepthMarket &depthMarket)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Ask 1:\t" << depthMarket.askPrice << '\t' << depthMarket.askVolume << '\n'
                  << " ------ " << QDateTime::fromSecsSinceEpoch(depthMarket.time, QTimeZone::utc()).toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")) << " lastPrice:" << depthMarket.lastPrice << " ------ " << '\n'
                  << "Bid 1:\t" << depthMarket.bidPrice << '\t' << depthMarket.bidVolume;
    return dbg;
}

DepthMarketCollection::DepthMarketCollection(const QStringList &instruments) :
    instrumentList(instruments)
{
    num = instruments.length();
    pMarket = new DepthMarket[num];

    for (int i = 0; i < num; i++) {
        instrumentIDToIdxMap.insert(instruments[i], i);
    }
}

DepthMarketCollection::~DepthMarketCollection()
{
    delete[] pMarket;
}

DepthMarket DepthMarketCollection::getDepthMarket(const QString &instrumentID) const
{
    int idx = getIdxByInstrument(instrumentID);
    return getDepthMarketByIdx(idx);
}

void DepthMarketCollection::takeLiquidity(const QString &instrumentID, bool ask)
{
    int idx = getIdxByInstrument(instrumentID);
    takeLiquidityByIdx(idx, ask);
}

void DepthMarketCollection::takeLiquidityByIdx(int idx, bool ask)
{
    if (ask) {
        pMarket[idx].askVolume = 0;
    } else {
        pMarket[idx].bidVolume = 0;
    }
}

void DepthMarketCollection::clearAll()
{
    memset(pMarket, 0, num * (sizeof(DepthMarket)));
}
