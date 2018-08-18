#ifndef LAZZYQUANTPROXY_H
#define LAZZYQUANTPROXY_H

#include <QObject>

class ComLazzyquantMarket_watcherInterface;
class ComLazzyquantSinyee_replayerInterface;
class ComLazzyquantTrade_executerInterface;
class ComLazzyquantQuant_traderInterface;

class LazzyQuantProxy : public QObject
{
    Q_OBJECT

    ComLazzyquantMarket_watcherInterface *pWatcher = nullptr;
    ComLazzyquantSinyee_replayerInterface *pReplayer = nullptr;
    ComLazzyquantTrade_executerInterface *pExecuter = nullptr;
    ComLazzyquantQuant_traderInterface *pTrader = nullptr;

public:
    explicit LazzyQuantProxy(QObject *parent = nullptr);
    ~LazzyQuantProxy();

signals:
    // 报告各模块状态: wather, replayer, executer, trader.
    void moduleStatus(bool, bool, bool, bool);

    // market_watcher or sinyee_replayer
    void tradingDayChanged(const QString &tradingDay);
    void endOfReplay(const QString &tradingDay);
    void newMarketData(const QString &instrumentID, int time, double lastPrice, int volume,
                       double askPrice1, int askVolume1, double bidPrice1, int bidVolume1);

    // quant_trader
    void newBarFormed(const QString &instrumentID, const QString &timeFrame);

    // trade_executer
    void dealMade(const QString &instrument, int volume);

private slots:
    void onTimer();

public slots:
    double getAvailable();
    void buyMarket(const QString &instrument, int volume, const QString &strategyID);
    void sellMarket(const QString &instrument, int volume, const QString &strategyID);

    // Indicator or strategy has been modified.
    void onModified(const QString &name);
    int getPositionByInstrumentId(const QString &instrument);
    int getPositionByStrategyId(const QString &id);
    QString getInstrumentByStrategyId(const QString &id);
    QStringList getStrategyId(const QString &instrument = QString());
    bool getStrategyEnabled(const QString &id);
    void setStrategyEnabled(const QString &id, bool state);

};

#endif // LAZZYQUANTPROXY_H
