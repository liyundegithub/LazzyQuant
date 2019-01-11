#ifndef AMA_H
#define AMA_H

#include "mql5_indicator.h"

class AMA : public MQL5IndicatorOnSingleDataBuffer
{
    Q_OBJECT
    Q_CLASSINFO("parameter_number", "5")
    Q_PROPERTY(int PeriodAMA MEMBER ExtPeriodAMA CONSTANT)
    Q_PROPERTY(int FastPeriodEMA MEMBER ExtFastPeriodEMA CONSTANT)
    Q_PROPERTY(int SlowPeriodEMA MEMBER ExtSlowPeriodEMA CONSTANT)
    Q_PROPERTY(int ShiftAMA MEMBER InpShiftAMA CONSTANT)

public:
    Q_INVOKABLE explicit AMA(int InpPeriodAMA, int InpFastPeriodEMA, int InpSlowPeriodEMA, int InpShiftAMA, ENUM_APPLIED_PRICE AppliedPrice, QObject *parent = 0);

    void OnInit() override;

protected:
    int InpShiftAMA;
    double ExtFastSC;
    double ExtSlowSC;
    int ExtPeriodAMA;
    int ExtSlowPeriodEMA;
    int ExtFastPeriodEMA;

    IndicatorBuffer<double> ExtAMABuffer;

    int OnCalculate (const int rates_total,                     // size of the price[] array
                     const int prev_calculated,                 // bars handled on a previous call
                     const int begin,                           // where the significant data start from
                     const _TimeSeries<double>& price           // array to calculate
                     ) override;

    template<class T>
    double CalculateER(const int nPosition,const T &PriceData);

};

#endif // AMA_H
