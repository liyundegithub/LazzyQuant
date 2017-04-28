#include <QMetaEnum>
#include <QSettings>
#include <QStringList>
#include <QDebug>

#include "config.h"
#include "utility.h"
#include "abstract_strategy.h"
#include "../bar.h"
#include "../indicator/abstract_indicator.h"

AbstractStrategy::AbstractStrategy(const QString &id, const QString& instrumentID, const QString& time_frame, QObject *parent) :
    QObject(parent),
    stratety_id(id),
    instrument(instrumentID),
    time_frame_str(time_frame)
{
    qDebug() << "id = " << id << ", instrumentID = " << instrumentID << ", time_frame = " << time_frame;

    result = new QSettings(QSettings::IniFormat, QSettings::UserScope, ORGANIZATION, "strategy_result");
    auto groupList = result->childGroups();
    if (groupList.contains(stratety_id)) {
        result->beginGroup(stratety_id);
        position = result->value("position", 0).toInt();
        if (result->contains("tp_price")) {
            tp_price = result->value("tp_price").toDouble();
        }
        if (result->contains("sl_price")) {
            sl_price = result->value("sl_price").toDouble();
        }
        result->endGroup();
    }

    lastCalcualtedBarTime = -1;
}

AbstractStrategy::~AbstractStrategy()
{
    delete result;
}

inline bool AbstractStrategy::isNewBar() const
{
    return (lastBar->time != lastCalcualtedBarTime);
}

inline void AbstractStrategy::resetPosition()
{
    position = 0;
    tp_price.reset();
    sl_price.reset();
    saveResult();
}

inline void AbstractStrategy::saveResult()
{
    result->beginGroup(stratety_id);
    result->setValue("lastSave", DATE_TIME);
    result->setValue("position", position.get());

    if (tp_price.is_initialized()) {
        result->setValue("tp_price", tp_price.get());
    } else if (result->contains("tp_price")) {
        result->remove("tp_price");
    }

    if (sl_price.is_initialized()) {
        result->setValue("sl_price", sl_price.get());
    } else if (result->contains("sl_price")) {
        result->remove("sl_price");
    }
    result->endGroup();
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
        for (auto* indicator : qAsConst(indicators)) {
            indicator->update();
        }
        onNewBar();
        if (position.is_initialized()) {
            saveResult();
        }
        lastCalcualtedBarTime = lastBar->time;
    }
}

void AbstractStrategy::onNewTick(uint time, double lastPrice)
{
    Q_UNUSED(time)
    checkTPSL(lastPrice);
}
