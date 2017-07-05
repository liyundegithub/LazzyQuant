#include "depth_market.h"
#include "base_strategy.h"

#include "trade_executer_interface.h"

extern com::lazzyquant::trade_executer *pExecuter;

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
    pExecuter->buyMarketAuto(instrumentID, vol);
    qDebug() << "Buy" << instrumentID << vol;
}

void BaseStrategy::sellInstrument(int idx, int vol)
{
    const QString instrumentID = pDepthMarkets->getInstrumentByIdx(idx);
    pExecuter->sellMarketAuto(instrumentID, vol);
    qDebug() << "Sell" << instrumentID << vol;
}
