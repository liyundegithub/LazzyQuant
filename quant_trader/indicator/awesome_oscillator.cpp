#include "awesome_oscillator.h"

#define DATA_LIMIT 33

AwesomeOscillator::AwesomeOscillator(QObject *parent) :
    MQL5Indicator(4, parent)
{
}

void AwesomeOscillator::OnInit()
{
//---- indicator buffers mapping
   SetIndexBuffer(0,ExtAOBuffer,INDICATOR_DATA);
   SetIndexBuffer(1,ExtColorBuffer,INDICATOR_COLOR_INDEX);
   SetIndexBuffer(2,ExtFastBuffer,INDICATOR_CALCULATIONS);
   SetIndexBuffer(3,ExtSlowBuffer,INDICATOR_CALCULATIONS);
//--- set accuracy
   IndicatorSetInteger(INDICATOR_DIGITS,_Digits+1);
//--- sets first bar from what index will be drawn
   PlotIndexSetInteger(0,PLOT_DRAW_BEGIN,33);
//--- name for DataWindow
   IndicatorSetString(INDICATOR_SHORTNAME,"AO");
//--- get handles
   ExtFastSMAHandle=iMA(NULL,0,5,0,MODE_SMA,PRICE_MEDIAN);
   ExtSlowSMAHandle=iMA(NULL,0,34,0,MODE_SMA,PRICE_MEDIAN);
//---- initialization done
}

int AwesomeOscillator::OnCalculate (const int rates_total,                     // size of input time series
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
//--- check for rates total
   if(rates_total<=DATA_LIMIT)
      return(0);// not enough bars for calculation
//--- not all data may be calculated
   int calculated=BarsCalculated(ExtFastSMAHandle);
   if(calculated<rates_total)
     {
      Print("Not all data of ExtFastSMAHandle is calculated (",calculated,"bars ). Error",GetLastError());
      return(0);
     }
   calculated=BarsCalculated(ExtSlowSMAHandle);
   if(calculated<rates_total)
     {
      Print("Not all data of ExtSlowSMAHandle is calculated (",calculated,"bars ). Error",GetLastError());
      return(0);
     }
//--- we can copy not all data
   int to_copy;
   if(prev_calculated>rates_total || prev_calculated<0) to_copy=rates_total;
   else
     {
      to_copy=rates_total-prev_calculated;
      if(prev_calculated>0) to_copy++;
     }
//--- get FastSMA buffer
   if(IsStopped()) return(0); //Checking for stop flag
   if(CopyBuffer(ExtFastSMAHandle,0,0,to_copy,ExtFastBuffer)<=0)
     {
      Print("Getting fast SMA is failed! Error",GetLastError());
      return(0);
     }
//--- get SlowSMA buffer
   if(IsStopped()) return(0); //Checking for stop flag
   if(CopyBuffer(ExtSlowSMAHandle,0,0,to_copy,ExtSlowBuffer)<=0)
     {
      Print("Getting slow SMA is failed! Error",GetLastError());
      return(0);
     }
//--- first calculation or number of bars was changed
   int i,limit;
   if(prev_calculated<=DATA_LIMIT)
     {
      for(i=0;i<DATA_LIMIT;i++)
         ExtAOBuffer[i]=0.0;
      limit=DATA_LIMIT;
     }
   else limit=prev_calculated-1;
//--- main loop of calculations
   for(i=limit;i<rates_total && !IsStopped();i++)
     {
      ExtAOBuffer[i]=ExtFastBuffer[i]-ExtSlowBuffer[i];
      if(ExtAOBuffer[i]>ExtAOBuffer[i-1])ExtColorBuffer[i]=0.0; // set color Green
      else                               ExtColorBuffer[i]=1.0; // set color Red
     }
//--- return value of prev_calculated for next call
   return(rates_total);
}
