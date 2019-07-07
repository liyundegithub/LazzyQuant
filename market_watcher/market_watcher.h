#ifndef MARKET_WATCHER_H
#define MARKET_WATCHER_H

#include <QObject>
#include <QAtomicInt>
#include <QStringList>
#include <QSet>
#include <QMap>
#include <QTime>

#include "time_mapper.h"

class QSettings;
class CThostFtdcMdApi;
class CTickReceiver;
struct CThostFtdcDepthMarketDataField;
struct CONFIG_ITEM;
class MultipleTimer;

class MarketWatcher : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lazzyquant.market_watcher")

public:
    explicit MarketWatcher(const CONFIG_ITEM &config, bool replayMode = false, QObject *parent = nullptr);
    ~MarketWatcher() override;

protected:
    TimeMapper mapTime;

    const QString name;
    const bool replayMode;
    QSettings *settings;

    QAtomicInt nRequestID;
    CThostFtdcMdApi *pUserApi;
    CTickReceiver *pReceiver;

    bool loggedIn;
    QSet<QString> subscribeSet;
    QMap<QString, QList<QPair<QTime, QTime>>> tradingTimeMap;   // 交易时间段总表.
    QMap<QString, QPair<int, int>> currentTradingTimeMap;   // 当前, 或下一交易时段表.

    bool saveDepthMarketData;
    QString saveDepthMarketDataPath;
    QTime localTime;    // 用来在保存行情数据时生成一个本地的时间戳, 以记录行情到达的先后顺序.
    QMap<QString, QList<CThostFtdcDepthMarketDataField>> depthMarketDataListMap;

    MultipleTimer *multiTimer;
    QList<QStringList> instrumentsToProcess;
    void setupTimers();
    void timesUp(int index);
    void setCurrentTradingTime(const QString &instrumentID);

    QByteArray brokerID;
    QByteArray userID;
    QByteArray password;

    void customEvent(QEvent *) override;

    void login();
    void subscribe();
    bool checkTradingTimes(const QString &instrumentID);
    void processDepthMarketData(const CThostFtdcDepthMarketDataField& depthMarketDataField);
    void emitNewMarketData(const CThostFtdcDepthMarketDataField& depthMarketDataField);

signals:
    void tradingDayChanged(const QString& tradingDay);
    void endOfReplay(const QString& tradingDay);
    void newMarketData(const QString& instrumentID, qint64 time, double lastPrice, int volume,
                       double askPrice1, int askVolume1, double bidPrice1, int bidVolume1);

public slots:
    QString getStatus() const;
    bool isReplayMode() const { return replayMode; }
    bool isLoggedIn() const { return loggedIn; }
    QString getTradingDay() const;
    void setTradingDay(const QString &tradingDay);
    void subscribeInstruments(const QStringList &instruments, bool updateIni = true);
    QStringList getSubscribeList() const;
    void startReplay(const QString &date);
    void quit();
};

#endif // MARKET_WATCHER_H

