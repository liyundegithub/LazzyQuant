#include <QtConcurrentRun>
#include <QSettings>
#include <QDebug>

#include "config_struct.h"
#include "market.h"
#include "utility.h"
#include "multiple_timer.h"
#include "market_watcher.h"
#include "market_watcher_adaptor.h"
#include "tick_receiver.h"

extern QList<Market> markets;

MarketWatcher::MarketWatcher(const CONFIG_ITEM &config, const bool replayMode, QObject *parent) :
    replayMode(replayMode), QObject(parent)
{
    nRequestID = 0;

    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, config.organization, config.name, this);
    QByteArray flowPath = settings->value("FlowPath").toByteArray();
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
    QStringList subscribeList = settings->childKeys();
    for (const QString &key : subscribeList) {
        if (settings->value(key).toBool()) {
            subscribeSet.insert(key);
        }
    }
    settings->endGroup();

    for (const QString &instrumentID : subscribeSet) {
        if (!checkTradingTimes(instrumentID)) {
            qCritical() << instrumentID << "has no proper trading time!";
        }
    }

    new Market_watcherAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(config.dbusObject, this);
    dbus.registerService(config.dbusService);
// 复盘模式到此为止 -------------------------------------------------------
    if (replayMode) {
        return;
    }
// 以下是实盘模式的相关设置 -------------------------------------------------
    settings->beginGroup("AccountInfo");
    brokerID = settings->value("BrokerID").toByteArray();
    userID = settings->value("UserID").toByteArray();
    password = settings->value("Password").toByteArray();
    settings->endGroup();

    // Pre-convert QString to char*
    c_brokerID = brokerID.data();
    c_userID = userID.data();
    c_password = password.data();

    pUserApi = CThostFtdcMdApi::CreateFtdcMdApi(flowPath.data());
    pReceiver = new CTickReceiver(this);
    pUserApi->RegisterSpi(pReceiver);

    settings->beginGroup("FrontSites");
    QStringList keys = settings->childKeys();
    const QString protocol = "tcp://";
    for (const QString &str : keys) {
        QString address = settings->value(str).toString();
        pUserApi->RegisterFront((protocol + address).toLatin1().data());
    }
    settings->endGroup();

    if (saveDepthMarketData)
        prepareSaveDepthMarketData();

    loggedIn = false;

    pUserApi->Init();
}

MarketWatcher::~MarketWatcher()
{
    if (!replayMode) {
        pUserApi->Release();
        delete pReceiver;
    }
}

void MarketWatcher::prepareSaveDepthMarketData()
{
    for (const QString &instrumentID : subscribeSet) {
        const QString path_for_this_instrumentID = saveDepthMarketDataPath + "/" + instrumentID;
        QDir dir(path_for_this_instrumentID);
        if (!dir.exists()) {
            bool ret = dir.mkpath(path_for_this_instrumentID);
            if (!ret) {
                qWarning() << "Create directory" << path_for_this_instrumentID << "failed!";
            }
        }
    }

    QMap<QTime, QStringList> endPointsMap;
    for (const QString &instrumentID : subscribeSet) {
        auto endPoints = getEndPoints(instrumentID);
        for (const auto &item : endPoints) {
            endPointsMap[item] << instrumentID;
        }
    }

    auto keys = endPointsMap.keys();
    qSort(keys);
    for (const auto &item : keys) {
        instrumentsToSave.append(endPointsMap[item]);
        saveBarTimePoints << item.addSecs(180); // Save data 3 minutes after market close
    }

    auto saveBarTimer = new MultipleTimer(saveBarTimePoints, this);
    connect(saveBarTimer, &MultipleTimer::timesUp, this, &MarketWatcher::saveDepthMarketDataToFile);
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

void MarketWatcher::saveDepthMarketDataToFile(int index)
{
    for (const auto &instrumentID : subscribeSet) {
        if (instrumentsToSave[index].contains(instrumentID)) {
            auto &depthMarketDataList = depthMarketDataListMap[instrumentID];
            if (depthMarketDataList.length() > 0) {
                QString file_name = saveDepthMarketDataPath + "/" + instrumentID + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz") + ".data";
                QFile depthMarketDataFile(file_name);
                depthMarketDataFile.open(QFile::WriteOnly);
                QDataStream wstream(&depthMarketDataFile);
                wstream << depthMarketDataList;
                depthMarketDataFile.close();
                depthMarketDataList.clear();
            }
        }
    }
}

void MarketWatcher::customEvent(QEvent *event)
{
    qDebug() << "customEvent: " << int(event->type());
    switch (int(event->type())) {
    case FRONT_CONNECTED:
        login();
        break;
    case FRONT_DISCONNECTED:
    {
        loggedIn = false;
        auto *fevent = static_cast<FrontDisconnectedEvent*>(event);
        // TODO
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
    case HEARTBEAT_WARNING:
    {
        auto *hevent = static_cast<HeartBeatWarningEvent*>(event);
        emit heartBeatWarning(hevent->getLapseTime());
    }
        break;
    case RSP_USER_LOGIN:
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
 * 用配置文件中的账号信息登陆行情端
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
 * 订阅subscribeSet里的合约
 */
void MarketWatcher::subscribe()
{
    const int num = subscribeSet.size();
    char* subscribe_array = new char[num * 32];
    char** ppInstrumentID = new char*[num];
    QSetIterator<QString> iterator(subscribeSet);
    for (int i = 0; i < num; i++) {
        ppInstrumentID[i] = strcpy(subscribe_array + 32 * i, iterator.next().toLatin1().data());
    }

    pUserApi->SubscribeMarketData(ppInstrumentID, num);
    delete[] ppInstrumentID;
    delete[] subscribe_array;
}

/*!
 * \brief MarketWatcher::checkTradingTimes
 * 查找各个交易市场, 找到相应合约的交易时间并事先储存到map里
 *
 * \param instrumentID 合约代码
 * \return 是否找到了该合约的交易时间
 */
bool MarketWatcher::checkTradingTimes(const QString &instrumentID)
{
    const QString instrument = getCode(instrumentID);
    for (const auto &market : markets) {
        for (const auto &code : market.codes) {
            if (instrument == code) {
                int i = 0, size = market.regexs.size();
                for (; i < size; i++) {
                    if (QRegExp(market.regexs[i]).exactMatch(instrumentID)) {
                        tradingTimeMap[instrumentID] = market.tradetimeses[i];
                        return true;
                    }
                }
                return false;   // instrumentID未能匹配任何正则表达式
            }
        }
    }
    return false;
}

/*!
 * \brief MarketWatcher::processDepthMarketData
 * 处理深度市场数据:
 * 1. 过滤无效的(如在交易时间外的, 或数据有错误的)行情消息
 * 2. 发送新行情数据(newMarketData signal)
 * 3. 如果需要, 将行情数据保存到文件
 *
 * \param depthMarketDataField 深度市场数据
 */
void MarketWatcher::processDepthMarketData(const CThostFtdcDepthMarketDataField& depthMarketDataField)
{
    const QString instrumentID(depthMarketDataField.InstrumentID);
    QTime time = QTime::fromString(depthMarketDataField.UpdateTime, "hh:mm:ss");

    for (const auto &tradetime : tradingTimeMap[instrumentID]) {
        if (isWithinRange(time, tradetime.first, tradetime.second)) {
            QTime emitTime = (time == tradetime.second) ? time.addSecs(-1) : time;
            emit newMarketData(instrumentID,
                               QTime(0, 0).secsTo(emitTime),
                               depthMarketDataField.LastPrice,
                               depthMarketDataField.Volume,
                               depthMarketDataField.AskPrice1,
                               depthMarketDataField.AskVolume1,
                               depthMarketDataField.BidPrice1,
                               depthMarketDataField.BidVolume1);

            if (saveDepthMarketData)
                depthMarketDataListMap[instrumentID].append(depthMarketDataField);
            break;
        }
    }
}

/*!
 * \brief MarketWatcher::getTradingDay
 * 获取交易日
 *
 * \return 交易日(格式YYYYMMDD)
 */
QString MarketWatcher::getTradingDay() const
{
    if (replayMode) {
        // TODO
    } else {
        return pUserApi->GetTradingDay();
    }
}

/*!
 * \brief MarketWatcher::subscribeInstruments
 * 订阅合约
 *
 * \param instruments 合约列表
 * \param updateIni 是否将订阅的合约列表更新到配置文件
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
        subscribeSet.insert(instruments[i]);    // 更新订阅列表
        ppInstrumentID[i] = strcpy(subscribe_array + 32 * i, instruments[i].toLatin1().data());
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

    if (updateIni) {
        settings->beginGroup("SubscribeList");
        QStringList subscribeList = settings->childKeys();
        QStringList enabledSubscribeList;
        for (const QString &key : subscribeList) {
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
 * 获取订阅合约列表
 *
 * \return 订阅合约列表
 */
QStringList MarketWatcher::getSubscribeList() const
{
    return subscribeSet.toList();
}

/*!
 * \brief MarketWatcher::startReplay
 * 复盘某一天的行情
 *
 * \param date 希望复盘的日期 (格式YYYYMMDD)
 * \param realSpeed 是否以实际速度复盘 (默认为false)
 */
void MarketWatcher::startReplay(const QString &date, bool realSpeed)
{
    if (!replayMode) {
        qWarning() << "Not in replay mode!";
        return;
    }

    qDebug() << "Start replaying" << date;

    // TODO realSpeed = true

    depthMarketDataListMap.clear();

    for (const auto &instrumentID : subscribeSet) {
        QDir dir(saveDepthMarketDataPath + "/" + instrumentID);
        auto marketDataFiles = dir.entryList(QStringList({date + "*"}), QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
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
            qDebug() << tmpList.size() << "in" << mdFileFullName;
            if (tmpList.length() > 0) {
                depthMarketDataListMap[instrumentID].append(tmpList);
            }
        }
    }

    QtConcurrent::run([=]() -> void {
        // TODO 过午夜零点的情况
        QList<CThostFtdcDepthMarketDataField> beforeMidnight;

        for (const auto &mdList : depthMarketDataListMap) {
            beforeMidnight.append(mdList);
        }

        const auto mdLessThen = [](auto item1, auto item2) -> bool {
            QTime time1 = QTime::fromString(item1.UpdateTime, "hh:mm:ss");
            QTime time2 = QTime::fromString(item2.UpdateTime, "hh:mm:ss");
            time1.addMSecs(item1.UpdateMillisec);
            time2.addMSecs(item2.UpdateMillisec);
            return time1 < time2;
        };

        std::stable_sort(beforeMidnight.begin(), beforeMidnight.end(), mdLessThen);

        for (const auto &depthMarketDataField : beforeMidnight) {
            const QString instrumentID(depthMarketDataField.InstrumentID);
            QTime time = QTime::fromString(depthMarketDataField.UpdateTime, "hh:mm:ss");

            emit newMarketData(instrumentID,
                               QTime(0, 0).secsTo(time),
                               depthMarketDataField.LastPrice,
                               depthMarketDataField.Volume,
                               depthMarketDataField.AskPrice1,
                               depthMarketDataField.AskVolume1,
                               depthMarketDataField.BidPrice1,
                               depthMarketDataField.BidVolume1);
        }
    });
}

/*!
 * \brief MarketWatcher::quit
 * 退出
 */
void MarketWatcher::quit()
{
    QCoreApplication::quit();
}
