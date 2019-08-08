#ifndef OPTION_ARBITRAGEUR_H
#define OPTION_ARBITRAGEUR_H

#include <QObject>
#include <QStringList>
#include <QSet>

class OptionHelper;
class OptionPricing;
class DepthMarketCollection;
class BaseStrategy;

class OptionArbitrageur : public QObject
{
    Q_OBJECT

protected:
    void loadOptionArbitrageurSettings();

    void setupRiskFree(const QStringList &options);
    void setupHighFreq(const QStringList &options);
    void preparePricing(const QMultiMap<QString, int> &underlyingKMap);

    QStringList allInstruments;
    OptionHelper *pHelper;

    QSet<QString> underlyingIDs;
    QStringList underlyingsForRiskFree;
    QStringList underlyingsForHighFreq;

    QString tradingDay;
    double availableMoney;
    double riskFreeInterestRate;

    OptionPricing *pPricingEngine = nullptr;
    DepthMarketCollection *pDepthMarkets = nullptr;
    BaseStrategy *pStrategy = nullptr;

public:
    explicit OptionArbitrageur(const QStringList &allInstruments, OptionHelper *pHelper, QObject *parent = nullptr);
    ~OptionArbitrageur();

    QSet<QString> getUnderlyingIDs() const { return underlyingIDs; }

public slots:
    void setTradingDay(const QString &tradingDay);
    void onMarketData(const QString &instrumentID, qint64 time, double lastPrice, int volume,
                      double askPrice1, int askVolume1, double bidPrice1, int bidVolume1);
    void onMarketClose();
};

#endif // OPTION_ARBITRAGEUR_H
