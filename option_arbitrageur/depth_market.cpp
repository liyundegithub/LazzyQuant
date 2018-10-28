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

DepthMarketCollection::DepthMarketCollection(const QMultiMap<QString, int> &underlyingKMap) :
    OptionIndex(underlyingKMap)
{
    pUnderlyingMarket = new DepthMarket[underlyingNum];
    pCallOptionMarket = new DepthMarket[underlyingNum * kNum];
    pPutOptionMarket  = new DepthMarket[underlyingNum * kNum];

    ppCallOption = (DepthMarket **) malloc(underlyingNum * sizeof(DepthMarket*));
    ppPutOption  = (DepthMarket **) malloc(underlyingNum * sizeof(DepthMarket*));

    for (int i = 0; i < underlyingNum; i++) {
        ppCallOption[i] = pCallOptionMarket + i * kNum;
        ppPutOption [i] = pPutOptionMarket  + i * kNum;
    }
}

DepthMarketCollection::~DepthMarketCollection()
{
    free(ppCallOption);
    free(ppPutOption);

    delete[] pUnderlyingMarket;
    delete[] pCallOptionMarket;
    delete[] pPutOptionMarket;
}

DepthMarket DepthMarketCollection::getUnderlyingDepthMarket(const QString &underlyingID) const
{
    int underlyingIdx = getIdxByUnderlyingID(underlyingID);
    return getUnderlyingDepthMarketByIdx(underlyingIdx);
}

DepthMarket DepthMarketCollection::getOptionDepthMarket(const QString &underlyingID, OPTION_TYPE type, int K) const
{
    int underlyingIdx = getIdxByUnderlyingID(underlyingID);
    int kIdx = getIdxByK(K);
    return getOptionDepthMarketByIdx(underlyingIdx, type, kIdx);
}

DepthMarket DepthMarketCollection::getOptionDepthMarketByIdx(int underlyingIdx, OPTION_TYPE type, int kIdx) const
{
    auto **ppOption = (type == CALL_OPT) ? ppCallOption : ppPutOption;
    return ppOption[underlyingIdx][kIdx];
}

void DepthMarketCollection::takeLiquidity(const QString &instrumentID, bool ask)
{
    if (isOption(instrumentID)) {
        QString underlyingID;
        OPTION_TYPE type;
        int exercisePrice;
        if (parseOptionID(instrumentID, underlyingID, type, exercisePrice)) {
            takeLiquidity(underlyingID, type, exercisePrice, ask);
        }
    } else {
        int underlyingIdx = getIdxByUnderlyingID(instrumentID);
        takeLiquidityByIdx(underlyingIdx, ask);
    }
}

void DepthMarketCollection::takeLiquidityByIdx(int underlyingIdx, bool ask)
{
    if (ask) {
        pUnderlyingMarket[underlyingIdx].askVolume = 0;
    } else {
        pUnderlyingMarket[underlyingIdx].bidVolume = 0;
    }
}

void DepthMarketCollection::takeLiquidity(const QString &underlyingID, OPTION_TYPE type, int K, bool ask)
{
    int underlyingIdx = getIdxByUnderlyingID(underlyingID);
    int kIdx = getIdxByK(K);
    takeLiquidityByIdx(underlyingIdx, type, kIdx, ask);
}

void DepthMarketCollection::takeLiquidityByIdx(int underlyingIdx, OPTION_TYPE type, int kIdx, bool ask)
{
    auto **ppOption = (type == CALL_OPT) ? ppCallOption : ppPutOption;

    if (ask) {
        ppOption[underlyingIdx][kIdx].askVolume = 0;
    } else {
        ppOption[underlyingIdx][kIdx].bidVolume = 0;
    }
}

void DepthMarketCollection::clearAll()
{
    memset(pUnderlyingMarket, 0, underlyingNum * (sizeof(DepthMarket)));
    memset(pCallOptionMarket, 0, underlyingNum * kNum * (sizeof(DepthMarket)));
    memset(pPutOptionMarket,  0, underlyingNum * kNum * (sizeof(DepthMarket)));
}
