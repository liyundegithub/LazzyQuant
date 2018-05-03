#include "divergent_bar.h"

DivergentBar::DivergentBar(QObject *parent) :
    MQL5Indicator(0, parent)
{
}

void DivergentBar::OnInit()
{
}

void DivergentBar::preCalculate()
{
    MQL5Indicator::preCalculate();
    this->bullishDivergent.resize(rates_total);
    this->bearishDivergent.resize(rates_total);
}

int DivergentBar::OnCalculate (const int rates_total,                     // size of input time series
                               const int prev_calculated,                 // bars handled in previous call
                               const _TimeSeries<qint64>& time,           // Time
                               const _TimeSeries<double>& open,           // Open
                               const _TimeSeries<double>& high,           // High
                               const _TimeSeries<double>& low,            // Low
                               const _TimeSeries<double>& close,          // Close
                               const _TimeSeries<qint64>& tick_volume,    // Tick Volume
                               const _TimeSeries<qint64>& volume,         // Real Volume
                               const _TimeSeries<int>& spread             // Spread
                               )
{
    Q_UNUSED(time)
    Q_UNUSED(open)
    Q_UNUSED(tick_volume)
    Q_UNUSED(volume)
    Q_UNUSED(spread)

    //--- check for bars count
    if(rates_total < 3)
        return(0);// not enough bars for calculation

    int begin;
    if (prev_calculated < 3) {
        begin = 2;
        ArrayInitialize(bullishDivergent,false);
        ArrayInitialize(bearishDivergent,false);
    } else {
        begin = prev_calculated - 1;
    }
    for (int i = begin; i < rates_total; i++) {
        bullishDivergent[i] = false;
        bearishDivergent[i] = false;

        if ((high[i] > high[i - 1]) && (high[i] > high[i - 2]) && (close[i] < ((high[i] + low[i]) / 2.0))) {
            bearishDivergent[i] = true;
        } else if ((low[i] < low[i - 1]) && (low[i] < low[i - 2]) && (close[i] > ((high[i] + low[i]) / 2.0))) {
            bullishDivergent[i] = true;
        }
    }
    //---- OnCalculate done. Return new prev_calculated.
    return(rates_total);
}
