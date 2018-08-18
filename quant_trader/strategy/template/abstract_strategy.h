#ifndef ABSTRACT_STRATEGY_H
#define ABSTRACT_STRATEGY_H

#include <boost/optional.hpp>
#include <QMap>
#include <QString>

#include "../../bar.h"

class QVariant;

class AbstractStrategy
{
protected:
    QString strategyID;
    QString instrumentID;
    int timeFrames;
    bool enabled = true;

    boost::optional<int> position;

public:
    AbstractStrategy(const QString &id, const QString &instrumentID, int timeFrames) :
        strategyID(id), instrumentID(instrumentID), timeFrames(timeFrames) {}
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

    QString getId() const  { return strategyID; }
    QString getInstrument() const { return instrumentID; }
    bool isEnabled() const  { return enabled; }
    void setEnabled(bool state) { enabled = state; }

};

#endif // ABSTRACT_STRATEGY_H
