#ifndef CTP_EXECUTER_H
#define CTP_EXECUTER_H

#include <QObject>
#include <QAtomicInt>
#include <QByteArray>
#include <QMap>
#include <QMutex>
#include <QDateTime>
#include <QPair>

struct CThostFtdcInstrumentField;
class CThostFtdcTraderApi;
class CTradeHandler;
class Order;
template<class T> class Expires;
struct CONFIG_ITEM;

class CtpExecuter : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lazzyquant.trade_executer")
public:
    explicit CtpExecuter(const CONFIG_ITEM &config, QObject *parent = 0);
    ~CtpExecuter();

protected:
    QAtomicInt nRequestID;
    QMutex traderApiMutex;
    CThostFtdcTraderApi *pUserApi;
    CTradeHandler *pHandler;

    QByteArray brokerID;
    QByteArray userID;
    QByteArray password;
    char* c_brokerID;
    char* c_userID;
    char* c_password;

    int FrontID;
    int SessionID;

    QMap<QString, int> target_pos_map;
    QMap<QString, int> yd_pos_map;
    QMap<QString, int> td_pos_map;
    QDateTime pos_update_time;
    QMultiMap<QString, Expires<Order>> order_map;
    QMap<QString, Expires<QPair<double, double>>> upper_lower_limit_map;
    QMap<QString, CThostFtdcInstrumentField> instruments_cache_map;

    void customEvent(QEvent *event) override;

    template<typename T>
    int callTraderApi(int (CThostFtdcTraderApi::* pTraderApi)(T *,int), T * pField);

    bool loggedIn;
    double available;
    bool allowToTrade;

private slots:
    int login();
    int qrySettlementInfo();
    int qrySettlementInfoConfirm();
    int qryInstrumentCommissionRate(const QString &instrument = QString());
    int qryInstrument(const QString &instrument = QString(), const QString &exchangeID = QString());
    int qryDepthMarketData(const QString &instrument = QString());
    int insertLimitOrder(const QString &instrument, bool open, int volume, double price, bool allOrAny = false, bool gfdOrIoc = true);
    int cancelOrder(char* orderRef, int frontID, int sessionID, const QString &instrument);
    int qryMaxOrderVolume(const QString &instrument, bool buy, char offsetFlag);
    int qryOrder(const QString &instrument = QString());
    int qryTrade(const QString &instrument = QString());
    int qryPosition(const QString &instrument = QString());
    int qryPositionDetail(const QString &instrument = QString());

    QDateTime getExpireTime() const;
    void operate(const QString &instrument, int new_position);

signals:
    void heartBeatWarning(int nTimeLapse);
    void dealMade(const QString& instrument, int volume);

public slots:
    bool isLoggedIn() const { return loggedIn; }
    QString getTradingDay() const;
    int confirmSettlementInfo();
    int qryTradingAccount();
    double getAvailable() const { return available; }
    void updateInstrumentsCache(const QStringList& instruments);
    QStringList getCachedInstruments(const QString &idPrefix = QString()) const;
    void buyLimit(const QString& instrument, int volume, double price, int orderType = 0);
    void sellLimit(const QString& instrument, int volume, double price, int orderType = 0);
    void setPosition(const QString& instrument, int new_position);
    int getPosition(const QString& instrument) const;
    int getPendingOrderVolume(const QString &instrument) const;
    void switchOn();
    void switchOff();
    void quit();
};

#endif // CTP_EXECUTER_H
