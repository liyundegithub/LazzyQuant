#ifndef OPTION_PRICING_H
#define OPTION_PRICING_H

#include "utility.h"

#include <QDate>
#include <QMap>
#include <QPair>

class OptionPricing
{
public:
    explicit OptionPricing(double r, double q = 0, bool american = true);

    void generate(const QList<double> &kList, const QList<double> &s0List, const QList<double> &sigmaList, const QDate &endDate, const QDate &startDate = QDate::currentDate(), const int steps = 100);
    void generate(const QList<double> &kList, const QList<double> &s0List, const QList<double> &sigmaList, const double T, const int steps);

    double getPrice(const double k, const double s, const double sigma, const OPTION_TYPE type);
    double getSigma(const double k, const double s, const double price, const OPTION_TYPE type);

protected:
    bool american;
    double r, q, dt;
    double a, p, u, d;

    void generate(const QList<double> &kList, const double s0, const double sigma, const int steps);
    QPair<double, double> findS(const double s);
    QPair<double, double> findSigma(const double sigma);

    //     K            S          sigma    price
    QMap<double, QMap<double, QMap<double, double>>> callPriceMap;
    QMap<double, QMap<double, QMap<double, double>>> putPriceMap;

    QList<double> kList;
    int kNum;
    QList<double> sList;
    int sNum;
    QList<double> sigmaList;
    int sigmaNum;
};

#endif // OPTION_PRICING_H
