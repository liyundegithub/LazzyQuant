#ifndef CTP_EXECUTER_H
#define CTP_EXECUTER_H

#include <QObject>
#include <QAtomicInt>
#include <QByteArray>
#include <QMap>
#include <QMutex>
#include <QPair>

#include "common_utility.h"

struct CThostFtdcInstrumentMarginRateField;
struct CThostFtdcInstrumentCommissionRateField;
struct CThostFtdcInstrumentField;

class CThostFtdcTraderApi;
class CTradeHandler;
class Order;
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

    bool preventSelfTrade;
    int orderCancelLimit;
    QMap<QString, int> orderCancelCountMap;
    QMultiMap<QString, Order> orderMap;

    QByteArray brokerID;
    QByteArray userID;
    QByteArray password;
    QByteArray userProductInfo;
    bool useAuthenticate;
    QByteArray authenticateCode;
    char* c_brokerID;
    char* c_userID;
    char* c_password;
    char* c_userProductInfo;
    char* c_authenticateCode;

    QMap<QString, int> ydLongPositions;
    QMap<QString, int> ydShortPositions;
    QMap<QString, int> tdLongPositions;
    QMap<QString, int> tdShortPositions;

    QMap<QString, CThostFtdcInstrumentMarginRateField> marginRateCache;
    QMap<QString, CThostFtdcInstrumentCommissionRateField> commissionRateCache;
    QMap<QString, CThostFtdcInstrumentField> instrumentDataCache;
    QMap<QString, QPair<double, double>> upperLowerLimitCache;

    void customEvent(QEvent *event) override;
    void timesUp(int index);

    template<typename T>
    int callTraderApi(int (CThostFtdcTraderApi::* pTraderApi)(T *,int), T * pField);

    bool loggedIn;
    bool cacheReady;
    double available;

private slots:
    int authenticate();
    int login();
    int qrySettlementInfo();
    int settlementInfoConfirm();
    int qrySettlementInfoConfirm();
    int qryTradingAccount();
    int qryInstrumentMarginRate(const QString &instrument = QString());
    int qryInstrumentCommissionRate(const QString &instrument = QString());
    int qryInstrument(const QString &instrument = QString(), const QString &exchangeID = QString());
    int qryDepthMarketData(const QString &instrument = QString());
    int insertLimitOrder(const QString &instrument, int openClose, int volume, double price, bool allOrAny = false, bool gfdOrIoc = true);
    int insertMarketOrder(const QString &instrument, int openClose, int volume);
    int orderAction(char* orderRef, int frontID, int sessionID, const QString &instrument);
    int insertParkedLimitOrder(const QString &instrument, int openClose, int volume, double price, bool allOrAny = false, bool gfdOrIoc = true);
    int insertParkedOrderAction(char* orderRef, int frontID, int sessionID, const QString &instrument);
    int removeParkedOrder(char* parkedOrderID);
    int removeParkedOrderAction(char* parkedOrderActionID);
    int qryMaxOrderVolume(const QString &instrument, bool direction, int openClose);
    int qryOrder(const QString &instrument = QString());
    int qryTrade(const QString &instrument = QString());
    int qryPosition(const QString &instrument = QString());
    int qryPositionDetail(const QString &instrument = QString());
    int insertExecOrder(const QString &instrument, OPTION_TYPE type, int volume);
    int insertQuote(const QString &instrument);

    bool checkLimitOrder(const QString& instrument, double price, bool direction, int orderType);
    bool distinguishYdTd(const QString &instrument);
    bool canUseMarketOrder(const QString &instrument);

signals:
    void dealMade(const QString& instrument, int volume);

public slots:
    QString getStatus() const;
    bool isLoggedIn() const { return loggedIn; }
    QString getTradingDay() const;
    void confirmSettlementInfo();
    void updateAccountInfo();
    double getAvailable() const { return available; }

    void updateInstrumentDataCache();
    QStringList getCachedInstruments(const QString &idPrefix = QString()) const;
    QString getExchangeID(const QString &instrument);
    QString getExpireDate(const QString &instrument);

    double getUpperLimit(const QString &instrument);
    double getLowerLimit(const QString &instrument);

    void updateOrderMap(const QString &instrument = QString());

    int qryParkedOrder(const QString &instrument = QString(), const QString &exchangeID = QString());
    int qryParkedOrderAction(const QString &instrument = QString(), const QString &exchangeID = QString());

    void buyLimitAuto(const QString& instrument, int volume, double price, int orderType = 0);
    void sellLimitAuto(const QString& instrument, int volume, double price, int orderType = 0);
    void buyLimit(const QString& instrument, int volume, double price, int orderType = 0);
    void sellLimit(const QString& instrument, int volume, double price, int orderType = 0);

    void buyMarketAuto(const QString& instrument, int volume, bool useSimulation = true);
    void sellMarketAuto(const QString& instrument, int volume, bool useSimulation = true);
    void buyMarket(const QString& instrument, int volume, bool useSimulation = true);
    void sellMarket(const QString& instrument, int volume, bool useSimulation = true);

    void cancelOrder(int orderRefID, int frontID, int sessionID, const QString &instrument);
    void cancelAllOrders(const QString &instrument = QString());

    void parkBuyLimit(const QString& instrument, int volume, double price, int orderType = 0);
    void parkSellLimit(const QString& instrument, int volume, double price, int orderType = 0);
    void parkOrderCancel(int orderRefID, int frontID, int sessionID, const QString &instrument);
    void parkOrderCancelAll(const QString &instrument);
    void removeParkedOrder(int id);
    void removeParkedOrderAction(int id);

    void setPosition(const QString& instrument, int newPosition);
    int getPosition(const QString& instrument) const;

    void execOption(const QString &instrument, int volume);
    void quote(const QString &instrument);

    void quit();
};

#endif // CTP_EXECUTER_H
