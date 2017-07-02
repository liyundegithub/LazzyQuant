#ifndef BASE_STRATEGY_H
#define BASE_STRATEGY_H

#include "common_utility.h"

class DepthMarketCollection;

class BaseStrategy
{
protected:
    DepthMarketCollection *pDepthMarkets;

    void buyInstrument(int idx, int vol);
    void sellInstrument(int idx, int vol);

public:
    explicit BaseStrategy(DepthMarketCollection *pDMC);
    virtual ~BaseStrategy();

    virtual void onInstrumentChanged(int idx) = 0;

    DepthMarketCollection* getDepthMarketCollection() const { return pDepthMarkets; }
};

#endif // BASE_STRATEGY_H
