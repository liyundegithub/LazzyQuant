#ifndef TRADE_HANDLER_H
#define TRADE_HANDLER_H

#include <QEvent>

#include "ThostFtdcTraderApi.h"

#define FRONT_CONNECTED                     (QEvent::User + 0)
#define FRONT_DISCONNECTED                  (QEvent::User + 1)
#define RSP_AUTHENTICATE                    (QEvent::User + 2)
#define RSP_USER_LOGIN                      (QEvent::User + 3)
#define RSP_USER_LOGOUT                     (QEvent::User + 4)
#define RSP_ERROR                           (QEvent::User + 5)
#define RSP_SETTLEMENT_INFO                 (QEvent::User + 6)
#define RSP_SETTLEMENT_CONFIRM              (QEvent::User + 7)
#define RSP_TRADING_ACCOUNT                 (QEvent::User + 8)
#define RSP_QRY_INSTRUMENT_MARGIN_RATE      (QEvent::User + 9)
#define RSP_QRY_INSTRUMENT_COMMISSION_RATE  (QEvent::User + 10)
#define RSP_QRY_INSTRUMENT                  (QEvent::User + 11)
#define RSP_DEPTH_MARKET_DATA               (QEvent::User + 12)
#define RSP_ORDER_INSERT                    (QEvent::User + 13)
#define RSP_ORDER_ACTION                    (QEvent::User + 14)
#define RSP_PARKED_ORDER_INSERT             (QEvent::User + 15)
#define RSP_PARKED_ORDER_ACTION             (QEvent::User + 16)
#define RSP_REMOVE_PARKED_ORDER             (QEvent::User + 17)
#define RSP_REMOVE_PARKED_ORDER_ACTION      (QEvent::User + 18)
#define ERR_RTN_ORDER_INSERT                (QEvent::User + 19)
#define ERR_RTN_ORDER_ACTION                (QEvent::User + 20)
#define RTN_ORDER                           (QEvent::User + 21)
#define RTN_TRADE                           (QEvent::User + 22)
#define RSP_QRY_ORDER                       (QEvent::User + 23)
#define RSP_QRY_TRADE                       (QEvent::User + 24)
#define RSP_QRY_PARKED_ORDER                (QEvent::User + 25)
#define RSP_QRY_PARKED_ORDER_ACTION         (QEvent::User + 26)
#define RSP_QRY_POSITION                    (QEvent::User + 27)
#define RSP_QRY_POSITION_DETAIL             (QEvent::User + 28)
#define RSP_QRY_MAX_ORDER_VOL               (QEvent::User + 29)
#define RSP_EXEC_ORDER_INSERT               (QEvent::User + 30)
#define RSP_EXEC_ORDER_ACTION               (QEvent::User + 31)
#define RSP_FOR_QUOTE_INSERT                (QEvent::User + 32)
#define RTN_EXEC_ORDER                      (QEvent::User + 33)
#define ERR_RTN_EXEC_ORDER_INSERT           (QEvent::User + 34)
#define ERR_RTN_EXEC_ORDER_ACTION           (QEvent::User + 35)
#define ERR_RTN_FOR_QUOTE_INSERT            (QEvent::User + 36)

struct RspInfo {
    const int errorID;
    const int nRequestID;

    RspInfo(int err, int id)
        : errorID(err), nRequestID(id) {}
};

class FrontConnectedEvent : public QEvent {
public:
    FrontConnectedEvent() :
        QEvent(QEvent::Type(FRONT_CONNECTED)) {}
};

class FrontDisconnectedEvent : public QEvent {
protected:
    const int reason;

public:
    explicit FrontDisconnectedEvent(int Reason) :
        QEvent(QEvent::Type(FRONT_DISCONNECTED)),
        reason(Reason) {}

    int getReason() const { return reason; }
};

class AuthenticateEvent : public QEvent, public RspInfo {
protected:
    const CThostFtdcRspAuthenticateField rspAuthenticate;

public:
    AuthenticateEvent(CThostFtdcRspAuthenticateField *pRspAuthenticate, int err, int id) :
        QEvent(QEvent::Type(RSP_AUTHENTICATE)),
        RspInfo(err, id),
        rspAuthenticate(*pRspAuthenticate) {}
};

class UserLoginEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcRspUserLoginField rspUserLogin;

    UserLoginEvent(CThostFtdcRspUserLoginField *pRspUserLogin, int err, int id) :
        QEvent(QEvent::Type(RSP_USER_LOGIN)),
        RspInfo(err, id),
        rspUserLogin(*pRspUserLogin) {}
};

class SettlementInfoEvent : public QEvent, public RspInfo {
public:
    const QList<CThostFtdcSettlementInfoField> settlementInfoList;

    SettlementInfoEvent(const QList<CThostFtdcSettlementInfoField> &list, int err, int id) :
        QEvent(QEvent::Type(RSP_SETTLEMENT_INFO)),
        RspInfo(err, id),
        settlementInfoList(list) {}
};

class SettlementInfoConfirmEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcSettlementInfoConfirmField settlementInfoConfirm;

    SettlementInfoConfirmEvent(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, int err, int id) :
        QEvent(QEvent::Type(RSP_SETTLEMENT_CONFIRM)),
        RspInfo(err, id),
        settlementInfoConfirm(*pSettlementInfoConfirm) {}
};

class TradingAccountEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcTradingAccountField tradingAccount;

    TradingAccountEvent(CThostFtdcTradingAccountField *pTradingAccount, int err, int id) :
        QEvent(QEvent::Type(RSP_TRADING_ACCOUNT)),
        RspInfo(err, id),
        tradingAccount(*pTradingAccount) {}
};

class RspQryInstrumentMarginRateEvent : public QEvent, public RspInfo {
public:
    const QList<CThostFtdcInstrumentMarginRateField> instrumentMarginRateList;

    RspQryInstrumentMarginRateEvent(const QList<CThostFtdcInstrumentMarginRateField> &list, int err, int id) :
        QEvent(QEvent::Type(RSP_QRY_INSTRUMENT_MARGIN_RATE)),
        RspInfo(err, id),
        instrumentMarginRateList(list) {}
};

class RspQryInstrumentCommissionRateEvent : public QEvent, public RspInfo {
public:
    const QList<CThostFtdcInstrumentCommissionRateField> instrumentCommissionRateList;

    RspQryInstrumentCommissionRateEvent(const QList<CThostFtdcInstrumentCommissionRateField> &list, int err, int id) :
        QEvent(QEvent::Type(RSP_QRY_INSTRUMENT_COMMISSION_RATE)),
        RspInfo(err, id),
        instrumentCommissionRateList(list) {}
};

class RspQryInstrumentEvent : public QEvent, public RspInfo {
public:
    const QList<CThostFtdcInstrumentField> instrumentList;

    RspQryInstrumentEvent(const QList<CThostFtdcInstrumentField> &list, int err, int id) :
        QEvent(QEvent::Type(RSP_QRY_INSTRUMENT)),
        RspInfo(err, id),
        instrumentList(list) {}
};

class DepthMarketDataEvent : public QEvent, public RspInfo {
public:
    const QList<CThostFtdcDepthMarketDataField> depthMarketDataList;

    DepthMarketDataEvent(const QList<CThostFtdcDepthMarketDataField> &list, int err, int id) :
        QEvent(QEvent::Type(RSP_DEPTH_MARKET_DATA)),
        RspInfo(err, id),
        depthMarketDataList(list) {}
};

class RspOrderInsertEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcInputOrderField inputOrderField;

    RspOrderInsertEvent(CThostFtdcInputOrderField *pInputOrder, int err, int id) :
        QEvent(QEvent::Type(RSP_ORDER_INSERT)),
        RspInfo(err, id),
        inputOrderField(*pInputOrder) {}
};

class RspOrderActionEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcInputOrderActionField inputOrderActionField;

    RspOrderActionEvent(CThostFtdcInputOrderActionField *pInputOrderAction, int err, int id) :
        QEvent(QEvent::Type(RSP_ORDER_ACTION)),
        RspInfo(err, id),
        inputOrderActionField(*pInputOrderAction) {}
};

class RspParkedOrderInsertEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcParkedOrderField parkedOrderField;

    RspParkedOrderInsertEvent(CThostFtdcParkedOrderField *pInputOrder, int err, int id) :
        QEvent(QEvent::Type(RSP_PARKED_ORDER_INSERT)),
        RspInfo(err, id),
        parkedOrderField(*pInputOrder) {}
};

class RspParkedOrderActionEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcParkedOrderActionField parkedOrderActionField;

    RspParkedOrderActionEvent(CThostFtdcParkedOrderActionField *pInputOrderAction, int err, int id) :
        QEvent(QEvent::Type(RSP_PARKED_ORDER_ACTION)),
        RspInfo(err, id),
        parkedOrderActionField(*pInputOrderAction) {}
};

class RspRemoveParkedOrderEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcRemoveParkedOrderField removeParkedOrderField;

    RspRemoveParkedOrderEvent(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, int err, int id) :
        QEvent(QEvent::Type(RSP_REMOVE_PARKED_ORDER)),
        RspInfo(err, id),
        removeParkedOrderField(*pRemoveParkedOrder) {}
};

class RspRemoveParkedOrderActionEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcRemoveParkedOrderActionField removeParkedOrderActionField;

    RspRemoveParkedOrderActionEvent(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, int err, int id) :
        QEvent(QEvent::Type(RSP_REMOVE_PARKED_ORDER)),
        RspInfo(err, id),
        removeParkedOrderActionField(*pRemoveParkedOrderAction) {}
};

class ErrRtnOrderInsertEvent: public QEvent, public RspInfo {
public:
    const CThostFtdcInputOrderField inputOrderField;

    ErrRtnOrderInsertEvent(CThostFtdcInputOrderField *pInputOrder, int err, int id) :
        QEvent(QEvent::Type(ERR_RTN_ORDER_INSERT)),
        RspInfo(err, id),
        inputOrderField(*pInputOrder) {}
};

class ErrRtnOrderActionEvent: public QEvent, public RspInfo {
public:
    const CThostFtdcOrderActionField inputOrderField;

    ErrRtnOrderActionEvent(CThostFtdcOrderActionField *pOrderAction, int err, int id) :
        QEvent(QEvent::Type(ERR_RTN_ORDER_ACTION)),
        RspInfo(err, id),
        inputOrderField(*pOrderAction) {}
};

class RtnOrderEvent : public QEvent {
public:
    const CThostFtdcOrderField orderField;

    explicit RtnOrderEvent(CThostFtdcOrderField *pOrderField) :
        QEvent(QEvent::Type(RTN_ORDER)),
        orderField(*pOrderField) {}
};

class RtnTradeEvent : public QEvent {
public:
    const CThostFtdcTradeField tradeField;

    explicit RtnTradeEvent(CThostFtdcTradeField *pTradeField) :
        QEvent(QEvent::Type(RTN_TRADE)),
        tradeField(*pTradeField) {}
};

class QryOrderEvent : public QEvent, public RspInfo {
public:
    const QList<CThostFtdcOrderField> orderList;

    QryOrderEvent(const QList<CThostFtdcOrderField> &list, int err, int id) :
        QEvent(QEvent::Type(RSP_QRY_ORDER)),
        RspInfo(err, id),
        orderList(list) {}
};

class QryTradeEvent : public QEvent, public RspInfo {
public:
    const QList<CThostFtdcTradeField> tradeList;

    QryTradeEvent(const QList<CThostFtdcTradeField> &list, int err, int id) :
        QEvent(QEvent::Type(RSP_QRY_TRADE)),
        RspInfo(err, id),
        tradeList(list) {}
};

class QryParkedOrderEvent : public QEvent, public RspInfo {
public:
    const QList<CThostFtdcParkedOrderField> parkedOrderList;

    QryParkedOrderEvent(const QList<CThostFtdcParkedOrderField> &list, int err, int id) :
        QEvent(QEvent::Type(RSP_QRY_PARKED_ORDER)),
        RspInfo(err, id),
        parkedOrderList(list) {}
};

class QryParkedOrderActionEvent : public QEvent, public RspInfo {
public:
    const QList<CThostFtdcParkedOrderActionField> parkedOrderActionList;

    QryParkedOrderActionEvent(const QList<CThostFtdcParkedOrderActionField> &list, int err, int id) :
        QEvent(QEvent::Type(RSP_QRY_PARKED_ORDER_ACTION)),
        RspInfo(err, id),
        parkedOrderActionList(list) {}
};

class PositionEvent : public QEvent, public RspInfo {
public:
    const QList<CThostFtdcInvestorPositionField> positionList;

    PositionEvent(const QList<CThostFtdcInvestorPositionField> &list, int err, int id) :
        QEvent(QEvent::Type(RSP_QRY_POSITION)),
        RspInfo(err, id),
        positionList(list) {}
};

class PositionDetailEvent : public QEvent, public RspInfo {
public:
    const QList<CThostFtdcInvestorPositionDetailField> positionDetailList;

    PositionDetailEvent(const QList<CThostFtdcInvestorPositionDetailField> &list, int err, int id) :
        QEvent(QEvent::Type(RSP_QRY_POSITION_DETAIL)),
        RspInfo(err, id),
        positionDetailList(list) {}
};

class QryMaxOrderVolumeEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcQueryMaxOrderVolumeField maxOrderVolumeField;

    QryMaxOrderVolumeEvent(CThostFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume, int err, int id) :
        QEvent(QEvent::Type(RSP_QRY_MAX_ORDER_VOL)),
        RspInfo(err, id),
        maxOrderVolumeField(*pQueryMaxOrderVolume) {}
};

class RspExecOrderInsertEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcInputExecOrderField inputExecOrderField;

    RspExecOrderInsertEvent(CThostFtdcInputExecOrderField *pInputExecOrder, int err, int id) :
        QEvent(QEvent::Type(RSP_EXEC_ORDER_INSERT)),
        RspInfo(err, id),
        inputExecOrderField(*pInputExecOrder) {}
};

class RspExecOrderActionEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcInputExecOrderActionField inputExecOrderActionField;

    RspExecOrderActionEvent(CThostFtdcInputExecOrderActionField *pInputExecOrderAction, int err, int id) :
        QEvent(QEvent::Type(RSP_EXEC_ORDER_ACTION)),
        RspInfo(err, id),
        inputExecOrderActionField(*pInputExecOrderAction) {}
};

class RspForQuoteInsertEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcInputForQuoteField inputForQuoteField;

    RspForQuoteInsertEvent(CThostFtdcInputForQuoteField *pInputForQuote, int err, int id) :
        QEvent(QEvent::Type(RSP_FOR_QUOTE_INSERT)),
        RspInfo(err, id),
        inputForQuoteField(*pInputForQuote) {}
};

class RtnExecOrderEvent : public QEvent {
public:
    const CThostFtdcExecOrderField execOrderField;

    explicit RtnExecOrderEvent(CThostFtdcExecOrderField *pExecOrderField) :
        QEvent(QEvent::Type(RTN_EXEC_ORDER)),
        execOrderField(*pExecOrderField) {}
};

class ErrRtnExecOrderInsertEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcInputExecOrderField pInputExecOrderField;

    ErrRtnExecOrderInsertEvent(CThostFtdcInputExecOrderField *pInputExecOrder, int err, int id) :
        QEvent(QEvent::Type(ERR_RTN_EXEC_ORDER_INSERT)),
        RspInfo(err, id),
        pInputExecOrderField(*pInputExecOrder) {}
};

class ErrRtnExecOrderActionEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcExecOrderActionField pExecOrderActionField;

    ErrRtnExecOrderActionEvent(CThostFtdcExecOrderActionField *pExecOrderAction, int err, int id) :
        QEvent(QEvent::Type(ERR_RTN_EXEC_ORDER_ACTION)),
        RspInfo(err, id),
        pExecOrderActionField(*pExecOrderAction) {}
};

class ErrRtnForQuoteInsertEvent : public QEvent, public RspInfo {
public:
    const CThostFtdcInputForQuoteField pInputForQuoteField;

    ErrRtnForQuoteInsertEvent(CThostFtdcInputForQuoteField *pInputForQuote, int err, int id) :
        QEvent(QEvent::Type(ERR_RTN_FOR_QUOTE_INSERT)),
        RspInfo(err, id),
        pInputForQuoteField(*pInputForQuote) {}
};

class CTradeHandler : public CThostFtdcTraderSpi {
    QObject * const receiver;

    int lastRequestID;
    QList<CThostFtdcSettlementInfoField> settlementInfoList;
    QList<CThostFtdcInstrumentMarginRateField> instrumentMarginRateList;
    QList<CThostFtdcInstrumentCommissionRateField> instrumentCommissionRateList;
    QList<CThostFtdcInstrumentField> instrumentList;
    QList<CThostFtdcDepthMarketDataField> depthMarketDataList;
    QList<CThostFtdcOrderField> orderList;
    QList<CThostFtdcTradeField> tradeList;
    QList<CThostFtdcParkedOrderField> parkedOrderList;
    QList<CThostFtdcParkedOrderActionField> parkedOrderActionList;
    QList<CThostFtdcInvestorPositionField> positionList;
    QList<CThostFtdcInvestorPositionDetailField> positionDetailList;

public:
    explicit CTradeHandler(QObject *obj);
    ~CTradeHandler();

    void postToReceiver(QEvent *event);

    template<class EVT, class F>
    void handleSingleRsp(F *pField, CThostFtdcRspInfoField *pRspInfo, int nRequestID = -1);

    template<class EVT, class F>
    void handleMultiRsp(QList<F> *pTList, F *pField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnFrontConnected();
    void OnFrontDisconnected(int nReason);

    void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);
    void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo);
    void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRtnOrder(CThostFtdcOrderField *pOrder);
    void OnRtnTrade(CThostFtdcTradeField *pTrade);

    void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryParkedOrder(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQueryMaxOrderVolume(CThostFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspExecOrderAction(CThostFtdcInputExecOrderActionField *pInputExecOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspForQuoteInsert(CThostFtdcInputForQuoteField *pInputForQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRtnExecOrder(CThostFtdcExecOrderField *pExecOrder);

    void OnErrRtnExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo);
    void OnErrRtnExecOrderAction(CThostFtdcExecOrderActionField *pExecOrderAction, CThostFtdcRspInfoField *pRspInfo);
    void OnErrRtnForQuoteInsert(CThostFtdcInputForQuoteField *pInputForQuote, CThostFtdcRspInfoField *pRspInfo);
};

#endif // TRADE_HANDLER_H
