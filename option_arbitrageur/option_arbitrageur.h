#ifndef OPTION_ARBITRAGEUR_H
#define OPTION_ARBITRAGEUR_H

#include <boost/optional.hpp>

#include "market_watcher_interface.h"
#include "trade_executer_interface.h"

#include "utility.h"

template <typename Key, typename T> class QMap;

struct DepthMarket {
    double askPrices[2];
    int askVolumes[2];
    double bidPrices[2];
    int bidVolumes[2];
};

class OptionArbitrageur : public QObject
{
    Q_OBJECT
protected:
    com::lazzyquant::market_watcher *pWatcher;
    com::lazzyquant::trade_executer *pExecuter;

    QMap<QString, boost::optional<int>> position_map;

    void loadOptionArbitrageurSettings();
    double threshold;

    //   期货合约    两档盘口
    QMap<QString, DepthMarket> future_market_data;
    //  标的期货合约       看涨/看跌       行权价   两档盘口
    QMap<QString, QMap<OPTION_DIR, QMap<int, DepthMarket>>> option_market_data;

    // Argitrage strategies
    void findInefficientPrices(const QString &futureID, OPTION_DIR dir = CALL_OPT, int exercisePrice = 0);
    void findCheapCallOptions(const QString &futureID);
    void checkCheapCallOptions(const QString &futureID, int exercisePrice);
    void findCheapPutOptions(const QString &futureID);
    void checkCheapPutOptions(const QString &futureID, int exercisePrice);
    void findReversedCallOptions(const QString &futureID, int exercisePriceToCheck);
    void checkReversedCallOptions(const QString &futureID, const QMap<int, DepthMarket> &callOptionMap, int lowExercisePrice, int highExercisePrice);
    void findReversedPutOptions(const QString &futureID, int exercisePriceToCheck);
    void checkReversedPutOptions(const QString &futureID, const QMap<int, DepthMarket> &putOptionMap, int lowExercisePrice, int highExercisePrice);
    void findCheapStrangles(const QString &futureID);

public:
    explicit OptionArbitrageur(QObject *parent = 0);
    ~OptionArbitrageur();

signals:

private slots:
    void onMarketData(const QString& instrumentID, uint time, double lastPrice, int volume,
                      double askPrice1, int askVolume1, double bidPrice1, int bidVolume1,
                      double askPrice2, int askVolume2, double bidPrice2, int bidVolume2);
};

#endif // OPTION_ARBITRAGEUR_H
