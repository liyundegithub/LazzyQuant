#include <QDebug>

#include "config.h"
#include "strategy_status.h"
#include "abstract_strategy.h"
#include "../bar.h"
#include "../indicator/abstract_indicator.h"

extern StrategyStatusManager *pStatusManager;

AbstractStrategy::AbstractStrategy(const QString &id, const QString& instrumentID, const QString& time_frame, QObject *parent) :
    IndicatorFunctions(parent),
    strategyID(id),
    instrument(instrumentID),
    time_frame_str(time_frame)
{
    qDebug() << "AbstractStrategy ctor, id =" << id << ", instrumentID =" << instrumentID << ", time_frame =" << time_frame;

    loadStatus();

    lastCalcualtedBarTime = -1;
}

AbstractStrategy::~AbstractStrategy()
{
    qDebug() << "~AbstractStrategy dtor, id =" << strategyID << ", instrumentID =" << instrument << ", time_frame =" << time_frame_str;
}

inline bool AbstractStrategy::isNewBar() const
{
    return (lastBar->time != lastCalcualtedBarTime);
}

void AbstractStrategy::loadStatus()
{
    const auto status = pStatusManager->getStatus(strategyID);
    position = status.position;
    tp_price = status.takeProfit;
    sl_price = status.stopLoss;

    qDebug() << "Load status:" << strategyID;
    qDebug() << status;
}

void AbstractStrategy::saveStatus()
{
    const StrategyStatus status = {position, tp_price, sl_price};

    qDebug() << "Save status:" << strategyID;
    qDebug() << status;

    pStatusManager->setStatus(strategyID, status);
}

inline void AbstractStrategy::resetPosition()
{
    position = 0;
    tp_price.reset();
    sl_price.reset();
    saveStatus();
}

void AbstractStrategy::checkTPSL(double price)
{
    if (!position.is_initialized()) {
        // No position
        return;
    }

    int position_value = position.get();

    // Check take profit
    if (tp_price.is_initialized()) {
        if (position_value > 0 && price > tp_price.get()) {
            resetPosition();
        }
        if (position_value < 0 && price < tp_price.get()) {
            resetPosition();
        }
    }

    // Check stop loss
    if (sl_price.is_initialized()) {
        if (position_value > 0 && price < sl_price.get()) {
            resetPosition();
        }
        if (position_value < 0 && price > sl_price.get()) {
            resetPosition();
        }
    }
}

void AbstractStrategy::checkIfNewBar()
{
    if (isNewBar()) {
        for (auto* indicator : qAsConst(dependIndicators)) {
            indicator->update();
        }
        onNewBar();
        if (position.is_initialized()) {
            saveStatus();
        }
        lastCalcualtedBarTime = lastBar->time;
    }
}

void AbstractStrategy::onNewTick(int time, double lastPrice)
{
    Q_UNUSED(time)
    checkTPSL(lastPrice);
}
