#ifndef BASE_STRATEGY_H
#define BASE_STRATEGY_H

#include "common_utility.h"

class DepthMarketCollection;

class BaseStrategy
{
protected:
    QString strategyID;
    DepthMarketCollection *pDepthMarkets;
    int position;

    void loadPosition();
    void savePosition();

    void buyInstrument(int idx, int vol);
    void sellInstrument(int idx, int vol);

public:
    BaseStrategy(const QString &id, DepthMarketCollection *pDMC);
    virtual ~BaseStrategy();

    virtual void onInstrumentChanged(int idx) = 0;
};

#endif // BASE_STRATEGY_H
