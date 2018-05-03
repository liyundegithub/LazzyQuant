#ifndef MACD_H
#define MACD_H

#include "mql5_indicator.h"

class Macd : public MQL5IndicatorOnSingleDataBuffer
{
    Q_OBJECT
    Q_CLASSINFO("parameter_number", "4")
    Q_PROPERTY(int FastEMA MEMBER InpFastEMA CONSTANT)
    Q_PROPERTY(int SlowEMA MEMBER InpSlowEMA CONSTANT)
    Q_PROPERTY(int SignalSMA MEMBER InpSignalSMA CONSTANT)

public:
    Q_INVOKABLE explicit Macd(int FastEMA, int SlowEMA, int SignalSMA, ENUM_APPLIED_PRICE appliedPrice = PRICE_CLOSE, QObject *parent = nullptr);

    void OnInit() override;

protected:
    MQL5Indicator* ExtFastMaHandle;
    MQL5Indicator* ExtSlowMaHandle;

    const int InpFastEMA;
    const int InpSlowEMA;
    const int InpSignalSMA;

    IndicatorBuffer<double> ExtMacdBuffer;
    IndicatorBuffer<double> ExtSignalBuffer;
    IndicatorBuffer<double> ExtFastMaBuffer;
    IndicatorBuffer<double> ExtSlowMaBuffer;

    int OnCalculate (const int rates_total,                     // size of the price[] array
                     const int prev_calculated,                 // bars handled on a previous call
                     const int begin,                           // where the significant data start from
                     const _TimeSeries<double>& price           // array to calculate
                     ) override;
};

#endif // MACD_H
