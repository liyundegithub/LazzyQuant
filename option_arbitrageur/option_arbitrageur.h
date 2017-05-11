#ifndef OPTION_ARBITRAGEUR_H
#define OPTION_ARBITRAGEUR_H

#include "utility.h"

#include <QObject>
#include <QSet>

class OptionPricing;
class DepthMarketCollection;
class BaseStrategy;

class OptionArbitrageur : public QObject
{
    Q_OBJECT
protected:
    void updateOptions();
    void timesUp(int index);
    void loadOptionArbitrageurSettings();

    void setupRiskFree(const QStringList &options);
    void setupHighFreq(const QStringList &options);
    void preparePricing(const QMultiMap<QString, int> &underlyingKMap);

    QSet<QString> underlyingIDs;
    QStringList underlyingsForRiskFree;
    QStringList underlyingsForHighFreq;

    int updateRetryCounter;
    double availableMoney;
    double riskFreeInterestRate;

    OptionPricing *pPricingEngine;
    DepthMarketCollection *pDepthMarkets;
    BaseStrategy *pStrategy;

public:
    explicit OptionArbitrageur(bool replayMode = false, const QString &replayDate = QString(), QObject *parent = 0);
    ~OptionArbitrageur();

signals:

private slots:
    void onMarketData(const QString& instrumentID, uint time, double lastPrice, int volume,
                      double askPrice1, int askVolume1, double bidPrice1, int bidVolume1);
};

#endif // OPTION_ARBITRAGEUR_H
