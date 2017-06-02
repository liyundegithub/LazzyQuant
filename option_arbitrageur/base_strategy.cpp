#include "depth_market.h"
#include "base_strategy.h"

#include "ctp_executer.h"

extern CtpExecuter *pExecuter;

BaseStrategy::BaseStrategy(DepthMarketCollection *pDMC) :
    pDepthMarkets(pDMC)
{
    //
}

BaseStrategy::~BaseStrategy()
{
    //
}

void BaseStrategy::buyUnderlying(int underlyingIdx, int vol, double price, int orderType)
{
    const QString underlyingID = pDepthMarkets->getUnderlyingIDByIdx(underlyingIdx);
    pExecuter->buyLimit(underlyingID, vol, price, true, orderType);
}

void BaseStrategy::sellUnderlying(int underlyingIdx, int vol, double price, int orderType)
{
    const QString underlyingID = pDepthMarkets->getUnderlyingIDByIdx(underlyingIdx);
    pExecuter->sellLimit(underlyingID, vol, price, true, orderType);
}

void BaseStrategy::buyOption(int underlyingIdx, OPTION_TYPE type, int kIdx, int vol, double price, int orderType)
{
    const QString optionID = pDepthMarkets->makeOptionByIdx(underlyingIdx, type, kIdx);
    pExecuter->buyLimit(optionID, vol, price, true, orderType);
}

void BaseStrategy::sellOption(int underlyingIdx, OPTION_TYPE type, int kIdx, int vol, double price, int orderType)
{
    const QString optionID = pDepthMarkets->makeOptionByIdx(underlyingIdx, type, kIdx);
    pExecuter->sellLimit(optionID, vol, price, true, orderType);
}
