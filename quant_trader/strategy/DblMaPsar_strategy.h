#ifndef DBLMAPSAR_STRATEGY_H
#define DBLMAPSAR_STRATEGY_H

#include "template/single_time_frame_strategy.h"

class DblMaPsarStrategy : public SingleTimeFrameStrategy
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit DblMaPsarStrategy(const QString &id, const QString &instrumentID, int timeFrame, QObject *parent = nullptr);

    void setParameter(const QVariant &param1, const QVariant &param2, const QVariant &param3,
                      const QVariant &param4, const QVariant &param5, const QVariant &param6,
                      const QVariant &param7, const QVariant &param8, const QVariant &param9) override;
    void setParameter(int fastPeriod, int slowPeriod, ENUM_MA_METHOD ma_method, ENUM_APPLIED_PRICE applied_price, double SARStep, double SARMaximum);
    void onNewBar() override;

protected:
    MQL5Indicator *fast_ma;
    MQL5Indicator *slow_ma;
    MQL5Indicator *psar;

};

#endif // DBLMAPSAR_STRATEGY_H
