#ifndef PAIR_TRADE_H
#define PAIR_TRADE_H

#include "base_strategy.h"

class PairTrade : public BaseStrategy
{
public:
    PairTrade(int position, const QStringList &instruments, int maxPosition, int minPosition, double openThreshold, double closeThreshold, DepthMarketCollection *pDMC);
    ~PairTrade();

    void onInstrumentChanged(int idx);

protected:
    int position;

    int firstIdx;
    int secondIdx;
    int thirdIdx;

    int maxPosition;
    int minPosition;

    double openThreshold;
    double closeThreshold;

    DepthMarket *first;
    DepthMarket *second;
};

#endif // PAIR_TRADE_H
