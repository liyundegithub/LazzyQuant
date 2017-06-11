#ifndef RISK_FREE_H
#define RISK_FREE_H

#include "common_utility.h"
#include "base_strategy.h"

class RiskFree : public BaseStrategy
{
public:
    RiskFree(double threshold, DepthMarketCollection *pDMC);
    ~RiskFree();

    void onUnderlyingChanged(int underlyingIdx);
    void onOptionChanged(int underlyingIdx, OPTION_TYPE type, int kIdx);

    const double threshold;

protected:
   // Argitrage strategies
   void findCheapCallOptions(int underlyingIdx);
   void checkCheapCallOptions(int underlyingIdx, int kIdx);
   void findCheapPutOptions(int underlyingIdx);
   void checkCheapPutOptions(int underlyingIdx, int kIdx);

   void findReversedCallOptions(int underlyingIdx, int kIdxToCheck);
   void checkReversedCallOptions(int underlyingIdx, int lowKIdx, int highKIdx);
   void findReversedPutOptions(int underlyingIdx, int kIdxToCheck);
   void checkReversedPutOptions(int underlyingIdx, int lowKIdx, int highKIdx);
};

#endif // RISK_FREE_H
