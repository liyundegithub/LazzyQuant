#include "depth_market.h"
#include "base_strategy.h"

BaseStrategy::BaseStrategy(DepthMarketCollection *pDMC) :
    pDepthMarkets(pDMC)
{
    //
}

BaseStrategy::~BaseStrategy()
{
    //
}

void BaseStrategy::buyInstrument(int idx, int vol)
{
    const QString instrumentID = pDepthMarkets->getInstrumentByIdx(idx);
    //pExecuter->buyMarketAuto(underlyingID, vol);
    qDebug() << "Buy" << instrumentID << vol;
}

void BaseStrategy::sellInstrument(int idx, int vol)
{
    const QString instrumentID = pDepthMarkets->getInstrumentByIdx(idx);
    //pExecuter->sellMarketAuto(underlyingID, vol);
    qDebug() << "Sell" << instrumentID << vol;
}
