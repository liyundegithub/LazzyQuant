#ifndef ABSTRACT_STRATEGY_H
#define ABSTRACT_STRATEGY_H

#include <QObject>

#include <boost/optional.hpp>

class Bar;
class AbstractIndicator;

class AbstractStrategy : public QObject
{
    Q_OBJECT
protected:
    const QString strategyID;
    const QString instrument;
    const QString time_frame_str;

    QList<AbstractIndicator*> indicators;
    QList<Bar> *barList;
    Bar *lastBar;

    boost::optional<int> position;
    boost::optional<double> tp_price;
    boost::optional<double> sl_price;

    uint lastCalcualtedBarTime;
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
    void setBarList(QList<Bar> *list, Bar *last) {
        barList = list;
        lastBar = last;
    }

    // Inherit from AbstractStrategy and override following virtual functions
    virtual void setParameter(const QVariant& param1, const QVariant& param2, const QVariant& param3,
                              const QVariant& param4, const QVariant& param5, const QVariant& param6,
                              const QVariant& param7, const QVariant& param8, const QVariant& param9) = 0;
    virtual void checkIfNewBar();
    virtual void onNewTick(uint time, double lastPrice);

signals:

public slots:
};

#endif // ABSTRACT_STRATEGY_H
