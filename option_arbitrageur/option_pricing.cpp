#include <cmath>
#include <cfloat>
#include <QDate>
#include <QDebug>

#include "trading_calendar.h"
#include "option_pricing.h"

extern TradingCalendar tradingCalendar;

OptionPricing::OptionPricing(const QMultiMap<QString, int> &underlyingKMap) :
    OptionIndex(underlyingKMap)
{
    pCallPrice = new S_SIGMA_PRICE[underlyingNum * kNum];
    pPutPrice  = new S_SIGMA_PRICE[underlyingNum * kNum];

    ppCallPrice = (S_SIGMA_PRICE **) malloc(underlyingNum * sizeof(S_SIGMA_PRICE*));
    ppPutPrice  = (S_SIGMA_PRICE **) malloc(underlyingNum * sizeof(S_SIGMA_PRICE*));

    for (int i = 0; i < underlyingNum; i++) {
        ppCallPrice[i] = pCallPrice + i * kNum;
        ppPutPrice[i]  = pPutPrice  + i * kNum;
    }

    pSigmaPrice = nullptr;
}

OptionPricing::~OptionPricing()
{
    delete[] pCallPrice;
    delete[] pPutPrice;

    free(ppCallPrice);
    free(ppPutPrice);

    if (pSigmaPrice != nullptr)
        free(pSigmaPrice);
}

void OptionPricing::setBasicParam(double r, double q, bool american)
{
    this->american = american;
    this->r = r;
    this->q = q;
}

void OptionPricing::setS0AndSigma(const QList<double> &s0List, const QList<double> &sigmaList)
{
    this->sList = s0List;
    std::sort(this->sList.begin(), this->sList.end());
    this->sNum = sList.length();

    this->sigmaList = sigmaList;
    std::sort(this->sigmaList.begin(), this->sigmaList.end());
    this->sigmaNum = sigmaList.length();

    pSigmaPrice = (double*) malloc(sigmaNum * sizeof(double));
    pLastSigmaPrice = pSigmaPrice + (sigmaNum - 1);
}

void OptionPricing::generate(const QString &underlyingID, const QDate &startDate, const QDate &endDate, int daysInOneYear, int steps)
{
    int underlyingIdx = getIdxByUnderlyingID(underlyingID);
    const int tradingDays = tradingCalendar.getTradingDays(startDate, endDate);
    generate(underlyingIdx, ((double)tradingDays) / ((double)daysInOneYear), steps);
}

void OptionPricing::generate(int underlyingIdx, double T, int steps)
{
    for (const auto sigma : qAsConst(sigmaList)) {
        dt = T / steps;
        a = exp((r - q) * dt);
        u = exp(sigma * sqrt(dt));
        d = exp(-sigma * sqrt(dt));
        p = (a - d) / (u - d);

        for (const auto s0 : qAsConst(sList)) {
            generate(underlyingIdx, s0, sigma, steps);
        }
    }
}

void OptionPricing::generate(int underlyingIdx, double s0, double sigma, int steps)
{
    const int length = (steps + 1);
    const int block_size = length * length;

    auto * data_block = (double*)malloc(block_size * 3 * sizeof(double));
    memset(data_block, 0, block_size * 3 * sizeof(double));
    auto ** ptr_block = (double**)malloc(length * 3 * sizeof(double*));
    memset(ptr_block, 0, length * 3 * sizeof(double*));
    double **S = ptr_block;
    double **C = ptr_block + length;
    double **P = ptr_block + length + length;
    for (int i = 0; i < length; i++) {
        S[i] = data_block + i * length;
        C[i] = data_block + block_size + i * length;
        P[i] = data_block + block_size + block_size + i * length;
    }

    for (int i = 0; i <= steps; i++) {
        for (int j = 0; j <= i; j++) {
            S[i][j] = s0 * pow(u, j) * pow(d, (i - j));
        }
    }

    for (int kIdx = 0; kIdx < kNum; kIdx ++) {
        int K = pK[kIdx];
        for (int j = 0; j <= steps; j++) {
            C[steps][j] = qMax(S[steps][j] - K, 0.0);
            P[steps][j] = qMax(K - S[steps][j], 0.0);
        }

        for (int i = steps - 1; i >= 0; i--) {
            for (int j = 0; j <= i; j ++) {
                C[i][j] = exp(-r * dt) * (p * C[i + 1][j + 1] + (1 - p) * C[i + 1][j]);
                P[i][j] = exp(-r * dt) * (p * P[i + 1][j + 1] + (1 - p) * P[i + 1][j]);
                if (american) {
                    C[i][j] = qMax(S[i][j] - K, C[i][j]);
                    P[i][j] = qMax(K - S[i][j], P[i][j]);
                }
            }
        }

        ppCallPrice[underlyingIdx][kIdx][s0][sigma] = C[0][0];
        ppPutPrice[underlyingIdx][kIdx][s0][sigma] = P[0][0];
    }

    free(ptr_block);
    free(data_block);
}

QPair<double, double> OptionPricing::findS(const double s) const
{
    int i = 0;
    for (; i < sNum - 1; i++) {
        if (sList[i] <= s && s <= sList[i + 1]) {
            return qMakePair(sList[i], sList[i + 1]);
        }
    }
    qDebug() << "Can not find S range contains" << s;
    return qMakePair(sList[sNum - 2], sList[sNum - 1]);
}

QPair<double, double> OptionPricing::findSigma(const double sigma) const
{
    int i = 0;
    for (; i < sigmaNum - 1; i++) {
        if (sigmaList[i] <= sigma && sigma <= sigmaList[i + 1]) {
            return qMakePair(sigmaList[i], sigmaList[i + 1]);
        }
    }
    qDebug() << "Can not find Sigma range contains" << sigma;
    return qMakePair(sigmaList[sigmaNum - 2], sigmaList[sigmaNum - 1]);
}

double OptionPricing::getPrice(const QString &underlyingID, const OPTION_TYPE type, int K, double s, double sigma) const
{
    if (!kList.contains(K)) {
        qDebug() << "No such exercise price! K =" << K;
        return -DBL_MAX;
    }

    int underlyingIdx = getIdxByUnderlyingID(underlyingID);
    int kIdx = getIdxByK(K);
    return getPriceByIdx(underlyingIdx, type, kIdx, s, sigma);
}

double OptionPricing::getPriceByIdx(int underlyingIdx, const OPTION_TYPE type, int kIdx, double s, double sigma) const
{
    const auto s1s2 = findS(s);
    const auto s1 = s1s2.first;
    const auto s2 = s1s2.second;
    const auto sigma1sigma2 = findSigma(sigma);
    const auto sigma1 = sigma1sigma2.first;
    const auto sigma2 = sigma1sigma2.second;

    S_SIGMA_PRICE *priceMap;
    if (type == CALL_OPT) {
        priceMap = &(ppCallPrice[underlyingIdx][kIdx]);
    } else {
        priceMap = &(ppPutPrice[underlyingIdx][kIdx]);
    }

    const auto p11 = (*priceMap)[s1][sigma1];
    const auto p12 = (*priceMap)[s1][sigma2];
    const auto p21 = (*priceMap)[s2][sigma1];
    const auto p22 = (*priceMap)[s2][sigma2];

    auto sum = p11 * (s2 - s) * (sigma2 - sigma)
             + p12 * (s2 - s) * (sigma - sigma1)
             + p21 * (s - s1) * (sigma2 - sigma)
             + p22 * (s - s1) * (sigma - sigma1);

    //           p11(s2-s)(sigma2-sigma)+p12(s2-s)(sigma-sigma1)+p21(s-s1)(sigma2-sigma)+p22(s-s1)(sigma-sigma1)
    // price = --------------------------------------------------------------------------------------------------
    //                                               (s2-s1)(sigma2-sigma1)
    return sum / (s2 - s1) / (sigma2 - sigma1);
}

double OptionPricing::getSigma(const QString &underlyingID, const OPTION_TYPE type, int K, double s, double price) const
{
    if (!kList.contains(K)) {
        qDebug() << "No such exercise price! K =" << K;
        return -DBL_MAX;
    }

    int underlyingIdx = getIdxByUnderlyingID(underlyingID);
    int kIdx = getIdxByK(K);
    return getSigmaByIdx(underlyingIdx, type, kIdx, s, price);
}

double OptionPricing::getSigmaByIdx(int underlyingIdx, const OPTION_TYPE type, int kIdx, double s, double price) const
{
    const auto s1s2 = findS(s);
    const auto s1 = s1s2.first;
    const auto s2 = s1s2.second;

    //   sigma    price
    QMap<double, double> *s1PriceMap;
    QMap<double, double> *s2PriceMap;

    if (type == CALL_OPT) {
        s1PriceMap = &(ppCallPrice[underlyingIdx][kIdx][s1]);
        s2PriceMap = &(ppCallPrice[underlyingIdx][kIdx][s2]);
    } else {
        s1PriceMap = &(ppPutPrice[underlyingIdx][kIdx][s1]);
        s2PriceMap = &(ppPutPrice[underlyingIdx][kIdx][s2]);
    }

    for (int i = 0; i < sigmaNum; i++) {
        pSigmaPrice[i] = ((*s1PriceMap)[sigmaList[i]] * (s2 - s) + (*s2PriceMap)[sigmaList[i]] * (s - s1)) / (s2 - s1);
    }

    if (price < *pSigmaPrice) {
        return DBL_MIN; // Too cheap
    } else if (*pLastSigmaPrice < price) {
        return DBL_MAX; // Too expensive
    }

    int i = 0;
    for (; i < sigmaNum - 1; i++) {
        if (pSigmaPrice[i] <= price && price <= pSigmaPrice[i + 1]) {
            break;
        }
    }

    const auto sigma1 = sigmaList[i];
    const auto sigma2 = sigmaList[i + 1];
    const auto sigma1price = pSigmaPrice[i];
    const auto sigma2price = pSigmaPrice[i + 1];

    return (price * (sigma2 - sigma1) - sigma1price * sigma2 + sigma2price * sigma1) / (sigma2price - sigma1price);
}
