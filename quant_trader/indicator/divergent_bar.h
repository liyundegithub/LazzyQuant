#ifndef DIVERGENT_BAR_H
#define DIVERGENT_BAR_H

#include "mql5_indicator.h"

class DivergentBar : public MQL5Indicator
{
    Q_OBJECT
    Q_CLASSINFO("parameter_number", "0")

public:
    Q_INVOKABLE explicit DivergentBar(QObject *parent = 0);

    void OnInit() override;

    IndicatorBuffer<bool> bullishDivergent;
    IndicatorBuffer<bool> bearishDivergent;

protected:
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

#endif // DIVERGENT_BAR_H
