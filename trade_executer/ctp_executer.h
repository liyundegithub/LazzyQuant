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

    enum LoginState {
        AUTHENTICATED,
        AUTHENTICATING,
        LOGGED_IN,
        LOGGING_IN,
        LOGGED_OUT,
        LOGGING_OUT,
    };
    Q_ENUM(LoginState)

protected:
    QAtomicInt nRequestID = 0;
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
    const char* c_brokerID;
    const char* c_userID;
    const char* c_password;
    const char* c_userProductInfo;
    const char* c_authenticateCode;

    QMap<QString, int> ydLongPositions;
    QMap<QString, int> ydShortPositions;
    QMap<QString, int> tdLongPositions;
    QMap<QString, int> tdShortPositions;

    QMap<QString, CThostFtdcInstrumentMarginRateField> marginRateCache;
    QMap<QString, CThostFtdcInstrumentCommissionRateField> commissionRateCache;
    QMap<QString, CThostFtdcInstrumentField> instrumentDataCache;
    QMap<QString, QPair<double, double>> upperLowerLimitCache;
    QStringList combineInstruments;

    void customEvent(QEvent *event) override;

    template<typename T>
    int callTraderApi(int (CThostFtdcTraderApi::* pTraderApi)(T *,int), T * pField);
    void loginStateMachine();

    bool targetLogin = true;
    LoginState loginState = LOGGED_OUT;
    bool userCacheReady = false;
    bool marketCacheReady = false;
    double available = 0.0;

private slots:
    int authenticate();
    int userLogin();
    int userLogout();
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
    int insertCombineOrder(const QString &instrument, int openClose1, int openClose2, int volume, double price, bool allOrAny = false, bool gfdOrIoc = true);
    int orderAction(const QString &instrument, const char *orderRef, int frontID, int sessionID);
    int insertParkedLimitOrder(const QString &instrument, int openClose, int volume, double price, bool allOrAny = false, bool gfdOrIoc = true);
    int insertParkedOrderAction(const QString &instrument, const char *exchangeID, const char *orderSysID);
    int removeParkedOrder(char *parkedOrderID);
    int removeParkedOrderAction(char *parkedOrderActionID);
    int qryMaxOrderVolume(const QString &instrument, bool direction, int openClose);
    int qryOrder(const QString &instrument = QString());
    int qryTrade(const QString &instrument = QString());
    int qryPosition(const QString &instrument = QString());
    int qryPositionDetail(const QString &instrument = QString());
    int insertExecOrder(const QString &instrument, OPTION_TYPE type, int volume);
    int insertQuote(const QString &instrument);

    bool checkLimitOrder(const QString &instrument, double price, bool direction, int orderType);
    bool distinguishYdTd(const QString &instrument);
    bool canUseMarketOrder(const QString &instrument);

signals:
    void dealMade(const QString& instrument, int volume);

public slots:
    void setLogin();
    void setLogout();
    void onMarketClose();

    QString getStatus() const;
    bool isLoggedIn() const { return loginState == LOGGED_IN; }
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

    void buyLimitAuto(const QString &instrument, int volume, double price, int orderType = LIMIT_ORDER);
    void sellLimitAuto(const QString &instrument, int volume, double price, int orderType = LIMIT_ORDER);
    void buyLimit(const QString &instrument, int volume, double price, int orderType = LIMIT_ORDER);
    void sellLimit(const QString &instrument, int volume, double price, int orderType = LIMIT_ORDER);

    void buyMarketAuto(const QString &instrument, int volume, bool useSimulation = true);
    void sellMarketAuto(const QString &instrument, int volume, bool useSimulation = true);
    void buyMarket(const QString &instrument, int volume, bool useSimulation = true);
    void sellMarket(const QString &instrument, int volume, bool useSimulation = true);

    void buyCombine(const QString &instrument1, const QString &instrument2, int volume, double price, int orderType = LIMIT_ORDER);
    void sellCombine(const QString &instrument1, const QString &instrument2, int volume, double price, int orderType = LIMIT_ORDER);

    void cancelOrder(const QString &instrument, int orderRefID, int frontID, int sessionID);
    void cancelAllOrders(const QString &instrument);

    void parkBuyLimit(const QString &instrument, int volume, double price, int orderType = LIMIT_ORDER);
    void parkSellLimit(const QString &instrument, int volume, double price, int orderType = LIMIT_ORDER);
    void parkOrderCancel(const QString &instrument, const QString &exchangeID, qulonglong orderSysID);
    void parkOrderCancelAll(const QString &instrument);
    void removeParkedOrder(int id);
    void removeParkedOrderAction(int id);

    void setPosition(const QString &instrument, int newPosition);
    int getPosition(const QString &instrument) const;

    void execOption(const QString &instrument, int volume);
    void quote(const QString &instrument);

    void quit();
};

#endif // CTP_EXECUTER_H
