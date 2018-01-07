#include "bollinger_band.h"

BollingerBand::BollingerBand(int InpBandsPeriod, int InpBandsShift, double InpBandsDeviations, ENUM_APPLIED_PRICE applyTo, QObject *parent) :
    MQL5IndicatorOnSingleDataBuffer(4, applyTo, parent),
    ExtMLBuffer(),
    ExtTLBuffer(),
    ExtBLBuffer(),
    ExtStdDevBuffer()
{
//--- check for input values
   if(InpBandsPeriod<2)
     {
      ExtBandsPeriod=20;
      printf("Incorrect value for input variable InpBandsPeriod=%d. Indicator will use value=%d for calculations.",InpBandsPeriod,ExtBandsPeriod);
     }
   else ExtBandsPeriod=InpBandsPeriod;
   if(InpBandsShift<0)
     {
      ExtBandsShift=0;
      printf("Incorrect value for input variable InpBandsShift=%d. Indicator will use value=%d for calculations.",InpBandsShift,ExtBandsShift);
     }
   else
      ExtBandsShift=InpBandsShift;
   if(InpBandsDeviations==0.0)
     {
      ExtBandsDeviations=2.0;
      printf("Incorrect value for input variable InpBandsDeviations=%f. Indicator will use value=%f for calculations.",InpBandsDeviations,ExtBandsDeviations);
     }
   else ExtBandsDeviations=InpBandsDeviations;
}

void BollingerBand::OnInit()
{
//--- define buffers
   SetIndexBuffer(0,ExtMLBuffer);
   SetIndexBuffer(1,ExtTLBuffer);
   SetIndexBuffer(2,ExtBLBuffer);
   SetIndexBuffer(3,ExtStdDevBuffer,INDICATOR_CALCULATIONS);
//--- set index labels
   PlotIndexSetString(0,PLOT_LABEL,"Bands("+string(ExtBandsPeriod)+") Middle");
   PlotIndexSetString(1,PLOT_LABEL,"Bands("+string(ExtBandsPeriod)+") Upper");
   PlotIndexSetString(2,PLOT_LABEL,"Bands("+string(ExtBandsPeriod)+") Lower");
//--- indicator name
   IndicatorSetString(INDICATOR_SHORTNAME,"Bollinger Bands");
//--- indexes draw begin settings
   ExtPlotBegin=ExtBandsPeriod-1;
   PlotIndexSetInteger(0,PLOT_DRAW_BEGIN,ExtBandsPeriod);
   PlotIndexSetInteger(1,PLOT_DRAW_BEGIN,ExtBandsPeriod);
   PlotIndexSetInteger(2,PLOT_DRAW_BEGIN,ExtBandsPeriod);
//--- indexes shift settings
   PlotIndexSetInteger(0,PLOT_SHIFT,ExtBandsShift);
   PlotIndexSetInteger(1,PLOT_SHIFT,ExtBandsShift);
   PlotIndexSetInteger(2,PLOT_SHIFT,ExtBandsShift);
//--- number of digits of indicator value
   IndicatorSetInteger(INDICATOR_DIGITS,_Digits+1);
//---- OnInit done
}

int BollingerBand::OnCalculate (const int rates_total,                  // size of the price[] array
                                const int prev_calculated,              // bars handled on a previous call
                                const int begin,                        // where the significant data start from
                                const _TimeSeries<double>& price        // array to calculate
                                )
{
//--- variables
   int pos;
//--- indexes draw begin settings, when we've recieved previous begin
   if(ExtPlotBegin!=ExtBandsPeriod+begin)
     {
      ExtPlotBegin=ExtBandsPeriod+begin;
      PlotIndexSetInteger(0,PLOT_DRAW_BEGIN,ExtPlotBegin);
      PlotIndexSetInteger(1,PLOT_DRAW_BEGIN,ExtPlotBegin);
      PlotIndexSetInteger(2,PLOT_DRAW_BEGIN,ExtPlotBegin);
     }
//--- check for bars count
   if(rates_total<ExtPlotBegin)
      return(0);
//--- starting calculation
   if(prev_calculated>1) pos=prev_calculated-1;
   else pos=0;
//--- main cycle
   for(int i=pos;i<rates_total && !IsStopped();i++)
     {
      //--- middle line
      ExtMLBuffer[i]=SimpleMA(i,ExtBandsPeriod,price);
      //--- calculate and write down StdDev
      ExtStdDevBuffer[i]=StdDev_Func(i,price,ExtMLBuffer,ExtBandsPeriod);
      //--- upper line
      ExtTLBuffer[i]=ExtMLBuffer[i]+ExtBandsDeviations*ExtStdDevBuffer[i];
      //--- lower line
      ExtBLBuffer[i]=ExtMLBuffer[i]-ExtBandsDeviations*ExtStdDevBuffer[i];
      //---
     }
//---- OnCalculate done. Return new prev_calculated.
   return(rates_total);
}

//+------------------------------------------------------------------+
//| Simple Moving Average                                            |
//+------------------------------------------------------------------+
template<class T>
double SimpleMA(const int position, const int period, const T &price)
  {
//---
   double result=0.0;
//--- check position
   if(position>=period-1 && period>0)
     {
      //--- calculate value
      for(int i=0;i<period;i++) result+=price[position-i];
      result/=period;
     }
//---
   return(result);
  }

//+------------------------------------------------------------------+
//| Calculate Standard Deviation                                     |
//+------------------------------------------------------------------+
template<class T, class U>
double StdDev_Func(int position,const T &price,const U &MAprice,int period)
  {
//--- variables
   double StdDev_dTmp=0.0;
//--- check for position
   if(position<period) return(StdDev_dTmp);
//--- calcualte StdDev
   for(int i=0;i<period;i++) StdDev_dTmp+=MathPow(price[position-i]-MAprice[position],2);
   StdDev_dTmp=MathSqrt(StdDev_dTmp/period);
//--- return calculated value
   return(StdDev_dTmp);
  }
//+------------------------------------------------------------------+
