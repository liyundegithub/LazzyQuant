#include "../quant_trader.h"
#include "mql5_indicator.h"
#include "indicator_functions.h"

IndicatorFunctions::IndicatorFunctions(QObject *parent) : QObject(parent)
{
    pTrader = dynamic_cast<QuantTrader*>(parent);
    Q_ASSERT(pTrader != nullptr);
}

void IndicatorFunctions::addDepend(AbstractIndicator* indicator)
{
    for (auto item : qAsConst(dependIndicators)) {
        if (item == indicator) {
            return;
        }
    }
    dependIndicators.append(indicator);
}

MQL5Indicator*  IndicatorFunctions::iAO(
   string           symbol,     // symbol name
   ENUM_TIMEFRAMES  period      // period
   )
{
    auto pIndicator = static_cast<MQL5Indicator*>(
                pTrader->registerIndicator(
                    symbol, period, "AwesomeOscillator"
                ));
    addDepend(pIndicator);
    return pIndicator;
}

MQL5Indicator*  IndicatorFunctions::iBands(
   string              symbol,            // symbol name
   ENUM_TIMEFRAMES     period,            // period
   int                 bands_period,      // period for average line calculation
   int                 bands_shift,       // horizontal shift of the indicator
   double              deviation,         // number of standard deviations
   ENUM_APPLIED_PRICE  applied_price      // type of price or handle
   )
{
    auto pIndicator = static_cast<MQL5Indicator*>(
                pTrader->registerIndicator(
                    symbol, period, "BollingerBand", bands_period, bands_shift, deviation, applied_price
                ));
    addDepend(pIndicator);
    return pIndicator;
}

MQL5Indicator*  IndicatorFunctions::iMA(
   string               symbol,            // symbol name
   ENUM_TIMEFRAMES      period,            // period
   int                  ma_period,         // averaging period
   int                  ma_shift,          // horizontal shift
   ENUM_MA_METHOD       ma_method,         // smoothing type
   ENUM_APPLIED_PRICE   applied_price      // type of price or handle
   )
{
    auto pIndicator = static_cast<MQL5Indicator*>(
                pTrader->registerIndicator(
                    symbol, period, "MA", ma_period, ma_shift, ma_method, applied_price
                ));
    addDepend(pIndicator);
    return pIndicator;
}

MQL5Indicator*  IndicatorFunctions::iSAR(
   string           symbol,      // symbol name
   ENUM_TIMEFRAMES  period,      // period
   double           step,        // price increment step - acceleration factor
   double           maximum      // maximum value of step
   )
{
    auto pIndicator = static_cast<MQL5Indicator*>(
                pTrader->registerIndicator(
                    symbol, period, "ParabolicSAR", step, maximum
                ));
    addDepend(pIndicator);
    return pIndicator;
}
