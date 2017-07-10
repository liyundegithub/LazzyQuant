#ifndef BUTTERFLY_H
#define BUTTERFLY_H

#include "base_strategy.h"

class DepthMarket;

class Butterfly : public BaseStrategy
{
public:
    Butterfly(const QString &strategyID, const QStringList &instruments, int maxPosition, int minPosition, double openThreshold, double closeThreshold, DepthMarketCollection *pDMC);
    ~Butterfly();

    void onInstrumentChanged(int idx);

protected:
    int firstIdx;
    int secondIdx;
    int thirdIdx;

    int maxPosition;
    int minPosition;

    double openThreshold;
    double closeThreshold;

    DepthMarket *first;
    DepthMarket *second;
    DepthMarket *third;

    void check010();
    void check101();
};

#endif // BUTTERFLY_H
