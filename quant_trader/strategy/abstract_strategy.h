#ifndef ABSTRACT_STRATEGY_H
#define ABSTRACT_STRATEGY_H

#include <boost/optional.hpp>

#include "../bar.h"
#include "../indicator/indicator_functions.h"

class AbstractStrategy : public IndicatorFunctions
{
    Q_OBJECT

protected:
    const QString strategyID;
    const QString instrument;
    const QString time_frame_str;

    QList<Bar> *barList;
    Bar* lastBar;
    _ListProxy<Bar> bars;

    boost::optional<int> position;
    boost::optional<double> tp_price;
    boost::optional<double> sl_price;

    qint64 lastCalcualtedBarTime;
    bool isNewBar() const;
    void resetPosition();
    void loadStatus();
    void saveStatus();
    virtual void checkTPSL(double price);
    virtual void onNewBar() = 0;

public:
    explicit AbstractStrategy(const QString& id, const QString& instrumentID, const QString& time_frame, QObject *parent = 0);
    ~AbstractStrategy();

    boost::optional<int> getPosition() const {
        return position;
    }

    // Should call setBarList after setParameter
    virtual void setBarList(QList<Bar> *list, Bar *last);

    // Inherit from AbstractStrategy and override following virtual functions
    virtual void setParameter(const QVariant& param1, const QVariant& param2, const QVariant& param3,
                              const QVariant& param4, const QVariant& param5, const QVariant& param6,
                              const QVariant& param7, const QVariant& param8, const QVariant& param9) = 0;
    virtual void checkIfNewBar();
    virtual void onNewTick(int time, double lastPrice);

signals:

public slots:
};

#endif // ABSTRACT_STRATEGY_H
