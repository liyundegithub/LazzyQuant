#include <functional>

#include "depth_market.h"
#include "base_strategy.h"

#include "trade_executer_interface.h"

extern std::function<void(const QString&, int, double, int)> buyLimit;
extern std::function<void(const QString&, int, double, int)> sellLimit;

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
    buyLimit(underlyingID, vol, price, orderType);
}

void BaseStrategy::sellUnderlying(int underlyingIdx, int vol, double price, int orderType)
{
    const QString underlyingID = pDepthMarkets->getUnderlyingIDByIdx(underlyingIdx);
    sellLimit(underlyingID, vol, price, orderType);
}

void BaseStrategy::buyOption(int underlyingIdx, OPTION_TYPE type, int kIdx, int vol, double price, int orderType)
{
    const QString optionID = pDepthMarkets->makeOptionByIdx(underlyingIdx, type, kIdx);
    buyLimit(optionID, vol, price, orderType);
}

void BaseStrategy::sellOption(int underlyingIdx, OPTION_TYPE type, int kIdx, int vol, double price, int orderType)
{
    const QString optionID = pDepthMarkets->makeOptionByIdx(underlyingIdx, type, kIdx);
    sellLimit(optionID, vol, price, orderType);
}
