#ifndef MARKET_WATCHER_H
#define MARKET_WATCHER_H

#include <QObject>
#include <QAtomicInt>
#include <QStringList>
#include <QSet>
#include <QMultiMap>

class QTimer;
class QTime;
class CThostFtdcMdApi;
class CTickReceiver;
struct CThostFtdcDepthMarketDataField;
struct CONFIG_ITEM;

class MarketWatcher : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lazzyquant.market_watcher")
public:
    explicit MarketWatcher(const CONFIG_ITEM &config, QObject *parent = 0);
    ~MarketWatcher();

protected:
    QAtomicInt nRequestID;
    CThostFtdcMdApi *pUserApi;
    CTickReceiver *pReceiver;
    QSet<QString> subscribeSet;
    QMap<QString, QList<QPair<QTime, QTime>>> tradingTimeMap;

    bool saveDepthMarketData;
    QString saveDepthMarketDataPath;
    QMap<QString, QList<CThostFtdcDepthMarketDataField>> depthMarketDataListMap;
    QTimer *saveBarTimer;
    QList<QTime> saveBarTimePoints;
    int saveBarTimeIndex;
    void setSaveDepthMarketData();
    void saveDepthMarketDataAndResetTimer();

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
    void processDepthMarketData(const CThostFtdcDepthMarketDataField&);

signals:
    void heartBeatWarning(int nTimeLapse);
    void newMarketData(const QString& instrumentID, uint time, double lastPrice, int volume,
                       double askPrice1, int askVolume1, double bidPrice1, int bidVolume1,
                       double askPrice2, int askVolume2, double bidPrice2, int bidVolume2);

public slots:
    QString getTradingDay() const;
    QStringList getSubscribeList() const;
    void quit();
};

#endif // MARKET_WATCHER_H

