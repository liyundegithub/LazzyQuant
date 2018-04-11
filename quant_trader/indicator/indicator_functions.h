#ifndef INDICATOR_FUNCTIONS_H
#define INDICATOR_FUNCTIONS_H

#include <QObject>

#include "../mql5_compatible.h"

class QuantTrader;
class AbstractIndicator;
class MQL5Indicator;

class IndicatorFunctions : public QObject
{
    Q_OBJECT

public:
    explicit IndicatorFunctions(QObject *parent = nullptr);

    enum ENUM_MA_METHOD {
        MODE_SMA,   // Simple averaging
        MODE_EMA,   // Exponential averaging
        MODE_SMMA,  // Smoothed averaging
        MODE_LWMA,  // Linear-weighted averaging
    };
    Q_ENUM(ENUM_MA_METHOD)

    enum ENUM_APPLIED_PRICE {
        PRICE_CLOSE,    // Close price
        PRICE_OPEN,     // Open price
        PRICE_HIGH,     // The maximum price for the period
        PRICE_LOW,      // The minimum price for the period
        PRICE_MEDIAN,   // Median price, (high + low)/2
        PRICE_TYPICAL,  // Typical price, (high + low + close)/3
        PRICE_WEIGHTED, // Average price, (high + low + close + close)/4
    };
    Q_ENUM(ENUM_APPLIED_PRICE)

protected:
    QuantTrader* pTrader;
    QList<AbstractIndicator*> dependIndicators;
    void addDepend(AbstractIndicator* indicator);

    MQL5Indicator*  iAO(
       string           symbol,     // symbol name
       ENUM_TIMEFRAMES  period      // period
       );

    MQL5Indicator*  iBands(
       string              symbol,            // symbol name
       ENUM_TIMEFRAMES     period,            // period
       int                 bands_period,      // period for average line calculation
       int                 bands_shift,       // horizontal shift of the indicator
       double              deviation,         // number of standard deviations
       ENUM_APPLIED_PRICE  applied_price      // type of price or handle
       );

    MQL5Indicator*  iMA(
       string               symbol,            // symbol name
       ENUM_TIMEFRAMES      period,            // period
       int                  ma_period,         // averaging period
       int                  ma_shift,          // horizontal shift
       ENUM_MA_METHOD       ma_method,         // smoothing type
       ENUM_APPLIED_PRICE   applied_price      // type of price or handle
       );

    MQL5Indicator*  iSAR(
       string           symbol,      // symbol name
       ENUM_TIMEFRAMES  period,      // period
       double           step,        // price increment step - acceleration factor
       double           maximum      // maximum value of step
       );

};

#endif // INDICATOR_FUNCTIONS_H
