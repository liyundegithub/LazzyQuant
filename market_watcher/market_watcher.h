#ifndef MARKET_WATCHER_H
#define MARKET_WATCHER_H

#include <QObject>
#include <QAtomicInt>
#include <QStringList>
#include <QSet>
#include <QMap>

class QSettings;
class QTimer;
class QTime;
class CThostFtdcMdApi;
class CTickReceiver;
struct CThostFtdcDepthMarketDataField;
struct CONFIG_ITEM;
class MultipleTimer;

class MarketWatcher : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lazzyquant.market_watcher")

public:
    explicit MarketWatcher(const CONFIG_ITEM &config, const bool replayMode = false, QObject *parent = 0);
    ~MarketWatcher();

protected:
    const bool replayMode;
    QSettings *settings;

    QAtomicInt nRequestID;
    CThostFtdcMdApi *pUserApi;
    CTickReceiver *pReceiver;

    bool loggedIn;
    QSet<QString> subscribeSet;
    QMap<QString, QList<QPair<QTime, QTime>>> tradingTimeMap;   // 交易时间段总表
    QMap<QString, QPair<QTime, QTime>> currentTradingTimeMap;   // 当前, 或下一交易时段表

    bool saveDepthMarketData;
    QString saveDepthMarketDataPath;
    QMap<QString, QList<CThostFtdcDepthMarketDataField>> depthMarketDataListMap;

    MultipleTimer *multiTimer;
    QList<QStringList> instrumentsToProcess;
    void setupTimers();
    void timesUp(int index);
    void setCurrentTradingTime(const QString &instrumentID);

    QByteArray brokerID;
    QByteArray userID;
    QByteArray password;
    char* c_brokerID;
    char* c_userID;
    char* c_password;

    void customEvent(QEvent *) override;

    void login();
    void subscribe();
    bool checkTradingTimes(const QString &instrumentID);
    void processDepthMarketData(const CThostFtdcDepthMarketDataField& depthMarketDataField);
    void emitNewMarketData(const CThostFtdcDepthMarketDataField& depthMarketDataField);

signals:
    void heartBeatWarning(int nTimeLapse);
    void newMarketData(const QString& instrumentID, uint time, double lastPrice, int volume,
                       double askPrice1, int askVolume1, double bidPrice1, int bidVolume1);

public slots:
    bool isReplayMode() const { return replayMode; }
    bool isLoggedIn() const { return loggedIn; }
    QString getTradingDay() const;
    void subscribeInstruments(const QStringList &instruments, bool updateIni = true);
    QStringList getSubscribeList() const;
    void startReplay(const QString &date, bool realSpeed = false);
    void quit();
};

#endif // MARKET_WATCHER_H

