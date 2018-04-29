#include "fractal.h"

#include <cfloat>

Fractal::Fractal(QObject *parent) :
    MQL5Indicator(2, parent),
    ExtUpperBuffer(),
    ExtLowerBuffer(),
    confirmedUpIndex(),
    confirmedDnIndex()
{
    //
}

void Fractal::OnInit()
{
//---- indicator buffers mapping
   SetIndexBuffer(0,ExtUpperBuffer,INDICATOR_DATA);
   SetIndexBuffer(1,ExtLowerBuffer,INDICATOR_DATA);
   IndicatorSetInteger(INDICATOR_DIGITS,_Digits);
//---- sets first bar from what index will be drawn
   PlotIndexSetInteger(0,PLOT_ARROW,217);
   PlotIndexSetInteger(1,PLOT_ARROW,218);
//---- arrow shifts when drawing
   PlotIndexSetInteger(0,PLOT_ARROW_SHIFT,ExtArrowShift);
   PlotIndexSetInteger(1,PLOT_ARROW_SHIFT,-ExtArrowShift);
//---- sets drawing line empty value--
   PlotIndexSetDouble(0,PLOT_EMPTY_VALUE,EMPTY_VALUE);
   PlotIndexSetDouble(1,PLOT_EMPTY_VALUE,EMPTY_VALUE);
//---- initialization done
}

void Fractal::preCalculate()
{
    MQL5Indicator::preCalculate();
    this->confirmedUpIndex.resize(rates_total);
    this->confirmedDnIndex.resize(rates_total);
}

int Fractal::OnCalculate (const int rates_total,                     // size of input time series
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
    Q_UNUSED(close)
    Q_UNUSED(tick_volume)
    Q_UNUSED(volume)
    Q_UNUSED(spread)

    //--- check for minimum rates count
    if(rates_total<3) {
        upState = ONE;
        dnState = ONE;
        return(0);
    }
    //--- detect current position
    int pos=prev_calculated;
    //--- correct position
    if(pos<1) {
        pos=1;
        //--- clean up arrays
        ArrayInitialize(ExtUpperBuffer,EMPTY_VALUE);
        ArrayInitialize(ExtLowerBuffer,EMPTY_VALUE);
        ArrayInitialize(confirmedUpIndex,0);
        ArrayInitialize(confirmedDnIndex,0);
    }

    for (int i = pos; i < rates_total - 1; i++) {
        ExtUpperBuffer[i] = EMPTY_VALUE;
        ExtLowerBuffer[i] = EMPTY_VALUE;
        confirmedUpIndex[i] = 0;
        confirmedDnIndex[i] = 0;

        switch (upState) {
        case ONE:
            max2 = qMax(high[i], high[i-1]);
            max3 = -DBL_MAX;
            max3Idx = -1;
            upState = TWO;
            break;
        case TWO:
            if (high[i] > max2) {
                max3 = high[i];
                max3Idx = i;
                upState = THREE;
            } else {
                max2 = qMax(high[i - 1], high[i]);
            }
            break;
        case THREE:
            if (high[i] < max3) {
                upState = FOUR;
            } else {
                max3 = high[i];
                max3Idx = i;
            }
            break;
        case FOUR:
            if (high[i] < max3) {
                confirmedUpIndex[i] = i - max3Idx;
                ExtUpperBuffer[max3Idx] = high[max3Idx];
                max2 = qMax(high[i - 1], high[i]);
                upState = TWO;
            } else if (high[i] > max3) {
                max3 = high[i];
                max3Idx = i;
                upState = THREE;
            } else {
                upState = THREE;
            }
            break;
        default:
            break;
        }

        switch (dnState) {
        case ONE:
            min2 = qMin(low[i], low[i-1]);
            min3 = -DBL_MAX;
            min3Idx = -1;
            dnState = TWO;
            break;
        case TWO:
            if (low[i] < min2) {
                min3 = low[i];
                min3Idx = i;
                dnState = THREE;
            } else {
                min2 = qMin(low[i - 1], low[i]);
            }
            break;
        case THREE:
            if (low[i] > min3) {
                dnState = FOUR;
            } else {
                min3 = low[i];
                min3Idx = i;
            }
            break;
        case FOUR:
            if (low[i] > min3) {
                confirmedDnIndex[i] = i - min3Idx;
                ExtLowerBuffer[min3Idx] = low[min3Idx];
                min2 = qMin(low[i - 1], low[i]);
                dnState = TWO;
            } else if (low[i] < min3) {
                min3 = low[i];
                min3Idx = i;
                dnState = THREE;
            } else {
                upState = THREE;
            }
            break;
        default:
            break;
        }
    }
    return rates_total - 1;
}
