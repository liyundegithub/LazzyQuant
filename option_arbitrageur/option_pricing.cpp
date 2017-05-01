#include <cmath>
#include <cfloat>
#include <QDebug>

#include "option_pricing.h"

OptionPricing::OptionPricing(double r, double q, bool american) :
    american(american), r(r), q(q)
{
    //
}

void OptionPricing::generate(const QList<double> &kList, const QList<double> &s0List, const QList<double> &sigmaList, const double T, const int steps)
{
    this->kList = kList;
    this->kNum = kList.length();
    this->sList = s0List;
    std::sort(this->sList.begin(), this->sList.end());
    this->sNum = sList.length();
    this->sigmaList = sigmaList;
    std::sort(this->sigmaList.begin(), this->sigmaList.end());
    this->sigmaNum = sigmaList.length();

    for (const auto sigma : sigmaList) {
        dt = T / steps;
        a = exp((r - q) * dt);
        u = exp(sigma * sqrt(dt));
        d = exp(-sigma * sqrt(dt));
        p = (a - d) / (u - d);

        for (const auto s0 : s0List) {
            generate(kList, s0, sigma, steps);
        }
    }
}

void OptionPricing::generate(const QList<double> &kList, const double s0, const double sigma, const int steps)
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

    for (const auto K : kList) {
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

        callPriceMap[K][s0][sigma] = C[0][0];
        putPriceMap[K][s0][sigma] = P[0][0];
    }

    free(ptr_block);
    free(data_block);
}

QPair<double, double> OptionPricing::findS(const double s)
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

QPair<double, double> OptionPricing::findSigma(const double sigma)
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

double OptionPricing::getPrice(const double k, const double s, const double sigma, const OPTION_TYPE type)
{
    if (!kList.contains(k)) {
        qDebug() << "No such exercise price! K =" << k;
        return -DBL_MAX;
    }

    const auto s1s2 = findS(s);
    const auto s1 = s1s2.first;
    const auto s2 = s1s2.second;
    const auto sigma1sigma2 = findSigma(sigma);
    const auto sigma1 = sigma1sigma2.first;
    const auto sigma2 = sigma1sigma2.second;

    //     S          sigma    price
    QMap<double, QMap<double, double>> *priceMap;
    if (type == CALL_OPT) {
        priceMap = &(callPriceMap[k]);
    } else {
        priceMap = &(putPriceMap[k]);
    }

    const auto s1sigma1 = (*priceMap)[s1][sigma1];
    const auto s1sigma2 = (*priceMap)[s1][sigma2];
    const auto s2sigma1 = (*priceMap)[s2][sigma1];
    const auto s2sigma2 = (*priceMap)[s2][sigma2];

    auto sum = s1sigma1 * (s2 - s) * (sigma2 - sigma)
             + s1sigma2 * (s2 - s) * (sigma - sigma1)
             + s2sigma1 * (s - s1) * (sigma2 - sigma)
             + s2sigma2 * (s - s1) * (sigma - sigma1);

    //           p11(s2-s)(sigma2-sigma)+p12(s2-s)(sigma-sigma1)+p21(s-s1)(sigma2-sigma)+p22(s-s1)(sigma-sigma1)
    // price = --------------------------------------------------------------------------------------------------
    //                                               (s2-s1)(sigma2-sigma1)
    return sum / (s2 - s1) / (sigma2 - sigma1);
}

double OptionPricing::getSigma(const double k, const double s, const double price, const OPTION_TYPE type)
{
    if (!kList.contains(k)) {
        qDebug() << "No such exercise price! K =" << k;
        return -DBL_MAX;
    }

    const auto s1s2 = findS(s);
    const auto s1 = s1s2.first;
    const auto s2 = s1s2.second;

    //   sigma    price
    QMap<double, double> *s1PriceMap;
    QMap<double, double> *s2PriceMap;

    if (type == CALL_OPT) {
        s1PriceMap = &(callPriceMap[k][s1]);
        s2PriceMap = &(callPriceMap[k][s2]);
    } else {
        s1PriceMap = &(putPriceMap[k][s1]);
        s2PriceMap = &(putPriceMap[k][s2]);
    }

    QMap<double, double> iPriceMap;
    for (const auto sigma : sigmaList) {
        iPriceMap[sigma] = ((*s1PriceMap)[sigma] * (s2 - s) + (*s2PriceMap)[sigma] * (s - s1)) / (s2 - s1);
    }

    int i = 0;
    for (; i < sigmaNum - 1; i++) {
        if (iPriceMap[sigmaList[i]] <= price && price <= iPriceMap[sigmaList[i + 1]]) {
            break;
        }
    }

    const auto sigma1 = sigmaList[i];
    const auto sigma2 = sigmaList[i + 1];
    const auto sigma1price = iPriceMap[sigma1];
    const auto sigma2price = iPriceMap[sigma2];

    return (price * (sigma2 - sigma1) - sigma1price * sigma2 + sigma2price * sigma1) / (sigma2price - sigma1price);
}
