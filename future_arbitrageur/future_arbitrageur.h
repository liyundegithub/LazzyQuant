#ifndef FUTURE_ARBITRAGEUR_H
#define FUTURE_ARBITRAGEUR_H

#include <QObject>

class DepthMarketCollection;
class BaseStrategy;

class FutureArbitrageur : public QObject
{
    Q_OBJECT
public:
    explicit FutureArbitrageur(QObject *parent = 0);
    ~FutureArbitrageur();

    void setupStrategies();

    DepthMarketCollection *pMarketCollection;
    QList<BaseStrategy *> pStrategyList;

signals:

public slots:
    void onMarketData(const QString& instrumentID, uint time, double lastPrice, int volume,
                      double askPrice1, int askVolume1, double bidPrice1, int bidVolume1);
};

#endif // FUTURE_ARBITRAGEUR_H
