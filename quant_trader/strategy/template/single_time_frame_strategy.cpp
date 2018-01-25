#include <QTime>
#include <QDebug>

#include "../../bar.h"
#include "../../indicator/abstract_indicator.h"
#include "single_time_frame_strategy.h"

SingleTimeFrameStrategy::SingleTimeFrameStrategy(const QString &id, const QString &instrumentID, int timeFrame, QObject *parent) :
    IndicatorFunctions(parent),
    AbstractStrategy(id, instrumentID, {timeFrame}),
    timeFrame(timeFrame),
    bars(nullptr, nullptr)
{
    qInfo() << "SingleTimeFrameStrategy ctor, id =" << strategyID << ", instrument =" << instrument << ", timeFrame =" << timeFrame;
}

SingleTimeFrameStrategy::~SingleTimeFrameStrategy()
{
    qInfo() << "~SingleTimeFrameStrategy dtor, id =" << strategyID << ", instrument =" << instrument << ", timeFrame =" << timeFrame;
}

void SingleTimeFrameStrategy::loadStatus()
{
    // TODO
    qInfo() << "Loaded status:" << strategyID;
}

void SingleTimeFrameStrategy::saveStatus()
{
    // TODO
    qInfo() << "Saved status:" << strategyID;
}

void SingleTimeFrameStrategy::setPosition(int newPosition)
{
    position = newPosition;
    saveStatus();
}

void SingleTimeFrameStrategy::resetPosition()
{
    position = 0;
    trailingStop.disable();
    saveStatus();
}

void SingleTimeFrameStrategy::checkTPSL(double price)
{
    if (!position.is_initialized()) {
        // No position
        return;
    }

    int position_value = position.get();
    Q_ASSERT((position_value == 0 && !trailingStop.getEnabled()) ||
             (position_value > 0  &&  trailingStop.getEnabled()  &&  trailingStop.getDirection()) ||
             (position_value < 0  &&  trailingStop.getEnabled()  && !trailingStop.getDirection()));

    if (trailingStop.checkStopLoss(price)) {
        resetPosition();
    }
}

void SingleTimeFrameStrategy::setBarList(const QMap<int, QPair<QList<Bar>*, Bar*>> &listAndLast)
{
    this->barList = listAndLast[timeFrame].first;
    this->lastBar = listAndLast[timeFrame].second;
    bars = _ListProxy<Bar>(this->barList, this->lastBar);
    bars.setAsSeries(true);
}

void SingleTimeFrameStrategy::checkIfNewBar(int newBarTimeFrame)
{
    if (this->timeFrame == newBarTimeFrame) {
        for (auto* indicator : qAsConst(dependIndicators)) {
            indicator->update();
        }
        onNewBar();
        trailingStop.update(bars[1].high, bars[1].low);
        if (trailingStop.getEnabled()) {
            qDebug().noquote() << QTime::fromMSecsSinceStartOfDay(bars[0].time % 86400 * 1000).toString() << trailingStop;
        }
        if (position.is_initialized()) {
            // Save even no position change
            saveStatus();
        }
    }
}

void SingleTimeFrameStrategy::onNewTick(int time, double lastPrice)
{
    Q_UNUSED(time)
    checkTPSL(lastPrice);
}
