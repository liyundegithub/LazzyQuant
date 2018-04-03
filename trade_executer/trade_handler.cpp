#include <QCoreApplication>

#include "trade_handler.h"

CTradeHandler::CTradeHandler(QObject *obj) :
    receiver(obj)
{
    lastRequestID = -1;
}

CTradeHandler::~CTradeHandler()
{
}

inline void CTradeHandler::postToReceiver(QEvent *event)
{
    QCoreApplication::postEvent(receiver, event);
}

template<class EVT, class F>
void CTradeHandler::handleSingleRsp(F *pField, CThostFtdcRspInfoField *pRspInfo, int nRequestID)
{
    int err = -1;
    if (pRspInfo != NULL) {
        err = pRspInfo->ErrorID;
    }

    if (pField != NULL) {
        postToReceiver(new EVT(pField, err, nRequestID));
    }
}

template<class EVT, class F>
void CTradeHandler::handleMultiRsp(QList<F> *pTList, F *pField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (nRequestID != lastRequestID) {  // 新Rsp
        lastRequestID = nRequestID;
        if (!pTList->isEmpty()) {       // 上一个requestID的消息没有接收全
            postToReceiver(new EVT(*pTList, 0, nRequestID));
            pTList->clear();
        }
        if (pRspInfo != NULL) {
            if (pRspInfo->ErrorID != 0) {
                postToReceiver(new EVT(*pTList, pRspInfo->ErrorID, nRequestID));
                return;
            }
        }
    }

    if (pField != NULL) {
        pTList->append(*pField);
    }
    if (bIsLast) {
        postToReceiver(new EVT(*pTList, 0, nRequestID));
        pTList->clear();
    }
}

void CTradeHandler::OnFrontConnected()
{
    postToReceiver(new FrontConnectedEvent());
}

void CTradeHandler::OnFrontDisconnected(int nReason)
{
    postToReceiver(new FrontDisconnectedEvent(nReason));
}

void CTradeHandler::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<AuthenticateEvent>(pRspAuthenticateField, pRspInfo, nRequestID);
}

void CTradeHandler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<UserLoginEvent>(pRspUserLogin, pRspInfo, nRequestID);
}

void CTradeHandler::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<UserLogoutEvent>(pUserLogout, pRspInfo, nRequestID);
}

void CTradeHandler::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    handleMultiRsp<SettlementInfoEvent>(&settlementInfoList, pSettlementInfo, pRspInfo, nRequestID, bIsLast);
}

void CTradeHandler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<SettlementInfoConfirmEvent>(pSettlementInfoConfirm, pRspInfo, nRequestID);
}

void CTradeHandler::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<SettlementInfoConfirmEvent>(pSettlementInfoConfirm, pRspInfo, nRequestID);
}

void CTradeHandler::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<TradingAccountEvent>(pTradingAccount, pRspInfo, nRequestID);
}

void CTradeHandler::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    handleMultiRsp<RspQryInstrumentMarginRateEvent>(&instrumentMarginRateList, pInstrumentMarginRate, pRspInfo, nRequestID, bIsLast);
}

void CTradeHandler::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    handleMultiRsp<RspQryInstrumentCommissionRateEvent>(&instrumentCommissionRateList, pInstrumentCommissionRate, pRspInfo, nRequestID, bIsLast);
}

void CTradeHandler::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    handleMultiRsp<RspQryInstrumentEvent>(&instrumentList, pInstrument, pRspInfo, nRequestID, bIsLast);
}

void CTradeHandler::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    handleMultiRsp<DepthMarketDataEvent>(&depthMarketDataList, pDepthMarketData, pRspInfo, nRequestID, bIsLast);
}

void CTradeHandler::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<RspOrderInsertEvent>(pInputOrder, pRspInfo, nRequestID);
}

void CTradeHandler::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<RspOrderActionEvent>(pInputOrderAction, pRspInfo, nRequestID);
}

void CTradeHandler::OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<RspParkedOrderInsertEvent>(pParkedOrder, pRspInfo, nRequestID);
}

void CTradeHandler::OnRspParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<RspParkedOrderActionEvent>(pParkedOrderAction, pRspInfo, nRequestID);
}

void CTradeHandler::OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<RspRemoveParkedOrderEvent>(pRemoveParkedOrder, pRspInfo, nRequestID);
}

void CTradeHandler::OnRspRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<RspRemoveParkedOrderActionEvent>(pRemoveParkedOrderAction, pRspInfo, nRequestID);
}

void CTradeHandler::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
    handleSingleRsp<ErrRtnOrderInsertEvent>(pInputOrder, pRspInfo);
}

void CTradeHandler::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
    handleSingleRsp<ErrRtnOrderActionEvent>(pOrderAction, pRspInfo);
}

void CTradeHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo != NULL) {
        postToReceiver(new RspErrorEvent(pRspInfo->ErrorID, nRequestID));
    }
}

void CTradeHandler::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    if (pOrder != NULL) {
        postToReceiver(new RtnOrderEvent(pOrder));
    }
}

void CTradeHandler::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    if (pTrade != NULL) {
        postToReceiver(new RtnTradeEvent(pTrade));
    }
}

void CTradeHandler::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    handleMultiRsp<QryOrderEvent>(&orderList, pOrder, pRspInfo, nRequestID, bIsLast);
}

void CTradeHandler::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    handleMultiRsp<QryTradeEvent>(&tradeList, pTrade, pRspInfo, nRequestID, bIsLast);
}

void CTradeHandler::OnRspQryParkedOrder(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    handleMultiRsp<QryParkedOrderEvent>(&parkedOrderList, pParkedOrder, pRspInfo, nRequestID, bIsLast);
}

void CTradeHandler::OnRspQryParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    handleMultiRsp<QryParkedOrderActionEvent>(&parkedOrderActionList, pParkedOrderAction, pRspInfo, nRequestID, bIsLast);
}

void CTradeHandler::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    handleMultiRsp<PositionEvent>(&positionList, pInvestorPosition, pRspInfo, nRequestID, bIsLast);
}

void CTradeHandler::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    handleMultiRsp<PositionDetailEvent>(&positionDetailList, pInvestorPositionDetail, pRspInfo, nRequestID, bIsLast);
}

void CTradeHandler::OnRspQueryMaxOrderVolume(CThostFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<QryMaxOrderVolumeEvent>(pQueryMaxOrderVolume, pRspInfo, nRequestID);
}

void CTradeHandler::OnRspExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<RspExecOrderInsertEvent>(pInputExecOrder, pRspInfo, nRequestID);
}

void CTradeHandler::OnRspExecOrderAction(CThostFtdcInputExecOrderActionField *pInputExecOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<RspExecOrderActionEvent>(pInputExecOrderAction, pRspInfo, nRequestID);
}

void CTradeHandler::OnRspForQuoteInsert(CThostFtdcInputForQuoteField *pInputForQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Q_UNUSED(bIsLast);
    handleSingleRsp<RspForQuoteInsertEvent>(pInputForQuote, pRspInfo, nRequestID);
}

void CTradeHandler::OnRtnExecOrder(CThostFtdcExecOrderField *pExecOrder)
{
    postToReceiver(new RtnExecOrderEvent(pExecOrder));
}

void CTradeHandler::OnErrRtnExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo)
{
    handleSingleRsp<ErrRtnExecOrderInsertEvent>(pInputExecOrder, pRspInfo);
}

void CTradeHandler::OnErrRtnExecOrderAction(CThostFtdcExecOrderActionField *pExecOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
    handleSingleRsp<ErrRtnExecOrderActionEvent>(pExecOrderAction, pRspInfo);
}

void CTradeHandler::OnErrRtnForQuoteInsert(CThostFtdcInputForQuoteField *pInputForQuote, CThostFtdcRspInfoField *pRspInfo)
{
    handleSingleRsp<ErrRtnForQuoteInsertEvent>(pInputForQuote, pRspInfo);
}
