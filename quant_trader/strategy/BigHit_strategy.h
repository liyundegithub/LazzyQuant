#ifndef BIGHIT_STRATEGY_H
#define BIGHIT_STRATEGY_H

#include "abstract_strategy.h"

#include <QQueue>
#include <QPair>

class BigHitStrategy : public AbstractStrategy
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit BigHitStrategy(const QString &id, const QString &instrumentID, const QString &time_frame, QObject *parent);

    void setParameter(const QVariant& param1, const QVariant& param2, const QVariant& param3,
                      const QVariant& param4, const QVariant& param5, const QVariant& param6,
                      const QVariant& param7, const QVariant& param8, const QVariant& param9);
    void setParameter(int deltaTime, double deltaPrice, int timeOut);
    void onNewBar();
    void onNewTick(uint time, double lastPrice);

protected:
    int dT;
    double dP;
    int tO;

    bool biggestVolEver;
    uint expireTime;
    int bigHit;
    QQueue<QPair<uint, double>> recentPrices;
};

#endif // BIGHIT_STRATEGY_H
