#ifndef AWESOME_OSCILLATOR_H
#define AWESOME_OSCILLATOR_H

#include "mql5_indicator.h"

class AwesomeOscillator : public MQL5Indicator
{
    Q_OBJECT
    Q_CLASSINFO("parameter_number", "0")

public:
    Q_INVOKABLE explicit AwesomeOscillator(QObject *parent = 0);

    void OnInit() override;

protected:
    MQL5Indicator* ExtFastSMAHandle;
    MQL5Indicator* ExtSlowSMAHandle;

    IndicatorBuffer<double> ExtAOBuffer;
    IndicatorBuffer<double> ExtColorBuffer;
    IndicatorBuffer<double> ExtFastBuffer;
    IndicatorBuffer<double> ExtSlowBuffer;

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

#endif // AWESOME_OSCILLATOR_H
