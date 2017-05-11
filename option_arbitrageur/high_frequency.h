#ifndef HIGH_FREQUENCY_H
#define HIGH_FREQUENCY_H

#include "utility.h"
#include "base_strategy.h"

class OptionPricing;

class HighFrequency : public BaseStrategy
{
    const OptionPricing *pPricingEngine;

public:
    HighFrequency(OptionPricing *pPricing, DepthMarketCollection *pDMC);
    ~HighFrequency();

    void onUnderlyingChanged(int underlyingIdx);
    void onOptionChanged(int underlyingIdx, OPTION_TYPE type, int kIdx);

};

#endif // HIGH_FREQUENCY_H
