#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QDataStream>
#include <QCoreApplication>
#include <QDebugStateSaver>

#include "config_struct.h"
#include "market.h"
#include "common_utility.h"
#include "multiple_timer.h"
#include "market_watcher.h"
#include "market_watcher_adaptor.h"
#include "tick_receiver.h"

extern QList<Market> markets;

MarketWatcher::MarketWatcher(const CONFIG_ITEM &config, bool replayMode, QObject *parent) :
    QObject(parent),
    name(config.name),
    replayMode(replayMode)
{
    settings = getSettingsSmart(config.organization, config.name, this).release();
    const auto flowPath = settings->value("FlowPath").toByteArray();
    saveDepthMarketData = settings->value("SaveDepthMarketData").toBool();
    saveDepthMarketDataPath = settings->value("SaveDepthMarketDataPath").toString();
    QDir dir(saveDepthMarketDataPath);
    if (!dir.exists()) {
        qWarning() << "SaveDepthMarketDataPath:" << saveDepthMarketDataPath << "does not exist!";
        if (replayMode) {
            qFatal("Can not replay without SaveDepthMarketDataPath!");
            return;
        }
        if (!dir.mkpath(saveDepthMarketDataPath)) {
            qWarning() << "Create directory:" << saveDepthMarketDataPath << "failed! Depth market data will not be saved!";
            saveDepthMarketData = false;
        }
    }

    settings->beginGroup("SubscribeList");
    const auto subscribeList = settings->childKeys();
    for (const auto &key : subscribeList) {
        if (settings->value(key).toBool()) {
            subscribeSet.insert(key);
        }
    }
    settings->endGroup();

    new Market_watcherAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(config.dbusObject, this);
    dbus.registerService(config.dbusService);
// 复盘模式和实盘模式共用的部分到此为止 ---------------------------------------
    if (replayMode) {
        qInfo() << "Relay mode is ready!";
        return;
    }
// 以下设置仅用于实盘模式 --------------------------------------------------
    nRequestID = 0;

    for (const auto &instrumentID : qAsConst(subscribeSet)) {
        if (checkTradingTimes(instrumentID)) {
            setCurrentTradingTime(instrumentID);
        } else {
            qCritical() << instrumentID << "has no proper trading time!";
        }
    }

    settings->beginGroup("AccountInfo");
    brokerID = settings->value("BrokerID").toByteArray();
    userID = settings->value("UserID").toByteArray();
    password = settings->value("Password").toByteArray();
    settings->endGroup();

    // Pre-convert QString to char*
    c_brokerID = brokerID.constData();
    c_userID = userID.constData();
    c_password = password.constData();

    pUserApi = CThostFtdcMdApi::CreateFtdcMdApi(flowPath.constData());
    pReceiver = new CTickReceiver(this);
    pUserApi->RegisterSpi(pReceiver);

    settings->beginGroup("FrontSites");
    const auto keys = settings->childKeys();
    const QString protocol = "tcp://";
    for (const auto &str : keys) {
        QString address = settings->value(str).toString();
        pUserApi->RegisterFront((protocol + address).toLatin1().data());
    }
    settings->endGroup();

    if (saveDepthMarketData) {
        // Check the directories for saving market data
        for (const auto &instrumentID : qAsConst(subscribeSet)) {
            const QString path_for_this_instrumentID = saveDepthMarketDataPath + "/" + instrumentID;
            QDir dir(path_for_this_instrumentID);
            if (!dir.exists()) {
                bool ret = dir.mkpath(path_for_this_instrumentID);
                if (!ret) {
                    qWarning() << "Create directory" << path_for_this_instrumentID << "failed!";
                }
            }
        }
    }

    multiTimer = nullptr;
    setupTimers();

    loggedIn = false;

    pUserApi->Init();
    localTime.start();
}

MarketWatcher::~MarketWatcher()
{
    if (!replayMode) {
        pUserApi->Release();
        delete pReceiver;
    }
}

void MarketWatcher::setupTimers()
{
    QMap<QTime, QStringList> endPointsMap;
    for (const auto &instrumentID : qAsConst(subscribeSet)) {
        const auto endPoints = getEndPoints(instrumentID);
        for (const auto &item : endPoints) {
            endPointsMap[item] << instrumentID;
        }
    }

    auto keys = endPointsMap.keys();
    std::sort(keys.begin(), keys.end());
    QList<QTime> saveBarTimePoints;
    for (const auto &timePoint : qAsConst(keys)) {
        instrumentsToProcess.append(endPointsMap[timePoint]);
        saveBarTimePoints << timePoint.addSecs(60); // Save data 3 minutes after market close
    }

    if (multiTimer != nullptr) {
        disconnect(multiTimer, &MultipleTimer::timesUp, this, &MarketWatcher::timesUp);
        delete multiTimer;
    }
    multiTimer = new MultipleTimer(saveBarTimePoints);
    connect(multiTimer, &MultipleTimer::timesUp, this, &MarketWatcher::timesUp);
}

QDataStream& operator<<(QDataStream& s, const CThostFtdcDepthMarketDataField& dataField)
{
    s.writeRawData((const char*)&dataField, sizeof(CThostFtdcDepthMarketDataField));
    return s;
}

QDataStream& operator>>(QDataStream& s, CThostFtdcDepthMarketDataField& dataField)
{
    s.readRawData((char*)&dataField, sizeof(CThostFtdcDepthMarketDataField));
    return s;
}

QDebug operator<<(QDebug dbg, const CThostFtdcDepthMarketDataField &dm)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Ask 1:\t" << dm.AskPrice1 << '\t' << dm.AskVolume1 << '\n'
                  << " ------ " << QString("%1:%2").arg(dm.UpdateTime).arg(dm.UpdateMillisec, 3, 10, QLatin1Char('0'))
                  << " lastPrice:" << dm.LastPrice << " ------ " << '\n'
                  << "Bid 1:\t" << dm.BidPrice1 << '\t' << dm.BidVolume1;
    return dbg;
}

void MarketWatcher::timesUp(int index)
{
    const auto today = QDate::currentDate();

    if (!tradingCalendar.isTradingDay(today)) {
        if (!tradingCalendar.tradesTonight(today.addDays(-1))) {
            depthMarketDataListMap.clear();
        } else {
            if (QTime::currentTime() > QTime(5, 0)) {
                depthMarketDataListMap.clear();
            }
        }
    }

    for (const auto &instrumentID : qAsConst(subscribeSet)) {
        if (instrumentsToProcess[index].contains(instrumentID)) {
            setCurrentTradingTime(instrumentID);

            if (saveDepthMarketData) {
                auto &depthMarketDataList = depthMarketDataListMap[instrumentID];
                if (depthMarketDataList.length() > 0) {
                    QString fileName = saveDepthMarketDataPath + "/" + instrumentID + "/" + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss_zzz")) + ".data";
                    QFile depthMarketDataFile(fileName);
                    depthMarketDataFile.open(QFile::WriteOnly);
                    QDataStream wstream(&depthMarketDataFile);
                    wstream << depthMarketDataList;
                    depthMarketDataFile.close();
                    depthMarketDataList.clear();
                }
            }
        }
    }
}

void MarketWatcher::setCurrentTradingTime(const QString &instrumentID)
{
    const int len = tradingTimeMap[instrumentID].length();
    QList<int> timeDiffs;
    for (const auto & timePair : qAsConst(tradingTimeMap[instrumentID])) {
        auto diff = QTime::currentTime().secsTo(timePair.second);
        if (diff <= 0) {
            diff += 86400;
        }
        timeDiffs.append(diff);
    }

    int min = INT_MAX;
    int minIdx = -1;
    for (int i = 0; i < len; i++) {
        if (timeDiffs[i] < min) {
            min = timeDiffs[i];
            minIdx = i;
        }
    }

    if (minIdx >= 0 && minIdx < len) {
        currentTradingTimeMap[instrumentID].first = QTime(0, 0).secsTo(tradingTimeMap[instrumentID][minIdx].first);
        currentTradingTimeMap[instrumentID].second = QTime(0, 0).secsTo(tradingTimeMap[instrumentID][minIdx].second);
    } else {
        qDebug() << "minIdx =" << minIdx;
        qFatal("Should never see this!");
    }
}

void MarketWatcher::customEvent(QEvent *event)
{
    switch (int(event->type())) {
    case FRONT_CONNECTED:
        login();
        break;
    case FRONT_DISCONNECTED:
    {
        auto *fevent = static_cast<FrontDisconnectedEvent*>(event);
        qInfo() << "Front Disconnected! reason =" << fevent->getReason();
        loggedIn = false;
    }
        break;
    case HEARTBEAT_WARNING:
        break;
    case RSP_USER_LOGIN:
        qInfo() << "Market watcher logged in OK!";
        loggedIn = true;
        subscribe();
        break;
    case RSP_USER_LOGOUT:
        break;
    case RSP_ERROR:
    case RSP_SUB_MARKETDATA:
    case RSP_UNSUB_MARKETDATA:
        break;
    case DEPTH_MARKET_DATA:
    {
        auto *devent = static_cast<DepthMarketDataEvent*>(event);
        qDebug().noquote().nospace() << devent->DepthMarketDataField.InstrumentID << "\t" << name << "\n"
                 << devent->DepthMarketDataField;
        processDepthMarketData(devent->DepthMarketDataField);
    }
        break;
    default:
        QObject::customEvent(event);
        break;
    }
}

/*!
 * \brief MarketWatcher::login
 * 用配置文件中的账号信息登陆行情端.
 */
void MarketWatcher::login()
{
    CThostFtdcReqUserLoginField reqUserLogin;
    memset(&reqUserLogin, 0, sizeof (CThostFtdcReqUserLoginField));
    strcpy(reqUserLogin.BrokerID, c_brokerID);
    strcpy(reqUserLogin.UserID, c_userID);
    strcpy(reqUserLogin.Password, c_password);

    pUserApi->ReqUserLogin(&reqUserLogin, nRequestID.fetchAndAddRelaxed(1));
}

/*!
 * \brief MarketWatcher::subscribe
 * 订阅subscribeSet里的合约.
 */
void MarketWatcher::subscribe()
{
    const int num = subscribeSet.size();
    char* subscribe_array = new char[num * 32];
    char** ppInstrumentID = new char*[num];
    QSetIterator<QString> iterator(subscribeSet);
    for (int i = 0; i < num; i++) {
        ppInstrumentID[i] = strcpy(subscribe_array + 32 * i, iterator.next().toLatin1().constData());
    }

    pUserApi->SubscribeMarketData(ppInstrumentID, num);
    delete[] ppInstrumentID;
    delete[] subscribe_array;
}

/*!
 * \brief MarketWatcher::checkTradingTimes
 * 查找各个交易市场, 找到相应合约的交易时间并事先储存到map里.
 *
 * \param instrumentID 合约代码.
 * \return 是否找到了该合约的交易时间.
 */
bool MarketWatcher::checkTradingTimes(const QString &instrumentID)
{
    const QString code = getCode(instrumentID);
    for (const auto &market : qAsConst(markets)) {
        for (const auto &marketCode : qAsConst(market.codes)) {
            if (code == marketCode) {
                const int size = market.regexs.size();
                int i = 0;
                for (; i < size; i++) {
                    if (QRegExp(market.regexs[i]).exactMatch(instrumentID)) {
                        tradingTimeMap[instrumentID] = market.tradetimeses[i];
                        return true;
                    }
                }
                return false;   // instrumentID未能匹配任何正则表达式.
            }
        }
    }
    return false;
}

/*!
 * \brief MarketWatcher::processDepthMarketData
 * 处理深度市场数据:
 * 1. 过滤无效的(如在交易时间外的, 或数据有错误的)行情消息.
 * 2. 发送新行情数据(newMarketData signal).
 * 3. 如果需要, 将行情数据保存到文件.
 *
 * \param depthMarketDataField 深度市场数据.
 */
void MarketWatcher::processDepthMarketData(const CThostFtdcDepthMarketDataField& depthMarketDataField)
{
    const QString instrumentID(depthMarketDataField.InstrumentID);
    int time = hhmmssToSec(depthMarketDataField.UpdateTime);

    const auto &tradetime = currentTradingTimeMap[instrumentID];
    if (isWithinRange(time, tradetime.first, tradetime.second)) {
        int emitTime = (time == tradetime.second) ? (time == 0 ? 86399 : (time - 1)) : time;
        emit newMarketData(instrumentID,
                           emitTime,
                           depthMarketDataField.LastPrice,
                           depthMarketDataField.Volume,
                           depthMarketDataField.AskPrice1,
                           depthMarketDataField.AskVolume1,
                           depthMarketDataField.BidPrice1,
                           depthMarketDataField.BidVolume1);

        if (saveDepthMarketData) {
            auto mdToSave = depthMarketDataField;
            *((int*)mdToSave.ActionDay) = localTime.elapsed();  // Add timestamp
            depthMarketDataListMap[instrumentID].append(mdToSave);
        }
    }
}

/*!
 * \brief MarketWatcher::emitNewMarketData
 * 发送newMarketData信号 (仅用于reply mode)
 *
 * \param depthMarketDataField 深度市场数据.
 */
void MarketWatcher::emitNewMarketData(const CThostFtdcDepthMarketDataField& depthMarketDataField)
{
    const QString instrumentID(depthMarketDataField.InstrumentID);
    int time = hhmmssToSec(depthMarketDataField.UpdateTime);
    emit newMarketData(instrumentID,
                       time,
                       depthMarketDataField.LastPrice,
                       depthMarketDataField.Volume,
                       depthMarketDataField.AskPrice1,
                       depthMarketDataField.AskVolume1,
                       depthMarketDataField.BidPrice1,
                       depthMarketDataField.BidVolume1);
}

/*!
 * \brief MarketWatcher::getStatus
 * 获取状态字符串.
 *
 * \return 状态.
 */
QString MarketWatcher::getStatus() const
{
    if (replayMode || (!replayMode && loggedIn)) {
        return "Ready";
    } else {
        return "NotReady";
    }
}

/*!
 * \brief MarketWatcher::getTradingDay
 * 获取交易日.
 *
 * \return 交易日(格式YYYYMMDD)
 */
QString MarketWatcher::getTradingDay() const
{
    if (replayMode) {
        return "ReplayMode";    // FIXME
    } else {
        return pUserApi->GetTradingDay();
    }
}

/*!
 * \brief MarketWatcher::subscribeInstruments
 * 订阅合约.
 *
 * \param instruments 合约列表.
 * \param updateIni 是否将订阅的合约列表更新到配置文件.
 */
void MarketWatcher::subscribeInstruments(const QStringList &instruments, bool updateIni)
{
    if (replayMode) {
        qWarning() << "Can not subscribe instruments in replay mode!";
        return;
    }

    const int num = instruments.size();
    char* subscribe_array = new char[num * 32];
    char** ppInstrumentID = new char*[num];

    for (int i = 0; i < num; i++) {
        subscribeSet.insert(instruments[i]);    // 更新订阅列表.
        ppInstrumentID[i] = strcpy(subscribe_array + 32 * i, instruments[i].toLatin1().constData());
    }

    if (loggedIn) {
        pUserApi->SubscribeMarketData(ppInstrumentID, num);
    }
    delete[] ppInstrumentID;
    delete[] subscribe_array;

    if (saveDepthMarketData) {
        for (const QString &instrumentID : instruments) {
            const QString path_for_this_instrumentID = saveDepthMarketDataPath + "/" + instrumentID;
            QDir dir(path_for_this_instrumentID);
            if (!dir.exists()) {
                bool ret = dir.mkpath(path_for_this_instrumentID);
                if (!ret) {
                    qWarning() << "Create directory" << path_for_this_instrumentID << "failed!";
                }
            }
        }
    }

    setupTimers();

    if (updateIni) {
        settings->beginGroup("SubscribeList");
        const auto subscribeList = settings->childKeys();
        QStringList enabledSubscribeList;
        for (const auto &key : subscribeList) {
            if (settings->value(key).toBool()) {
                enabledSubscribeList.append(key);
            }
        }

        for (const auto &instrument : instruments) {
            if (!enabledSubscribeList.contains(instrument)) {
                settings->setValue(instrument, "1");
            }
        }
        settings->endGroup();
    }
}

/*!
 * \brief MarketWatcher::getSubscribeList
 * 获取订阅合约列表.
 *
 * \return 订阅合约列表.
 */
QStringList MarketWatcher::getSubscribeList() const
{
    return subscribeSet.toList();
}

/*!
 * \brief MarketWatcher::startReplay
 * 复盘某一天的行情.
 *
 * \param date 希望复盘的日期 (格式YYYYMMDD)
 */
void MarketWatcher::startReplay(const QString &date)
{
    if (!replayMode) {
        qWarning() << "Not in replay mode!";
        return;
    }

    qDebug() << "Start replaying" << date;
    QList<CThostFtdcDepthMarketDataField> mdList;

    for (const auto &instrumentID : subscribeSet) {
        QDir dir(saveDepthMarketDataPath + "/" + instrumentID);
        const auto marketDataFiles = dir.entryList(QStringList({date + "*"}), QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
        for (const auto &fileName : marketDataFiles) {
            QString mdFileFullName = saveDepthMarketDataPath + "/" + instrumentID + "/" + fileName;
            QFile mdFile(mdFileFullName);
            if (!mdFile.open(QFile::ReadOnly)) {
                qCritical() << "Open file:" << mdFileFullName << "failed!";
                continue;
            }
            QDataStream mdStream(&mdFile);
            QList<CThostFtdcDepthMarketDataField> tmpList;
            mdStream >> tmpList;
            if (tmpList.length() > 0) {
                qDebug() << tmpList.size() << "in" << mdFileFullName;
                mdList.append(tmpList);
            }
        }
    }

    std::stable_sort(mdList.begin(), mdList.end(), [](const auto &item1, const auto &item2) -> bool {
        int ret = QString::compare(item1.UpdateTime, item2.UpdateTime);
        if (ret == 0) {
            if (item1.UpdateMillisec == item2.UpdateMillisec) {
                return *((int*)item1.ActionDay) < *((int*)item2.ActionDay); // 比较本地时间戳.
            } else {
                return item1.UpdateMillisec < item2.UpdateMillisec;
            }
        } else {
            return ret < 0;
        }
    });

    if (!mdList.empty()) {
        emit tradingDayChanged(date);

        for (const auto &md : qAsConst(mdList)) {
            emitNewMarketData(md);
        }
        emit endOfReplay(date);
    }
}

/*!
 * \brief MarketWatcher::quit
 * 退出.
 */
void MarketWatcher::quit()
{
    QCoreApplication::quit();
}
