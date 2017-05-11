#include "depth_market.h"
#include "option_pricing.h"
#include "high_frequency.h"

HighFrequency::HighFrequency(OptionPricing *pPricing, DepthMarketCollection *pDMC) :
    BaseStrategy(pDMC),
    pPricingEngine(pPricing)
{
}

HighFrequency::~HighFrequency()
{
}

void HighFrequency::onUnderlyingChanged(int underlyingIdx)
{
}

void HighFrequency::onOptionChanged(int underlyingIdx, OPTION_TYPE type, int kIdx)
{
}
