#include "strategy_status.h"
#include "depth_market.h"
#include "base_strategy.h"

#include "trade_executer_interface.h"

extern com::lazzyquant::trade_executer *pExecuter;
extern StrategyStatusManager *pStatusManager;

BaseStrategy::BaseStrategy(const QString &id, DepthMarketCollection *pDMC) :
    strategyID(id),
    pDepthMarkets(pDMC)
{
    loadPosition();
}

BaseStrategy::~BaseStrategy()
{
    qDebug() << "Strategy";
    savePosition();
}

void BaseStrategy::loadPosition()
{
    auto p = pStatusManager->getPosition(strategyID);
    if (p.is_initialized()) {
        position = p.get();
    } else {
        position = 0;
    }
}

void BaseStrategy::savePosition()
{
    pStatusManager->setPosition(strategyID, position);
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
