#ifndef FRACTAL_H
#define FRACTAL_H

#include "mql5_indicator.h"

class Fractal : public MQL5Indicator
{
    Q_OBJECT
    Q_CLASSINFO("parameter_number", "0")

    int ExtArrowShift = -10;

    enum {
        ONE,
        TWO,
        THREE,
        FOUR,
    } upState = ONE, dnState = ONE;

public:
    Q_INVOKABLE explicit Fractal(QObject *parent = 0);

    void OnInit() override;

    IndicatorBuffer<double> ExtUpperBuffer;
    IndicatorBuffer<double> ExtLowerBuffer;
    IndicatorBuffer<int> confirmedUpIndex;
    IndicatorBuffer<int> confirmedDnIndex;

protected:
    double max2, max3, min2, min3;
    int max3Idx, min3Idx;

    void preCalculate() override;
    int OnCalculate (const int rates_total,                     // size of input time series
                     const int prev_calculated,                 // bars handled in previous call
                     const _TimeSeries<qint64>& time,           // Time
                     const _TimeSeries<double>& open,           // Open
                     const _TimeSeries<double>& high,           // High
                     const _TimeSeries<double>& low,            // Low
                     const _TimeSeries<double>& close,          // Close
                     const _TimeSeries<qint64>& tick_volume,    // Tick Volume
                     const _TimeSeries<qint64>& volume,         // Real Volume
                     const _TimeSeries<int>& spread             // Spread
                     ) override;
};

#endif // FRACTAL_H
