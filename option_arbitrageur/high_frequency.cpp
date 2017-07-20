#include <QtDataVisualization>

#include "depth_market.h"
#include "option_pricing.h"
#include "high_frequency.h"
#include "option_helper.h"

HighFrequency::HighFrequency(OptionPricing *pPricing, DepthMarketCollection *pDMC) :
    BaseStrategy(pDMC),
    pPricingEngine(pPricing)
{
    underlyingNum = pDMC->getUnderlyingNum();
    kNum = pDMC->getKNum();

    pUnderlyingPrices = (double *) malloc(underlyingNum * sizeof(double));

    pCallSigma = (double *) malloc (underlyingNum * kNum * sizeof(double) * 2);
    pPutSigma  = (double *) malloc (underlyingNum * kNum * sizeof(double) * 2);

    ppCallAskSigma = (double **) malloc (underlyingNum * sizeof(double*));
    ppPutAskSigma  = (double **) malloc (underlyingNum * sizeof(double*));
    ppCallBidSigma = (double **) malloc (underlyingNum * sizeof(double*));
    ppPutBidSigma  = (double **) malloc (underlyingNum * sizeof(double*));

    for (int i = 0; i < underlyingNum; i++) {
        ppCallAskSigma[i] = pCallSigma + i * kNum;
        ppPutAskSigma[i]  = pPutSigma  + i * kNum;
        ppCallBidSigma[i] = pCallSigma + i * kNum + underlyingNum * kNum;
        ppPutBidSigma[i]  = pPutSigma  + i * kNum + underlyingNum * kNum;
    }

    surface = new Q3DSurface();
    surface->setFlags(surface->flags() ^ Qt::FramelessWindowHint);
    pCallAskData = new QSurfaceDataArray;
    pCallBidData = new QSurfaceDataArray;
    for (int i = 0; i < underlyingNum; i++) {
        QSurfaceDataRow *pcadataRow = new QSurfaceDataRow;
        QSurfaceDataRow *pcbdataRow = new QSurfaceDataRow;
        for (int j = 0; j < kNum; j++) {
            *pcadataRow << QVector3D(j, 10.0f, i);
            *pcbdataRow << QVector3D(j, 10.0f, i);
        }
        *pCallAskData << pcadataRow;
        *pCallBidData << pcbdataRow;
    }

    pCallAskSeries = new QSurface3DSeries;
    pCallAskSeries->dataProxy()->resetArray(pCallAskData);
    pCallBidSeries = new QSurface3DSeries;
    pCallBidSeries->dataProxy()->resetArray(pCallBidData);

    pCallAskSeries->setBaseColor(QColor(255, 0, 0));
    surface->addSeries(pCallAskSeries);
    surface->addSeries(pCallBidSeries);

    surface->axisY()->setRange(5.0f, 25.0f);
    surface->axisY()->setAutoAdjustRange(false);

    surface->show();
}

HighFrequency::~HighFrequency()
{
    free(pUnderlyingPrices);

    free(pCallSigma);
    free(pPutSigma);

    free(ppCallAskSigma);
    free(ppPutAskSigma);
    free(ppCallBidSigma);
    free(ppPutBidSigma);
}

void HighFrequency::onUnderlyingChanged(int underlyingIdx)
{
    const auto underlying = pDepthMarkets->getUnderlyingDepthMarketByIdx(underlyingIdx);

    if (underlying.isLowerLimit() || underlying.isUpperLimit()) {
        // Limitted
        qDebug() << "Limited!";
        clear(underlyingIdx);
        return;
    }
    pUnderlyingPrices[underlyingIdx] = (underlying.askPrice + underlying.bidPrice) / 2.0;

    for (int kIdx = 0; kIdx < kNum; kIdx++) {
        auto option = pDepthMarkets->getOptionDepthMarketByIdx(underlyingIdx, PUT_OPT, kIdx);
        const auto s = pUnderlyingPrices[underlyingIdx];

        auto askSigma = pPricingEngine->getSigmaByIdx(underlyingIdx, PUT_OPT, kIdx, s, option.askPrice);
        auto bidSigma = pPricingEngine->getSigmaByIdx(underlyingIdx, PUT_OPT, kIdx, s, option.bidPrice);

        (*(*pCallAskData)[underlyingIdx])[kIdx].setY(askSigma * 100.0);
        (*(*pCallBidData)[underlyingIdx])[kIdx].setY(bidSigma * 100.0);

        pCallAskSeries->dataProxy()->resetArray(pCallAskData);
        pCallBidSeries->dataProxy()->resetArray(pCallBidData);
    }
}

void HighFrequency::onOptionChanged(int underlyingIdx, OPTION_TYPE type, int kIdx)
{
    auto option = pDepthMarkets->getOptionDepthMarketByIdx(underlyingIdx, type, kIdx);
    const auto s = pUnderlyingPrices[underlyingIdx];
    const QString optionID = pDepthMarkets->makeOptionByIdx(underlyingIdx, type, kIdx);

    if (type == PUT_OPT) {
        auto askSigma = pPricingEngine->getSigmaByIdx(underlyingIdx, type, kIdx, s, option.askPrice);
        auto bidSigma = pPricingEngine->getSigmaByIdx(underlyingIdx, type, kIdx, s, option.bidPrice);

        (*(*pCallAskData)[underlyingIdx])[kIdx].setY(askSigma * 100.0);
        (*(*pCallBidData)[underlyingIdx])[kIdx].setY(bidSigma * 100.0);
        //emit series->dataProxy()->itemChanged(kIdx, underlyingIdx);
        pCallAskSeries->dataProxy()->resetArray(pCallAskData);
        pCallBidSeries->dataProxy()->resetArray(pCallBidData);
    }
}

void HighFrequency::clear(int underlyingIdx)
{
    pUnderlyingPrices[underlyingIdx] = 0.0;
    memset(ppCallAskSigma[underlyingIdx], 0, kNum * sizeof(double));
    memset(ppPutAskSigma[underlyingIdx],  0, kNum * sizeof(double));
    memset(ppCallBidSigma[underlyingIdx], 0, kNum * sizeof(double));
    memset(ppPutBidSigma[underlyingIdx],  0, kNum * sizeof(double));
}
