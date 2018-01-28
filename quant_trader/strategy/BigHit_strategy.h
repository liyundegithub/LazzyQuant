#ifndef BIGHIT_STRATEGY_H
#define BIGHIT_STRATEGY_H

#include "template/single_time_frame_strategy.h"

#include <QQueue>
#include <QPair>

class BigHitStrategy : public SingleTimeFrameStrategy
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit BigHitStrategy(const QString &id, const QString &instrumentID, int timeFrame, QObject *parent);

    void setParameter(const QVariant &param1, const QVariant &param2, const QVariant &param3,
                      const QVariant &param4, const QVariant &param5, const QVariant &param6,
                      const QVariant &param7, const QVariant &param8, const QVariant &param9) override;
    void setParameter(int deltaTime, double deltaPrice, int timeOut);
    void onNewBar();
    void onNewTick(int time, double lastPrice);

protected:
    int dT;
    double dP;
    int tO;

    bool biggestVolEver;
    int expireTime;
    int bigHit;
    QQueue<QPair<int, double>> recentPrices;
};

#endif // BIGHIT_STRATEGY_H
