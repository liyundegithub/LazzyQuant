#ifndef HIGH_FREQUENCY_H
#define HIGH_FREQUENCY_H

#include <Q3DSurface>
#include <QSurfaceDataProxy>
using namespace QtDataVisualization;

#include "base_strategy.h"

class OptionPricing;

class HighFrequency : public BaseStrategy
{
    const OptionPricing *pPricingEngine;

    int underlyingNum;
    int kNum;

    // TODO 节间全部clear()
    double * pUnderlyingPrices;

    double * pCallSigma;
    double * pPutSigma;

    double ** ppCallAskSigma;
    double ** ppPutAskSigma;
    double ** ppCallBidSigma;
    double ** ppPutBidSigma;

    Q3DSurface *surface;
    QSurfaceDataArray *pCallAskData;
    QSurfaceDataArray *pCallBidData;
    QSurface3DSeries *pCallAskSeries;
    QSurface3DSeries *pCallBidSeries;

public:
    HighFrequency(OptionPricing *pPricing, DepthMarketCollection *pDMC);
    ~HighFrequency();

    void onUnderlyingChanged(int underlyingIdx);
    void onOptionChanged(int underlyingIdx, OPTION_TYPE type, int kIdx);

    void clear(int underlyingIdx);
};

#endif // HIGH_FREQUENCY_H
