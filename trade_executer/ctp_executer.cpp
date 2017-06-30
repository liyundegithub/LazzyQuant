#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cfloat>
#include <QSettings>
#include <QtConcurrentRun>
#include <QTimer>
#include <QTextCodec>
#include <QCoreApplication>

#include "config_struct.h"
#include "multiple_timer.h"
#include "ctp_executer.h"
#include "trade_executer_adaptor.h"
#include "trade_handler.h"
#include "order.h"

#define OPEN                0
#define CLOSE               1
#define CLOSE_TODAY         2
#define CLOSE_YESTERDAY     3

const TThostFtdcOffsetFlagType ctpOffsetFlags[] = {
    THOST_FTDC_OF_Open,
    THOST_FTDC_OF_Close,
    THOST_FTDC_OF_CloseToday,
    THOST_FTDC_OF_CloseYesterday,
};

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

/*!
 * \brief _sleep
 * 暂停当前线程. 代码参考QTestLib模块, 不要在主线程中调用.
 *
 * \param ms 暂停时长(<932毫秒)
 */
static inline void _sleep(int ms)
{
#ifdef Q_OS_WIN
    Sleep(uint(ms));
#else
    struct timespec ts = { 0, (ms % 1024) * 1024 * 1024 };
    nanosleep(&ts, NULL);
#endif
}

CtpExecuter::CtpExecuter(const CONFIG_ITEM &config, QObject *parent) :
    QObject(parent)
{
    nRequestID = 0;

    loggedIn = false;
    cacheReady = false;
    available = 0.0f;

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, config.organization, config.name);
    QByteArray flowPath = settings.value("FlowPath").toByteArray();
    preventSelfTrade = settings.value("PreventSelfTrade", 1).toBool();
    orderCancelLimit = settings.value("OrderCancelLimit", 300).toInt();

    settings.beginGroup("AccountInfo");
    brokerID = settings.value("BrokerID").toByteArray();
    userID = settings.value("UserID").toByteArray();
    password = settings.value("Password").toByteArray();
    userProductInfo = settings.value("UserProductInfo").toByteArray();
    useAuthenticate = settings.value("UseAuthenticate").toBool();
    authenticateCode = settings.value("AuthenticateCode").toByteArray();
    settings.endGroup();

    // Pre-convert QString to char*
    c_brokerID = brokerID.data();
    c_userID = userID.data();
    c_password = password.data();
    c_userProductInfo = userProductInfo.data();
    c_authenticateCode = authenticateCode.data();

    pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi(flowPath.data());
    pHandler = new CTradeHandler(this);
    pUserApi->RegisterSpi(pHandler);

    pUserApi->SubscribePrivateTopic(THOST_TERT_QUICK);
    pUserApi->SubscribePublicTopic(THOST_TERT_QUICK);

    settings.beginGroup("FrontSites");
    const auto keys = settings.childKeys();
    const QString protocol = "tcp://";
    for (const auto &str : keys) {
        QString address = settings.value(str).toString();
        pUserApi->RegisterFront((protocol + address).toLatin1().data());
    }
    settings.endGroup();

    new Trade_executerAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(config.dbusObject, this);
    dbus.registerService(config.dbusService);

    pUserApi->Init();

    QList<QTime> timePoints = { QTime(8, 50), QTime(20, 50) };
    auto multiTimer = new MultipleTimer(timePoints, this);
    connect(multiTimer, &MultipleTimer::timesUp, this, &CtpExecuter::timesUp);
}

CtpExecuter::~CtpExecuter()
{
    pUserApi->Release();
    delete pHandler;
}

void CtpExecuter::customEvent(QEvent *event)
{
    qDebug() << "customEvent: " << int(event->type());
    switch (int(event->type())) {
    case FRONT_CONNECTED:
        if (useAuthenticate) {
            authenticate();
        } else {
            login();
        }
        break;
    case FRONT_DISCONNECTED:
    {
        loggedIn = false;
        cacheReady = false;
        auto *fevent = static_cast<FrontDisconnectedEvent*>(event);
        // TODO reset position maps, make update times invalid
        switch (fevent->getReason()) {
        case 0x1001: // 网络读失败
            break;
        case 0x1002: // 网络写失败
            break;
        case 0x2001: // 接收心跳超时
            break;
        case 0x2002: // 发送心跳失败
            break;
        case 0x2003: // 收到错误报文
            break;
        default:
            break;
        }
    }
        break;
    case RSP_AUTHENTICATE:
    {
        auto *aevent = static_cast<AuthenticateEvent*>(event);
        if (aevent->errorID == 0) {
            qInfo() << DATE_TIME << "Authenticate OK!";
            login();
        } else {
            qWarning() << DATE_TIME << "Authenticate failed! Error ID =" << aevent->errorID;
        }
    }
        break;
    case RSP_USER_LOGIN:
    {
        auto *uevent = static_cast<UserLoginEvent*>(event);
        if (uevent->errorID == 0) {
            loggedIn = true;
            cacheReady = false;

            qInfo() << DATE_TIME << "UserLogin OK! FrontID =" << uevent->rspUserLogin.FrontID << ", SessionID =" << uevent->rspUserLogin.SessionID;

            settlementInfoConfirm();
            QTimer::singleShot(1000, this, SLOT(updateOrderMap()));
            QTimer::singleShot(2000, this, SLOT(qryPositionDetail()));
            QTimer::singleShot(5000, this, &CtpExecuter::updateInstrumentDataCache);
        } else {
            qWarning() << DATE_TIME << "UserLogin failed! Error ID =" << uevent->errorID;
        }
    }
        break;
    case RSP_USER_LOGOUT:
        break;
    case RSP_ERROR:
        break;
    case RSP_SETTLEMENT_INFO:
    {
        auto *sevent = static_cast<SettlementInfoEvent*>(event);
        if (sevent->errorID == 0) {
            const auto &list = sevent->settlementInfoList;
            QString msg;
            for (const auto & item : list) {
                msg += QTextCodec::codecForName("GBK")->toUnicode(item.Content);
            }
            qDebug() << msg;
        }
    }
        break;
    case RSP_TRADING_ACCOUNT:
    {
        auto *tevent = static_cast<TradingAccountEvent*>(event);
        available = tevent->tradingAccount.Available;
        qDebug() << "available = " << available;
    }
        break;
    case RSP_QRY_INSTRUMENT_MARGIN_RATE:
    {
        auto *qmevent = static_cast<RspQryInstrumentMarginRateEvent*>(event);
        for (const auto &item : qmevent->instrumentMarginRateList) {
            marginRateCache[item.InstrumentID] = item;
        }
        qInfo() << DATE_TIME << " Updated" << qmevent->instrumentMarginRateList.size() << "instrument margin rate!";
    }
        break;
    case RSP_QRY_INSTRUMENT_COMMISSION_RATE:
    {
        auto *qcevent = static_cast<RspQryInstrumentCommissionRateEvent*>(event);
        for (const auto &item : qcevent->instrumentCommissionRateList) {
            commissionRateCache[item.InstrumentID] = item;
        }
        qInfo() << DATE_TIME << " Updated" << qcevent->instrumentCommissionRateList.size() << "instrument commission rate!";
    }
        break;
    case RSP_QRY_INSTRUMENT:
    {
        auto *qievent = static_cast<RspQryInstrumentEvent*>(event);
        for (const auto &item : qievent->instrumentList) {
            instrumentDataCache[item.InstrumentID] = item;
        }
        qInfo() << DATE_TIME << " Updated" << qievent->instrumentList.size() << "instruments!";
    }
        break;
    case RSP_DEPTH_MARKET_DATA:
    {
        auto *devent = static_cast<DepthMarketDataEvent*>(event);
        for (const auto &item : devent->depthMarketDataList) {
            const QString instrument = item.InstrumentID;
            upperLowerLimitCache[instrument] = qMakePair(item.UpperLimitPrice, item.LowerLimitPrice);
        }
        qInfo() << DATE_TIME << " Updated" << devent->depthMarketDataList.size() << "depth market data!";
    }
        break;
    case RSP_ORDER_INSERT:
    {
        auto *ievent = static_cast<RspOrderInsertEvent*>(event);
        qWarning() << DATE_TIME << "Order insert failed! errorID =" << ievent->errorID;
    }
        break;
    case RSP_ORDER_ACTION:
    {
        auto *aevent = static_cast<RspOrderActionEvent*>(event);
        qWarning() << DATE_TIME << "Order cancel failed! errorID =" << aevent->errorID;
    }
        break;
    case RSP_PARKED_ORDER_INSERT:
    {
        auto *pievent = static_cast<RspParkedOrderInsertEvent*>(event);
        qDebug() << (pievent->errorID);
    }
        break;
    case RSP_PARKED_ORDER_ACTION:
    {
        auto *paevent = static_cast<RspParkedOrderActionEvent*>(event);
    }
        break;
    case RSP_REMOVE_PARKED_ORDER:
    {
        auto *rpevent = static_cast<RspRemoveParkedOrderEvent*>(event);
    }
        break;
    case RSP_REMOVE_PARKED_ORDER_ACTION:
    {
        auto *rpaevent = static_cast<RspRemoveParkedOrderActionEvent*>(event);
    }
        break;
    case ERR_RTN_ORDER_INSERT:
    {
        auto *eievent = static_cast<ErrRtnOrderInsertEvent*>(event);
        qWarning() << DATE_TIME << "Order insert failed! errorID =" << eievent->errorID;
    }
        break;
    case ERR_RTN_ORDER_ACTION:
    {
        auto *eaevent = static_cast<ErrRtnOrderActionEvent*>(event);
        qWarning() << DATE_TIME << "Order cancel failed! errorID =" << eaevent->errorID;
    }
        break;
    case RTN_ORDER:
    {
        auto *revent = static_cast<RtnOrderEvent*>(event);
        qDebug() << revent->orderField.VolumeTotal << revent->orderField.InsertTime << revent->orderField.OrderStatus <<
                    QTextCodec::codecForName("GBK")->toUnicode(revent->orderField.StatusMsg);

        int refId;
        int frontId = revent->orderField.FrontID;
        int sessionId = revent->orderField.SessionID;
        sscanf(revent->orderField.OrderRef, "%12d", &refId);

        bool updated = false;
        for (auto &item : orderMap) {
            if (item.refId == refId && item.frontId == frontId && item.sessionId == sessionId) {
                item.updateStatus(revent->orderField);
                updated = true;
            }
        }
        if (!updated) {
            orderMap.insert(revent->orderField.InstrumentID, revent->orderField);
        }
    }
        break;
    case RTN_TRADE:
    {
        auto *tevent = static_cast<RtnTradeEvent*>(event);
        int volume = tevent->tradeField.Volume;
        const bool direction = (tevent->tradeField.Direction == THOST_FTDC_D_Buy);

        switch(tevent->tradeField.OffsetFlag) {
        case THOST_FTDC_OF_Open:            //开仓
            if (direction) {
                tdLongPositions[tevent->tradeField.InstrumentID] += volume;
            } else {
                tdShortPositions[tevent->tradeField.InstrumentID] += volume;
            }
            break;
        case THOST_FTDC_OF_Close:           //平仓
            // 默认先平昨仓, 再平今仓
            // TODO 在考虑平今手续费减免的情况下, 应当先平今仓, 再平昨仓
            if (direction) {
                int ydPosition = ydShortPositions[tevent->tradeField.InstrumentID];
                int closeYd = qMin(ydPosition, volume);
                if (closeYd > 0) {
                    ydShortPositions[tevent->tradeField.InstrumentID] -= closeYd;
                    volume -= closeYd;
                }
                tdShortPositions[tevent->tradeField.InstrumentID] -= volume;
            } else {
                int ydPosition = ydLongPositions[tevent->tradeField.InstrumentID];
                int closeYd = qMin(ydPosition, volume);
                if (closeYd > 0) {
                    ydLongPositions[tevent->tradeField.InstrumentID] -= closeYd;
                    volume -= closeYd;
                }
                tdLongPositions[tevent->tradeField.InstrumentID] -= volume;
            }
            break;
        case THOST_FTDC_OF_CloseToday:      //平今
            if (direction) {
                tdShortPositions[tevent->tradeField.InstrumentID] -= volume;
            } else {
                tdLongPositions[tevent->tradeField.InstrumentID] -= volume;
            }
            break;
        case THOST_FTDC_OF_CloseYesterday:  //平昨
            if (direction) {
                ydShortPositions[tevent->tradeField.InstrumentID] -= volume;
            } else {
                ydLongPositions[tevent->tradeField.InstrumentID] -= volume;
            }
            break;
        default:
            // FIXME 默认当平今处理
            if (direction) {
                tdShortPositions[tevent->tradeField.InstrumentID] -= volume;
            } else {
                tdLongPositions[tevent->tradeField.InstrumentID] -= volume;
            }
            break;
        }

        emit dealMade(tevent->tradeField.InstrumentID, tevent->tradeField.Volume * (direction ? 1 : -1));
    }
        break;
    case RSP_QRY_ORDER:
    {
        auto *qoevent = static_cast<QryOrderEvent*>(event);

        for (const auto &item : qoevent->orderList) {
            orderMap.insert(item.InstrumentID, item);
            qDebug() << item.InstrumentID << QTextCodec::codecForName("GBK")->toUnicode(item.StatusMsg);
            qDebug() << item.OrderRef << item.FrontID << item.SessionID;
        }
    }
        break;
    case RSP_QRY_TRADE:
    {
        auto *qtevent = static_cast<QryTradeEvent*>(event);
    }
        break;
    case RSP_QRY_PARKED_ORDER:
    {
        auto *qpevent = static_cast<QryParkedOrderEvent*>(event);
        parkedOrders = qpevent->parkedOrderList;
        for (const auto &item : parkedOrders) {
            qDebug() << item.ParkedOrderID << item.InstrumentID << item.Direction << item.LimitPrice << item.VolumeTotalOriginal;
        }
    }
        break;
    case RSP_QRY_PARKED_ORDER_ACTION:
    {
        auto *qpaevent = static_cast<QryParkedOrderActionEvent*>(event);
        parkedOrderActions = qpaevent->parkedOrderActionList;
    }
        break;
    case RSP_QRY_POSITION:
    {
        auto *pevent = static_cast<PositionEvent*>(event);
        ydLongPositions.clear();
        ydShortPositions.clear();
        tdLongPositions.clear();
        tdShortPositions.clear();

        for (const auto &item : pevent->positionList) {
            if (item.PosiDirection == THOST_FTDC_PD_Long) {
                ydLongPositions[item.InstrumentID] += item.Position - item.TodayPosition;
                tdLongPositions[item.InstrumentID] += item.TodayPosition;
            } else {
                ydShortPositions[item.InstrumentID] += item.Position - item.TodayPosition;
                tdShortPositions[item.InstrumentID] += item.TodayPosition;
            }
        }
    }
        break;
    case RSP_QRY_POSITION_DETAIL:
    {
        auto *pevent = static_cast<PositionDetailEvent*>(event);
        ydLongPositions.clear();
        ydShortPositions.clear();
        tdLongPositions.clear();
        tdShortPositions.clear();

        for (const auto &item : pevent->positionDetailList) {
            qDebug() << item.InstrumentID << "position:" << item.Volume << item.OpenDate << item.TradingDay << item.ExchangeID;

            if (item.Direction == THOST_FTDC_D_Buy) {
                if (strcmp(item.OpenDate, item.TradingDay) == 0) {
                    tdLongPositions[item.InstrumentID] += item.Volume;
                } else {
                    ydLongPositions[item.InstrumentID] += item.Volume;
                }
            } else {
                if (strcmp(item.OpenDate, item.TradingDay) == 0) {
                    tdShortPositions[item.InstrumentID] += item.Volume;
                } else {
                    ydShortPositions[item.InstrumentID] += item.Volume;
                }
            }
        }
    }
        break;
    case RSP_QRY_MAX_ORDER_VOL:
    {
        auto *qmevent = static_cast<QryMaxOrderVolumeEvent*>(event);
        qDebug() << qmevent->maxOrderVolumeField.InstrumentID << qmevent->maxOrderVolumeField.MaxVolume;
    }
        break;
    case RSP_EXEC_ORDER_INSERT:
        break;
    case RSP_EXEC_ORDER_ACTION:
        break;
    case RSP_FOR_QUOTE_INSERT:
        break;
    case RTN_EXEC_ORDER:
        break;
    case ERR_RTN_EXEC_ORDER_INSERT:
        break;
    case ERR_RTN_EXEC_ORDER_ACTION:
        break;
    case ERR_RTN_FOR_QUOTE_INSERT:
        break;
    default:
        QObject::customEvent(event);
        break;
    }
}

void CtpExecuter::timesUp(int index)
{
    switch (index) {
    case 0: // Before 9:00
    case 1: // Before 21:00
        // TODO 检查是否是交易日
        if (!loggedIn) {
            login();
        }
        break;
    default:
        qWarning() << "Should never see this! Something is wrong! index =" << index;
        break;
    }
}

/*!
 * \brief CtpExecuter::callTraderApi
 * 尝试调用pTraderApi, 如果失败(返回值不是0),
 * 就在一个新线程里反复调用pTraderApi, 直至成功
 *
 * \param pTraderApi CThostFtdcTraderApi类的成员函数指针
 * \param pField pTraderApi函数的第一个参数，成功调用pTraderApi或超时之后释放
 */
template<typename T>
int CtpExecuter::callTraderApi(int (CThostFtdcTraderApi::* pTraderApi)(T *,int), T * pField)
{
    int id = nRequestID.fetchAndAddRelaxed(1);

    if (traderApiMutex.tryLock()) {
        int ret = (pUserApi->*pTraderApi)(pField, id);
        traderApiMutex.unlock();
        if (ret == 0) {
            free(pField);
            return id;
        }
    }

    QtConcurrent::run([=]() -> void {
        int count_down = 100;
        while (count_down-- > 0) {
            _sleep(400 - count_down * 2);   // TODO 改进退避算法
            traderApiMutex.lock();
            int ret = (pUserApi->*pTraderApi)(pField, id);
            traderApiMutex.unlock();
            if (ret == 0) {
                break;
            }
        }
        free(pField);
    });

    return id;
}

int CtpExecuter::authenticate()
{
    CThostFtdcReqAuthenticateField reqAuthenticate;
    memset(&reqAuthenticate, 0, sizeof (CThostFtdcReqAuthenticateField));
    strcpy(reqAuthenticate.BrokerID, c_brokerID);
    strcpy(reqAuthenticate.UserID, c_userID);
    strcpy(reqAuthenticate.UserProductInfo, c_userProductInfo);
    strcpy(reqAuthenticate.AuthCode, c_authenticateCode);

    int id = nRequestID.fetchAndAddRelaxed(1);
    traderApiMutex.lock();
    int ret = pUserApi->ReqAuthenticate(&reqAuthenticate, id);
    traderApiMutex.unlock();
    Q_UNUSED(ret);
    return id;
}

/*!
 * \brief CtpExecuter::login
 * 用配置文件中的账号信息登陆交易端
 *
 * \return nRequestID
 */
int CtpExecuter::login()
{
    CThostFtdcReqUserLoginField reqUserLogin;
    memset(&reqUserLogin, 0, sizeof (CThostFtdcReqUserLoginField));
    strcpy(reqUserLogin.BrokerID, c_brokerID);
    strcpy(reqUserLogin.UserID, c_userID);
    strcpy(reqUserLogin.Password, c_password);
    strcpy(reqUserLogin.UserProductInfo, c_userProductInfo);

    int id = nRequestID.fetchAndAddRelaxed(1);
    traderApiMutex.lock();
    int ret = pUserApi->ReqUserLogin(&reqUserLogin, id);
    traderApiMutex.unlock();
    Q_UNUSED(ret);
    return id;
}

/*!
 * \brief CtpExecuter::qrySettlementInfo
 * 发送投资者结算结果查询请求
 *
 * \return nRequestID
 */
int CtpExecuter::qrySettlementInfo()
{
    auto *pField = (CThostFtdcQrySettlementInfoField*) malloc(sizeof(CThostFtdcQrySettlementInfoField));
    memset(pField, 0, sizeof (CThostFtdcQrySettlementInfoField));
    strcpy(pField->BrokerID, c_brokerID);
    strcpy(pField->InvestorID, c_userID);

    return callTraderApi(&CThostFtdcTraderApi::ReqQrySettlementInfo, pField);
}

/*!
 * \brief CtpExecuter::settlementInfoConfirm
 * 发送投资者结算结果确认请求
 *
 * \return nRequestID
 */
int CtpExecuter::settlementInfoConfirm()
{
    CThostFtdcSettlementInfoConfirmField confirmField;
    memset(&confirmField, 0, sizeof (CThostFtdcSettlementInfoConfirmField));
    strcpy(confirmField.BrokerID, c_brokerID);
    strcpy(confirmField.InvestorID, c_userID);

    int id = nRequestID.fetchAndAddRelaxed(1);
    traderApiMutex.lock();
    int ret = pUserApi->ReqSettlementInfoConfirm(&confirmField, id);
    traderApiMutex.unlock();
    Q_UNUSED(ret);
    return id;
}

/*!
 * \brief CtpExecuter::qrySettlementInfoConfirm
 * 发送投资者结算结果确认查询请求
 *
 * \return nRequestID
 */
int CtpExecuter::qrySettlementInfoConfirm()
{
    auto *pField = (CThostFtdcQrySettlementInfoConfirmField*) malloc(sizeof(CThostFtdcQrySettlementInfoConfirmField));
    memset(pField, 0, sizeof (CThostFtdcQrySettlementInfoConfirmField));
    strcpy(pField->BrokerID, c_brokerID);
    strcpy(pField->InvestorID, c_userID);

    return callTraderApi(&CThostFtdcTraderApi::ReqQrySettlementInfoConfirm, pField);
}

/*!
 * \brief CtpExecuter::qryTradingAccount
 * 发送查询资金账户请求
 *
 * \return nRequestID
 */
int CtpExecuter::qryTradingAccount()
{
    auto *pField = (CThostFtdcQryTradingAccountField*) malloc(sizeof(CThostFtdcQryTradingAccountField));
    memset(pField, 0, sizeof (CThostFtdcQryTradingAccountField));
    strcpy(pField->BrokerID, c_brokerID);
    strcpy(pField->InvestorID, c_userID);

    return callTraderApi(&CThostFtdcTraderApi::ReqQryTradingAccount, pField);
}

/*!
 * \brief CtpExecuter::qryInstrumenMarginRate
 * 发送查询保证金率请求(投机)
 *
 * \param instrument 合约代码(为空代表所有持仓合约)
 * \return nRequestID
 */
int CtpExecuter::qryInstrumentMarginRate(const QString &instrument)
{
    auto * pField = (CThostFtdcQryInstrumentMarginRateField*) malloc(sizeof(CThostFtdcQryInstrumentMarginRateField));
    memset(pField, 0, sizeof (CThostFtdcQryInstrumentMarginRateField));
    strcpy(pField->BrokerID, c_brokerID);
    strcpy(pField->InvestorID, c_userID);
    strcpy(pField->InstrumentID, instrument.toLatin1().data());
    pField->HedgeFlag = THOST_FTDC_HF_Speculation;

    return callTraderApi(&CThostFtdcTraderApi::ReqQryInstrumentMarginRate, pField);
}

/*!
 * \brief CtpExecuter::qryInstrumentCommissionRate
 * 发送查询手续费率请求
 *
 * \param instrument 合约代码(为空代表所有持仓合约)
 * \return nRequestID
 */
int CtpExecuter::qryInstrumentCommissionRate(const QString &instrument)
{
    auto * pField = (CThostFtdcQryInstrumentCommissionRateField*) malloc(sizeof(CThostFtdcQryInstrumentCommissionRateField));
    memset(pField, 0, sizeof (CThostFtdcQryInstrumentCommissionRateField));
    strcpy(pField->BrokerID, c_brokerID);
    strcpy(pField->InvestorID, c_userID);
    strcpy(pField->InstrumentID, instrument.toLatin1().data());

    return callTraderApi(&CThostFtdcTraderApi::ReqQryInstrumentCommissionRate, pField);
}

/*!
 * \brief CtpExecuter::qryInstrument
 * 发送查询合约请求
 *
 * \param instrument 合约代码(为空代表所有合约)
 * \param exchangeID 交易所代码(为空代表所有交易所)
 * \return nRequestID
 */
int CtpExecuter::qryInstrument(const QString &instrument, const QString &exchangeID)
{
    auto * pField = (CThostFtdcQryInstrumentField*) malloc(sizeof(CThostFtdcQryInstrumentField));
    memset(pField, 0, sizeof(CThostFtdcQryInstrumentField));
    strcpy(pField->InstrumentID, instrument.toLatin1().data());
    strcpy(pField->ExchangeID, exchangeID.toLatin1().data());

    return callTraderApi(&CThostFtdcTraderApi::ReqQryInstrument, pField);
}

/*!
 * \brief CtpExecuter::qryDepthMarketData
 * 发送查询合约行情请求
 *
 * \param instrument 合约代码
 * \return nRequestID
 */
int CtpExecuter::qryDepthMarketData(const QString &instrument)
{
    auto *pField = (CThostFtdcQryDepthMarketDataField*) malloc(sizeof(CThostFtdcQryDepthMarketDataField));
    strcpy(pField->InstrumentID, instrument.toLatin1().data());

    return callTraderApi(&CThostFtdcTraderApi::ReqQryDepthMarketData, pField);
}

/*!
 * \brief CtpExecuter::insertLimitOrder
 * 下限价单 (包括FOK, FAK)
 *
 * \param instrument 合约代码
 * \param openClose 开平标志(通用宏定义)
 * \param volume 手数(非零整数, 正数代表开多/平空, 负数代表开空/平多)
 * \param price 价格(限价, 不得超出涨跌停范围)
 * \param allOrAny 全部成交或撤单(true)/任意数量成交剩余撤单(false)
 * \param gfdOrIoc 今日有效(true)/立即成交否则撤单(false)
 * \return nRequestID
 */
int CtpExecuter::insertLimitOrder(const QString &instrument, int openClose, int volume, double price, bool allOrAny, bool gfdOrIoc)
{
    Q_ASSERT(volume != 0);

    CThostFtdcInputOrderField inputOrder;
    memset(&inputOrder, 0, sizeof (CThostFtdcInputOrderField));
    strcpy(inputOrder.BrokerID, c_brokerID);
    strcpy(inputOrder.InvestorID, c_userID);
    strcpy(inputOrder.InstrumentID, instrument.toLatin1().data());
    strcpy(inputOrder.OrderRef, "");
//	sprintf(inputOrder.OrderRef, "%12d", orderRef);
//	orderRef++;

    inputOrder.Direction = volume > 0 ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell;
    inputOrder.CombOffsetFlag[0] = ctpOffsetFlags[openClose];
    inputOrder.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
    inputOrder.VolumeTotalOriginal = qAbs(volume);
    inputOrder.VolumeCondition = allOrAny ? THOST_FTDC_VC_CV : THOST_FTDC_VC_AV;
    inputOrder.MinVolume = 1;
    inputOrder.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    inputOrder.IsAutoSuspend = 0;
    inputOrder.UserForceClose = 0;
    inputOrder.ContingentCondition = THOST_FTDC_CC_Immediately;
    inputOrder.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    inputOrder.LimitPrice = price;
    inputOrder.TimeCondition = gfdOrIoc ? THOST_FTDC_TC_GFD : THOST_FTDC_TC_IOC;

    int id = nRequestID.fetchAndAddRelaxed(1);
    traderApiMutex.lock();
    int ret = pUserApi->ReqOrderInsert(&inputOrder, id);
    traderApiMutex.unlock();
    Q_UNUSED(ret);
    return id;
}

/*!
 * \brief CtpExecuter::insertMarketOrder
 * 下市价单
 *
 * \param instrument 合约代码
 * \param openClose 开平标志(通用宏定义)
 * \param volume 手数(非零整数, 正数代表开多/平空, 负数代表开空/平多)
 * \return nRequestID
 */
int CtpExecuter::insertMarketOrder(const QString &instrument, int openClose, int volume)
{
    Q_ASSERT(volume != 0);

    CThostFtdcInputOrderField inputOrder;
    memset(&inputOrder, 0, sizeof (CThostFtdcInputOrderField));
    strcpy(inputOrder.BrokerID, c_brokerID);
    strcpy(inputOrder.InvestorID, c_userID);
    strcpy(inputOrder.InstrumentID, instrument.toLatin1().data());
    strcpy(inputOrder.OrderRef, "");
//	sprintf(inputOrder.OrderRef, "%12d", orderRef);
//	orderRef++;

    inputOrder.Direction = volume > 0 ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell;
    inputOrder.CombOffsetFlag[0] = ctpOffsetFlags[openClose];
    inputOrder.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
    inputOrder.VolumeTotalOriginal = qAbs(volume);
    inputOrder.VolumeCondition = THOST_FTDC_VC_AV;
    inputOrder.MinVolume = 1;
    inputOrder.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    inputOrder.IsAutoSuspend = 0;
    inputOrder.UserForceClose = 0;
    inputOrder.ContingentCondition = THOST_FTDC_CC_Immediately;
    inputOrder.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
    inputOrder.LimitPrice = 0;
    inputOrder.TimeCondition = THOST_FTDC_TC_IOC;

    int id = nRequestID.fetchAndAddRelaxed(1);
    traderApiMutex.lock();
    int ret = pUserApi->ReqOrderInsert(&inputOrder, id);
    traderApiMutex.unlock();
    Q_UNUSED(ret);
    return id;
}

/*!
 * \brief CtpExecuter::orderAction
 * 报单操作(仅支持撤单)
 *
 * \param orderRef 报单引用(TThostFtdcOrderRefType)
 * \param frontID 前置编号
 * \param sessionID 会话编号
 * \param instrument 合约代码
 * \return nRequestID
 */
int CtpExecuter::orderAction(char* orderRef, int frontID, int sessionID, const QString &instrument)
{
    CThostFtdcInputOrderActionField orderAction;
    memset(&orderAction, 0, sizeof(CThostFtdcInputOrderActionField));
    strcpy(orderAction.BrokerID, c_brokerID);
    strcpy(orderAction.InvestorID, c_userID);
    strcpy(orderAction.InstrumentID, instrument.toLatin1().data());
    memcpy(orderAction.OrderRef, orderRef, sizeof(TThostFtdcOrderRefType));
    orderAction.FrontID = frontID;
    orderAction.SessionID = sessionID;
    orderAction.ActionFlag = THOST_FTDC_AF_Delete;

    int id = nRequestID.fetchAndAddRelaxed(1);
    traderApiMutex.lock();
    int ret = pUserApi->ReqOrderAction(&orderAction, id);
    traderApiMutex.unlock();
    Q_UNUSED(ret);
    return id;
}

/*!
 * \brief CtpExecuter::insertParkedLimitOrder
 * 预埋限价单 (包括FOK, FAK)
 *
 * \param instrument 合约代码
 * \param openClose 开平标志(通用宏定义)
 * \param volume 手数(非零整数, 正数代表开多/平空, 负数代表开空/平多)
 * \param price 价格(限价, 不得超出涨跌停范围)
 * \param allOrAny 全部成交或撤单(true)/任意数量成交剩余撤单(false)
 * \param gfdOrIoc 今日有效(true)/立即成交否则撤单(false)
 * \return nRequestID
 */
int CtpExecuter::insertParkedLimitOrder(const QString &instrument, int openClose, int volume, double price, bool allOrAny, bool gfdOrIoc)
{
    Q_ASSERT(volume != 0);

    CThostFtdcParkedOrderField parkedOrder;
    memset(&parkedOrder, 0, sizeof (CThostFtdcInputOrderField));
    strcpy(parkedOrder.BrokerID, c_brokerID);
    strcpy(parkedOrder.InvestorID, c_userID);
    strcpy(parkedOrder.InstrumentID, instrument.toLatin1().data());
    strcpy(parkedOrder.OrderRef, "");
//	sprintf(inputOrder.OrderRef, "%12d", orderRef);
//	orderRef++;

    parkedOrder.Direction = volume > 0 ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell;
    parkedOrder.CombOffsetFlag[0] = ctpOffsetFlags[openClose];
    parkedOrder.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
    parkedOrder.VolumeTotalOriginal = qAbs(volume);
    parkedOrder.VolumeCondition = allOrAny ? THOST_FTDC_VC_CV : THOST_FTDC_VC_AV;
    parkedOrder.MinVolume = 1;
    parkedOrder.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    parkedOrder.IsAutoSuspend = 0;
    parkedOrder.UserForceClose = 0;
    parkedOrder.ContingentCondition = THOST_FTDC_CC_Immediately;
    parkedOrder.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    parkedOrder.LimitPrice = price;
    parkedOrder.TimeCondition = gfdOrIoc ? THOST_FTDC_TC_GFD : THOST_FTDC_TC_IOC;

    int id = nRequestID.fetchAndAddRelaxed(1);
    traderApiMutex.lock();
    int ret = pUserApi->ReqParkedOrderInsert(&parkedOrder, id);
    traderApiMutex.unlock();
    Q_UNUSED(ret);
    return id;
}

/*!
 * \brief CtpExecuter::qryMaxOrderVolume
 * 发送查询最大报单数量请求
 *
 * \param instrument 合约代码
 * \param direction 多空方向 (true: 多, false: 空)
 * \param openClose 开平标志(通用宏定义)
 * \return nRequestID
 */
int CtpExecuter::qryMaxOrderVolume(const QString &instrument, bool direction, int openClose)
{
    auto *pField = (CThostFtdcQueryMaxOrderVolumeField *) malloc(sizeof(CThostFtdcQueryMaxOrderVolumeField));
    memset(pField, 0, sizeof(CThostFtdcQueryMaxOrderVolumeField));
    strcpy(pField->BrokerID, c_brokerID);
    strcpy(pField->InvestorID, c_userID);
    strcpy(pField->InstrumentID, instrument.toLatin1().data());
    pField->Direction = direction ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell;
    pField->OffsetFlag = ctpOffsetFlags[openClose];
    pField->HedgeFlag = THOST_FTDC_HF_Speculation;

    return callTraderApi(&CThostFtdcTraderApi::ReqQueryMaxOrderVolume, pField);
}

/*!
 * \brief CtpExecuter::qryOrder
 * 发送查询报单请求
 *
 * \param instrument 合约代码
 * \return nRequestID
 */
int CtpExecuter::qryOrder(const QString &instrument)
{
    auto *pField = (CThostFtdcQryOrderField *) malloc(sizeof(CThostFtdcQryOrderField));
    memset(pField, 0, sizeof(CThostFtdcQryOrderField));
    strcpy(pField->BrokerID, c_brokerID);
    strcpy(pField->InvestorID, c_userID);
    strcpy(pField->InstrumentID, instrument.toLatin1().data());

    return callTraderApi(&CThostFtdcTraderApi::ReqQryOrder, pField);
}

/*!
 * \brief CtpExecuter::qryTrade
 * 发送查询成交请求
 *
 * \param instrument 合约代码
 * \return nRequestID
 */
int CtpExecuter::qryTrade(const QString &instrument)
{
    auto *pField = (CThostFtdcQryTradeField *) malloc(sizeof(CThostFtdcQryTradeField));
    memset(pField, 0, sizeof(CThostFtdcQryTradeField));
    strcpy(pField->BrokerID, c_brokerID);
    strcpy(pField->InvestorID, c_userID);
    strcpy(pField->InstrumentID, instrument.toLatin1().data());

    return callTraderApi(&CThostFtdcTraderApi::ReqQryTrade, pField);
}

/*!
 * \brief CtpExecuter::qryPosition
 * 发送查询持仓请求
 *
 * \param instrument 合约代码
 * \return nRequestID
 */
int CtpExecuter::qryPosition(const QString &instrument)
{
    auto *pField = (CThostFtdcQryInvestorPositionField*) malloc(sizeof(CThostFtdcQryInvestorPositionField));
    memset(pField, 0, sizeof(CThostFtdcQryInvestorPositionField));
    strcpy(pField->BrokerID, c_brokerID);
    strcpy(pField->InvestorID, c_userID);
    strcpy(pField->InstrumentID, instrument.toLatin1().data());

    return callTraderApi(&CThostFtdcTraderApi::ReqQryInvestorPosition, pField);
}

/*!
 * \brief CtpExecuter::qryPositionDetail
 * 发送查询持仓明细请求
 *
 * \param instrument 合约代码
 * \return nRequestID
 */
int CtpExecuter::qryPositionDetail(const QString &instrument)
{
    auto *pField = (CThostFtdcQryInvestorPositionDetailField*) malloc(sizeof(CThostFtdcQryInvestorPositionDetailField));
    memset(pField, 0, sizeof(CThostFtdcQryInvestorPositionDetailField));
    strcpy(pField->BrokerID, c_brokerID);
    strcpy(pField->InvestorID, c_userID);
    strcpy(pField->InstrumentID, instrument.toLatin1().data());

    return callTraderApi(&CThostFtdcTraderApi::ReqQryInvestorPositionDetail, pField);
}

/*!
 * \brief CtpExecuter::insertExecOrder
 * 发送期权行权指令
 *
 * \param instrument 期权合约代码
 * \param type 期权类型(看涨/看跌)
 * \param volume 行权手数
 * \return nRequestID
 */
int CtpExecuter::insertExecOrder(const QString &instrument, OPTION_TYPE type, int volume)
{
    CThostFtdcInputExecOrderField exc;
    memset(&exc, 0, sizeof(CThostFtdcInputExecOrderField));
    strcpy(exc.BrokerID, c_brokerID);
    strcpy(exc.InvestorID, c_userID);
    strcpy(exc.InstrumentID, instrument.toLatin1().data());
    //memcpy(exc.ExecOrderRef, )
    exc.OffsetFlag = THOST_FTDC_OF_Close;
    exc.HedgeFlag = THOST_FTDC_HF_Speculation;
    exc.ActionType = THOST_FTDC_ACTP_Exec;
    if (type == CALL_OPT) {
        exc.PosiDirection = THOST_FTDC_PD_Long;
    } else {
        exc.PosiDirection = THOST_FTDC_PD_Short;
    }

    exc.ReservePositionFlag = THOST_FTDC_EOPF_Reserve;
    exc.CloseFlag = THOST_FTDC_EOCF_NotToClose;

    int id = nRequestID.fetchAndAddRelaxed(1);
    traderApiMutex.lock();
    int ret = pUserApi->ReqExecOrderInsert(&exc, id);
    traderApiMutex.unlock();
    Q_UNUSED(ret);
    return id;
}

/*!
 * \brief CtpExecuter::insertQuote
 * 发送询价指令
 *
 * \param instrument 合约代码
 * \return nRequestID
 */
int CtpExecuter::insertQuote(const QString &instrument)
{
    CThostFtdcInputForQuoteField quote;
    memset(&quote, 0, sizeof(CThostFtdcInputForQuoteField));
    strcpy(quote.BrokerID, c_brokerID);
    strcpy(quote.InvestorID, c_userID);
    strcpy(quote.InstrumentID, instrument.toLatin1().data());
    //memcpy(quote.ForQuoteRef, )

    int id = nRequestID.fetchAndAddRelaxed(1);
    traderApiMutex.lock();
    int ret = pUserApi->ReqForQuoteInsert(&quote, id);
    traderApiMutex.unlock();
    Q_UNUSED(ret);
    return id;
}

/*!
 * \brief CtpExecuter::checkLimitOrder
 * 1. 检查限价单价格是否合理, 即是否可能造成自成交
 * 2. 检查该合约撤单次数是否已超标
 *
 * \param instrument 合约代码
 * \param price 限价单价格
 * \param direction 多空方向(true: 多, false: 空)
 * \param orderType 订单类型 (0:普通限价单, 1:FAK, 2:FOK)
 * \return true: 检查通过, false: 未通过检查
 */
bool CtpExecuter::checkLimitOrder(const QString& instrument, double price, bool direction, int orderType)
{
    if (orderType == 0) {
        if (preventSelfTrade) {
            const auto orderList = orderMap.values(instrument);

            double maxLongPrice = -DBL_MAX;
            double minShortPrice = DBL_MAX;
            for (const auto item : orderList) {
                if (item.status == OrderStatus::PENDING || item.status == OrderStatus::UNKNOWN) {
                    if (item.direction) {
                        if (item.price > maxLongPrice) {
                            maxLongPrice = item.price;
                        }
                    } else {
                        if (item.price < minShortPrice) {
                            minShortPrice = item.price;
                        }
                    }
                }
            }

            const double tolerance = 0.0000001;

            if (direction) {
                if (price > (minShortPrice - tolerance)) {
                    qWarning() << DATE_TIME << "This limit order may cause self trade, which is not allowed!";
                    return false;
                }
            } else {
                if (price < (maxLongPrice + tolerance)) {
                    qWarning() << DATE_TIME << "This limit order may cause self trade, which is not allowed!";
                    return false;
                }
            }
        }

        if (orderCancelCountMap[instrument] >= orderCancelLimit) {
            qWarning() << DATE_TIME << "The cancel order counter exceeds limit on this instrument!";
            return false;
        }
    }
    return true;
}

/*!
 * \brief CtpExecuter::distinguishYdTd
 * 判断平仓时是否需要使用"平昨"或"平今"标志
 *
 * \param instrument 合约代码
 * \return true表示使用"平昨"或"平今"标志, false表示使用"平仓"标志(即不区分昨仓和今仓)
 */
bool CtpExecuter::distinguishYdTd(const QString &instrument)
{
    if (instrumentDataCache.contains(instrument)) {
        // 上期所平仓时需要使用"平昨"或"平今"标志
        if (strcmp(instrumentDataCache.value(instrument).ExchangeID, "SHFE") == 0) {
            return true;
        }
    }
    return false;
}

/*!
 * \brief CtpExecuter::canUseAnyPrice
 * 判断该合约是否支持市价单
 *
 * \param instrument 合约代码
 * \return true表示支持市价单, false表示不支持市价单
 */
bool CtpExecuter::canUseAnyPrice(const QString &instrument)
{
    if (instrumentDataCache.contains(instrument)) {
        // 上期所不支持市价单
        if (strcmp(instrumentDataCache.value(instrument).ExchangeID, "SHFE") == 0) {
            return false;
        }
        // 大商所期权合约不支持市价单
        if (strcmp(instrumentDataCache.value(instrument).ExchangeID, "DCE") == 0 && isOption(instrument)) {
            return false;
        }
        // TODO 中金所后两个季月合约不支持市价单
    }
    return true;
}

/*!
 * \brief CtpExecuter::getStatus
 * 获取状态字符串
 *
 * \return 状态
 */
QString CtpExecuter::getStatus() const
{
    if (loggedIn && cacheReady) {
        return "Ready";
    } else {
        return "NotReady";
    }
}

/*!
 * \brief CtpExecuter::getTradingDay
 * 获取交易日
 *
 * \return 交易日(YYYYMMDD)
 */
QString CtpExecuter::getTradingDay() const
{
    if (loggedIn) {
        return pUserApi->GetTradingDay();
    } else {
        return INVALID_DATE_STRING;
    }
}

void CtpExecuter::confirmSettlementInfo()
{
    if (loggedIn) {
        settlementInfoConfirm();
    } else {
        qWarning() << DATE_TIME << "ConfirmSettleInfo failed! Not logged in!";
    }
}

void CtpExecuter::updateAccountInfo()
{
    if (loggedIn) {
        qryTradingAccount();
    } else {
        qWarning() << DATE_TIME << "UpdateAccountInfo failed! Not logged in!";
    }
}

/*!
 * \brief CtpExecuter::updateInstrumentDataCache
 * 请求查询合约基本信息和当前市价信息
 * 返回结果将被存入缓存, 供盘中快速查询
 *
 * \param instruments 合约代码列表
 */
void CtpExecuter::updateInstrumentDataCache()
{
    if (loggedIn) {
        upperLowerLimitCache.clear();
        instrumentDataCache.clear();
        qryInstrument();
        qryDepthMarketData();
        cacheReady = true;  //FIXME
    } else {
        qWarning() << DATE_TIME << "UpdateInstrumentDataCache failed! Not logged in!";
    }
}

/*!
 * \brief CtpExecuter::getCachedInstruments
 * 返回缓存中所有以idPrefix开头的合约代码列表
 *
 * \param idPrefix 合约代码或其部分前缀 (默认值为空, 表示所有合约)
 */
QStringList CtpExecuter::getCachedInstruments(const QString &idPrefix) const
{
    const auto instruments = instrumentDataCache.keys();
    QStringList ret;
    for (const auto &instrument : instruments) {
        if (instrument.startsWith(idPrefix)) {
            ret.append(instrument);
        }
    }
    return ret;
}

/*!
 * \brief CtpExecuter::getExchangeID
 * 从缓存中查询合约的交易所代码并返回
 *
 * \param instrument 合约代码
 * \return 交易所代码
 */
QString CtpExecuter::getExchangeID(const QString &instrument)
{
    if (instrumentDataCache.contains(instrument)) {
        return instrumentDataCache.value(instrument).ExchangeID;
    } else {
        return "";
    }
}

/*!
 * \brief CtpExecuter::getExpireDate
 * 从缓存中查询合约的到期日并返回
 *
 * \param instrument 合约代码
 * \return 合约到期日
 */
QString CtpExecuter::getExpireDate(const QString &instrument)
{
    if (instrumentDataCache.contains(instrument)) {
        return instrumentDataCache.value(instrument).ExpireDate;
    } else {
        return INVALID_DATE_STRING;
    }
}

/*!
 * \brief CtpExecuter::getUpperLimit
 * 从缓存中查询合约的涨停价并返回
 *
 * \param instrument 合约代码
 * \return 涨停价 (如果缓存中查不到返回-DBL_MAX)
 */
double CtpExecuter::getUpperLimit(const QString &instrument)
{
    if (upperLowerLimitCache.contains(instrument)) {
        return upperLowerLimitCache.value(instrument).first;
    } else {
        return -DBL_MAX;
    }
}

/*!
 * \brief CtpExecuter::getLowerLimit
 * 从缓存中查询合约的跌停价并返回
 *
 * \param instrument 合约代码
 * \return 跌停价 (如果缓存中查不到返回DBL_MAX)
 */
double CtpExecuter::getLowerLimit(const QString &instrument)
{
    if (upperLowerLimitCache.contains(instrument)) {
        return upperLowerLimitCache.value(instrument).second;
    } else {
        return DBL_MAX;
    }
}

void CtpExecuter::updateOrderMap(const QString &instrument)
{
    if (!loggedIn) {
        return;
    }

    if (instrument == "") {
        orderMap.clear();
    } else {
        const auto keys = orderMap.uniqueKeys();
        for (const auto key : keys) {
            orderMap.remove(key);
        }
    }

    qryOrder(instrument);
}

int CtpExecuter::qryParkedOrder(const QString &instrument, const QString &exchangeID)
{
    auto *pField = (CThostFtdcQryParkedOrderField*) malloc(sizeof (CThostFtdcQryParkedOrderField));
    memset(pField, 0, sizeof(CThostFtdcQryParkedOrderField));
    strcpy(pField->BrokerID, c_brokerID);
    strcpy(pField->InvestorID, c_userID);
    strcpy(pField->InstrumentID, instrument.toLatin1().data());
    strcpy(pField->ExchangeID, exchangeID.toLatin1().data());

    return callTraderApi(&CThostFtdcTraderApi::ReqQryParkedOrder, pField);
}

int CtpExecuter::qryParkedOrderAction(const QString &instrument, const QString &exchangeID)
{
    auto *pField = (CThostFtdcQryParkedOrderActionField*) malloc(sizeof (CThostFtdcQryParkedOrderActionField));
    memset(pField, 0, sizeof(CThostFtdcQryParkedOrderActionField));
    strcpy(pField->BrokerID, c_brokerID);
    strcpy(pField->InvestorID, c_userID);
    strcpy(pField->InstrumentID, instrument.toLatin1().data());
    strcpy(pField->ExchangeID, exchangeID.toLatin1().data());

    return callTraderApi(&CThostFtdcTraderApi::ReqQryParkedOrderAction, pField);
}

/*!
 * \brief analyzeOrderType
 * 分析报单类型
 * 普通限价单: allOrAny = false, gfdOrIoc = true
 * FAK:      allOrAny = false, gfdOrIoc = false
 * FOK:      allOrAny = true,  gfdOrIoc = false
 *
 * \param orderType 订单类型 (0:普通限价单, 1:FAK, 2:FOK)
 * \param allOrAny 全部成交或撤单(true)/任意数量成交剩余撤单(false)
 * \param gfdOrIoc 今日有效(true)/立即成交否则撤单(false)
 */
static inline void analyzeOrderType(int orderType, bool &allOrAny, bool &gfdOrIoc)
{
    allOrAny = (orderType == 2);
    gfdOrIoc = (orderType == 0);
}

/*!
 * \brief CtpExecuter::buyLimitAuto
 * 限价买进合约 (开多或平空, 如有空头持仓先平仓)
 *
 * \param instrument 合约代码
 * \param volume 买进数量 (大于零)
 * \param price 买进价格 (必须在涨跌停范围内)
 * \param orderType 订单类型 (0:普通限价单, 1:FAK, 2:FOK)
 */
void CtpExecuter::buyLimitAuto(const QString& instrument, int volume, double price, int orderType)
{
    qDebug() << DATE_TIME << "buyLimitAuto" << instrument << ": volume =" << volume << ", price =" << price << ", orderType =" << orderType;

    if (!checkLimitOrder(instrument, price, true, orderType)) {
        return;
    }

    bool allOrAny, gfdOrIoc;
    analyzeOrderType(orderType, allOrAny, gfdOrIoc);

    int remainVol = volume;
    if (distinguishYdTd(instrument)) {
        int closeTd = qMin(tdShortPositions[instrument], remainVol);
        if (closeTd > 0) {
            insertLimitOrder(instrument, CLOSE_TODAY, closeTd, price, allOrAny, gfdOrIoc);
            remainVol -= closeTd;
        }

        int closeYd = qMin(ydShortPositions[instrument], remainVol);
        if (closeYd > 0) {
            insertLimitOrder(instrument, CLOSE_YESTERDAY, closeYd, price, allOrAny, gfdOrIoc);
            remainVol -= closeYd;
        }
    } else {
        int closeVol = qMin(tdShortPositions[instrument] + ydShortPositions[instrument], remainVol);
        if (closeVol > 0) {
            insertLimitOrder(instrument, CLOSE, closeVol, price, allOrAny, gfdOrIoc);
            remainVol -= closeVol;
        }
    }

    if (remainVol > 0) {
        insertLimitOrder(instrument, OPEN, remainVol, price, allOrAny, gfdOrIoc);
    }
}

/*!
 * \brief CtpExecuter::sellLimitAuto
 * 限价卖出合约 (开空或平多, 如有多头持仓先平仓)
 *
 * \param instrument 合约代码
 * \param volume 卖出数量 (大于零)
 * \param price 卖出价格 (必须在涨跌停范围内)
 * \param orderType 订单类型 (0:普通限价单, 1:FAK, 2:FOK)
 */
void CtpExecuter::sellLimitAuto(const QString& instrument, int volume, double price, int orderType)
{
    qDebug() << DATE_TIME << "sellLimitAuto" << instrument << ": volume =" << volume << ", price =" << price << ", orderType =" << orderType;

    if (!checkLimitOrder(instrument, price, false, orderType)) {
        return;
    }

    bool allOrAny, gfdOrIoc;
    analyzeOrderType(orderType, allOrAny, gfdOrIoc);

    int remainVol = volume;
    if (distinguishYdTd(instrument)) {
        int closeTd = qMin(tdLongPositions[instrument], remainVol);
        if (closeTd > 0) {
            insertLimitOrder(instrument, CLOSE_TODAY, - closeTd, price, allOrAny, gfdOrIoc);
            remainVol -= closeTd;
        }

        int closeYd = qMin(ydLongPositions[instrument], remainVol);
        if (closeYd > 0) {
            insertLimitOrder(instrument, CLOSE_YESTERDAY, - closeYd, price, allOrAny, gfdOrIoc);
            remainVol -= closeYd;
        }
    } else {
        int closeVol = qMin(tdLongPositions[instrument] + ydLongPositions[instrument], remainVol);
        if (closeVol > 0) {
            insertLimitOrder(instrument, CLOSE, - closeVol, price, allOrAny, gfdOrIoc);
            remainVol -= closeVol;
        }
    }

    if (remainVol > 0) {
        insertLimitOrder(instrument, OPEN, - remainVol, price, allOrAny, gfdOrIoc);
    }
}

/*!
 * \brief CtpExecuter::buyLimit
 * 限价买进合约 (开多)
 *
 * \param instrument 合约代码
 * \param volume 买进数量 (大于零)
 * \param price 买进价格 (必须在涨跌停范围内)
 * \param orderType 订单类型 (0:普通限价单, 1:FAK, 2:FOK)
 */
void CtpExecuter::buyLimit(const QString& instrument, int volume, double price, int orderType)
{
    qDebug() << DATE_TIME << "buyLimit" << instrument << ": volume =" << volume << ", price =" << price << ", orderType =" << orderType;

    if (!checkLimitOrder(instrument, price, true, orderType)) {
        return;
    }

    bool allOrAny, gfdOrIoc;
    analyzeOrderType(orderType, allOrAny, gfdOrIoc);

    insertLimitOrder(instrument, OPEN, volume, price, allOrAny, gfdOrIoc);
}

/*!
 * \brief CtpExecuter::sellLimit
 * 限价卖出合约 (开空)
 *
 * \param instrument 合约代码
 * \param volume 卖出数量 (大于零)
 * \param price 卖出价格 (必须在涨跌停范围内)
 * \param orderType 订单类型 (0:普通限价单, 1:FAK, 2:FOK)
 */
void CtpExecuter::sellLimit(const QString& instrument, int volume, double price, int orderType)
{
    qDebug() << DATE_TIME << "sellLimit" << instrument << ": volume =" << volume << ", price =" << price << ", orderType =" << orderType;

    if (!checkLimitOrder(instrument, price, false, orderType)) {
        return;
    }

    bool allOrAny, gfdOrIoc;
    analyzeOrderType(orderType, allOrAny, gfdOrIoc);

    insertLimitOrder(instrument, OPEN, - volume, price, allOrAny, gfdOrIoc);
}

/*!
 * \brief CtpExecuter::buyMarketAuto
 * 市价买进合约 (开多或平空, 如有空头持仓先平仓)
 *
 * \param instrument 合约代码
 * \param volume 买进数量 (大于零)
 * \param useSimulation 使用涨停板限价单模拟市价单
 */
void CtpExecuter::buyMarketAuto(const QString &instrument, int volume, bool useSimulation)
{
    qDebug() << DATE_TIME << "buyMarketAuto" << instrument << ": volume =" << volume;

    if (!canUseAnyPrice(instrument) || useSimulation) {
        // Use FAK instead of market order
        buyLimitAuto(instrument, volume, upperLowerLimitCache.value(instrument).first, 1);
    } else {
        int remainVol = volume;
        if (distinguishYdTd(instrument)) {
            int closeTd = qMin(tdShortPositions[instrument], remainVol);
            if (closeTd > 0) {
                insertMarketOrder(instrument, CLOSE_TODAY, closeTd);
                remainVol -= closeTd;
            }

            int closeYd = qMin(ydShortPositions[instrument], remainVol);
            if (closeYd > 0) {
                insertMarketOrder(instrument, CLOSE_YESTERDAY, closeYd);
                remainVol -= closeYd;
            }
        } else {
            int closeVol = qMin(tdShortPositions[instrument] + ydShortPositions[instrument], remainVol);
            if (closeVol > 0) {
                insertMarketOrder(instrument, CLOSE, closeVol);
                remainVol -= closeVol;
            }
        }

        if (remainVol > 0) {
            insertMarketOrder(instrument, OPEN, remainVol);
        }
    }
}

/*!
 * \brief CtpExecuter::sellMarketAuto
 * 市价卖出合约 (开空或平多, 如有多头持仓先平仓)
 *
 * \param instrument 合约代码
 * \param volume 卖出数量 (大于零)
 * \param useSimulation 使用跌停板限价单模拟市价单
 */
void CtpExecuter::sellMarketAuto(const QString &instrument, int volume, bool useSimulation)
{
    qDebug() << DATE_TIME << "sellMarketAuto" << instrument << ": volume =" << volume;

    if (!canUseAnyPrice(instrument) || useSimulation) {
        // Use FAK instead of market order
        sellLimitAuto(instrument, volume, upperLowerLimitCache.value(instrument).second, 1);
    } else {
        int remainVol = volume;
        if (distinguishYdTd(instrument)) {
            int closeTd = qMin(tdLongPositions[instrument], remainVol);
            if (closeTd > 0) {
                insertMarketOrder(instrument, CLOSE_TODAY, - closeTd);
                remainVol -= closeTd;
            }

            int closeYd = qMin(ydLongPositions[instrument], remainVol);
            if (closeYd > 0) {
                insertMarketOrder(instrument, CLOSE_YESTERDAY, - closeYd);
                remainVol -= closeYd;
            }
        } else {
            int closeVol = qMin(tdLongPositions[instrument] + ydLongPositions[instrument], remainVol);
            if (closeVol > 0) {
                insertMarketOrder(instrument, CLOSE, - closeVol);
                remainVol -= closeVol;
            }
        }

        if (remainVol > 0) {
            insertMarketOrder(instrument, OPEN, - remainVol);
        }
    }
}

/*!
 * \brief CtpExecuter::buyMarket
 * 市价买进合约 (开多)
 *
 * \param instrument 合约代码
 * \param volume 买进数量 (大于零)
 * \param useSimulation 使用涨停板限价单模拟市价单
 */
void CtpExecuter::buyMarket(const QString &instrument, int volume, bool useSimulation)
{
    qDebug() << DATE_TIME << "buyMarket" << instrument << ": volume =" << volume;

    if (!canUseAnyPrice(instrument) || useSimulation) {
        // Use FAK instead of market order
        buyLimit(instrument, volume, upperLowerLimitCache.value(instrument).first, 1);
    } else {
        insertMarketOrder(instrument, OPEN, volume);
    }
}

/*!
 * \brief CtpExecuter::sellMarket
 * 市价卖出合约 (开空)
 *
 * \param instrument 合约代码
 * \param volume 卖出数量 (大于零)
 * \param useSimulation 使用跌停板限价单模拟市价单
 */
void CtpExecuter::sellMarket(const QString &instrument, int volume, bool useSimulation)
{
    qDebug() << DATE_TIME << "sellMarket" << instrument << ": volume =" << volume;

    if (!canUseAnyPrice(instrument) || useSimulation) {
        // Use FAK instead of market order
        sellLimit(instrument, volume, upperLowerLimitCache.value(instrument).second, 1);
    } else {
        insertMarketOrder(instrument, OPEN, - volume);
    }
}

/*!
 * \brief CtpExecuter::cancelOrder
 * 取消未成交的订单
 *
 * \param orderRefID 订单引用号
 * \param frontID 前置编号
 * \param sessionID 会话编号
 * \param instrument 合约代码
 */
void CtpExecuter::cancelOrder(int orderRefID, int frontID, int sessionID, const QString &instrument)
{
    if (loggedIn) {
        TThostFtdcOrderRefType orderRef;
        sprintf(orderRef, "%12d", orderRefID);
        orderAction(orderRef, frontID, sessionID, instrument);

        const auto orderList = orderMap.values();
        for (const auto &item : orderList) {
            if (item.refId == orderRefID && item.frontId == frontID && item.sessionId == sessionID) {
                QString instrument = item.instrument;
                orderCancelCountMap[instrument] ++;
                qInfo() << DATE_TIME << "Cancel order count of" << item.instrument << ":" << orderCancelCountMap[item.instrument];
                break;
            }
        }
    } else {
        qWarning() << DATE_TIME << "Cancel order failed! Not logged in!";
    }
}

/*!
 * \brief CtpExecuter::cacelAllOrders
 * 取消该合约未成交的订单
 *
 * \param instrument 合约代码, 如果为空代表取消所有合约上未成交的订单
 */
void CtpExecuter::cacelAllOrders(const QString &instrument)
{
    if (loggedIn) {
        const auto orderList = (instrument == "") ? orderMap.values() : orderMap.values(instrument);
        for (const auto &item : orderList) {
            if (item.status == OrderStatus::UNKNOWN || item.status == OrderStatus::PENDING) {
                TThostFtdcOrderRefType orderRef;
                sprintf(orderRef, "%12d", item.refId);
                orderAction(orderRef, item.frontId, item.sessionId, item.instrument);
                orderCancelCountMap[item.instrument] ++;
                qInfo() << "Cancel order count of" << item.instrument << ":" << orderCancelCountMap[item.instrument];
            }
        }
    } else {
        qWarning() << DATE_TIME << "Cancel order failed! Not logged in!";
    }
}

/*!
 * \brief CtpExecuter::parkBuyLimit
 * 预埋限价开多单, 不管已有持仓与否
 *
 * \param instrument 合约代码
 * \param volume 买进数量 (大于零)
 * \param price 买进价格 (必须在涨跌停范围内)
 * \param orderType 订单类型 (0:普通限价单, 1:FAK, 2:FOK)
 */
void CtpExecuter::parkBuyLimit(const QString& instrument, int volume, double price, int orderType)
{
    qDebug() << DATE_TIME << "parkBuyLimit" << instrument << ": volume =" << volume << ", price =" << price << ", orderType =" << orderType;

    bool allOrAny, gfdOrIoc;
    analyzeOrderType(orderType, allOrAny, gfdOrIoc);

    insertParkedLimitOrder(instrument, OPEN, volume, price, allOrAny, gfdOrIoc);
}

/*!
 * \brief CtpExecuter::parkSellLimit
 * 预埋限价开空单, 不管已有持仓与否
 *
 * \param instrument 合约代码
 * \param volume 卖出数量 (大于零)
 * \param price 卖出价格 (必须在涨跌停范围内)
 * \param orderType 订单类型 (0:普通限价单, 1:FAK, 2:FOK)
 */
void CtpExecuter::parkSellLimit(const QString& instrument, int volume, double price, int orderType)
{
    qDebug() << DATE_TIME << "parkSellLimit" << instrument << ": volume =" << volume << ", price =" << price << ", orderType =" << orderType;

    bool allOrAny, gfdOrIoc;
    analyzeOrderType(orderType, allOrAny, gfdOrIoc);

    insertParkedLimitOrder(instrument, OPEN, - volume, price, allOrAny, gfdOrIoc);
}

/*!
 * \brief CtpExecuter::setPosition
 * 为该合约设置一个新的目标仓位, 如果与当前仓位不同, 则执行相应操作以达成目标
 *
 * \param instrument 合约代码
 * \param newPosition 新目标仓位
 */
void CtpExecuter::setPosition(const QString& instrument, int newPosition)
{
    qDebug() << instrument << ":" << "new position =" << newPosition;

    const auto limit = upperLowerLimitCache.value(instrument);
    int position = getPosition(instrument);

    if (position != -INT_MAX) {
        int diff = newPosition - position;
        if (diff > 0) {
            buyLimitAuto(instrument, diff, limit.first);
        } else if (diff < 0) {
            sellLimitAuto(instrument, diff, limit.second);
        }
    }
}

/*!
 * \brief CtpExecuter::getPosition
 * 获取该合约的仓位, 如同时持有多仓和空仓则合并计算
 *
 * \param instrument 合约代码
 * \return 该合约的仓位, 正数表示净多头, 负数表示净空头, 如果缓存未同步返回-INT_MAX
 */
int CtpExecuter::getPosition(const QString& instrument) const
{
    if (cacheReady) {
        qDebug() << "ydLongPositions =" << ydLongPositions.value(instrument);
        qDebug() << "tdLongPositions =" << tdLongPositions.value(instrument);
        qDebug() << "ydShortPositions =" << ydShortPositions.value(instrument);
        qDebug() << "tdShortPositions =" << tdShortPositions.value(instrument);
        return ydLongPositions.value(instrument) + tdLongPositions.value(instrument) - ydShortPositions.value(instrument) - tdShortPositions.value(instrument);
    } else {
        qWarning() << DATE_TIME << "Cache is not ready!";
        return -INT_MAX;
    }
}

/*!
 * \brief CtpExecuter::getPendingOrderVolume
 * 获取该合约未成交订单的仓位
 * 该函数必须在成功登陆并更新订单表之后调用
 *
 * \param instrument 被查询的合约代码
 * \return 该合约未成交订单的仓位之和
 */
int CtpExecuter::getPendingOrderVolume(const QString &instrument) const
{
    int sum = 0;
    const auto orderList = orderMap.values(instrument);
    for (const auto& order : orderList) {
        if (order.status == OrderStatus::UNKNOWN ||
            order.status == OrderStatus::PENDING)
        {
            sum += order.remainVolume();
        }
    }
    return sum;
}

/*!
 * \brief CtpExecuter::execOption
 * 期权行权
 *
 * \param instrument 期权合约代码
 * \param volume 行权手数
 */
void CtpExecuter::execOption(const QString &instrument, int volume)
{
    if (loggedIn) {
        if (isOption(instrument)) {
            QString underlyingID;
            OPTION_TYPE type;
            int K;
            if (parseOptionID(instrument, underlyingID, type, K)) {
                insertExecOrder(instrument, type, volume);
            }
        } else {
            qWarning() << instrument << "is not option!";
        }
    } else {
        qWarning() << DATE_TIME << "Execute" << instrument << "failed! Not logged in!";
    }
}

/*!
 * \brief CtpExecuter::quote
 * 询价
 *
 * \param instrument 合约代码
 */
void CtpExecuter::quote(const QString &instrument)
{
    if (loggedIn) {
        insertQuote(instrument);
    } else {
        qWarning() << DATE_TIME << "Quote for" << instrument << "failed! Not logged in!";
    }
}

/*!
 * \brief CtpExecuter::quit
 * 退出
 */
void CtpExecuter::quit()
{
    QCoreApplication::quit();
}
