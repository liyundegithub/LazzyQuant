#ifndef DEPTH_MARKET_H
#define DEPTH_MARKET_H

#include <QDebug>

class DepthMarket {
public:
    int   time;
    double lastPrice;
    double askPrice;
    int    askVolume;
    double bidPrice;
    int    bidVolume;

    DepthMarket();
    DepthMarket(int time, double lastPrice, double askPrice, int askVolume, double bidPrice, int bidVolume);
    bool significantChange(const DepthMarket &other) const;
    bool isUpperLimit() const { return bidVolume == 0; }
    bool isLowerLimit() const { return askVolume == 0; }
};

QDebug operator<<(QDebug dbg, const DepthMarket &depthMarket);

class DepthMarketCollection
{
public:
    DepthMarket *pMarket;
    int num;
    QStringList instrumentList;
    QMap<QString, int> instrumentIDToIdxMap;

    explicit DepthMarketCollection(const QStringList &instruments);
    ~DepthMarketCollection();

    int getNum() const { return num; }

    QString getInstrumentByIdx(int idx) const { return instrumentList[idx]; }
    int getIdxByInstrument(const QString &instrumentID) const { return instrumentIDToIdxMap.value(instrumentID, -1); }

    DepthMarket getDepthMarket(const QString &instrumentID) const;
    DepthMarket getDepthMarketByIdx(int idx) const { return pMarket[idx]; }

    void takeLiquidity(const QString &instrumentID, bool ask);
    void takeLiquidityByIdx(int idx, bool ask);
    void clearAll();
};

#endif // DEPTH_MARKET_H
