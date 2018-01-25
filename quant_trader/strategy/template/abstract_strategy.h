#ifndef ABSTRACT_STRATEGY_H
#define ABSTRACT_STRATEGY_H

#include <boost/optional.hpp>
#include <QMap>
#include <QStringList>

#include "../../bar.h"

class QVariant;

class AbstractStrategy
{
protected:
    const QString strategyID;
    const QString instrument;
    const QList<int> timeFrames;

    boost::optional<int> position;

public:
    AbstractStrategy(const QString &id, const QString &instrumentID, const QList<int> &timeFrames) :
        strategyID(id), instrument(instrumentID), timeFrames(timeFrames) {}
    virtual ~AbstractStrategy() {}

    virtual void setBarList(const QMap<int, QPair<QList<Bar>*, Bar*>>&) {}

    virtual void loadStatus() = 0;
    virtual void saveStatus() = 0;

    virtual void resetPosition() = 0;
    boost::optional<int> getPosition() const {
        return position;
    }

    virtual void setParameter(const QVariant &param1, const QVariant &param2, const QVariant &param3,
                              const QVariant &param4, const QVariant &param5, const QVariant &param6,
                              const QVariant &param7, const QVariant &param8, const QVariant &param9) = 0;

    virtual void checkIfNewBar(int) {}
    virtual void onNewTick(int time, double lastPrice) = 0;

};

#endif // ABSTRACT_STRATEGY_H
