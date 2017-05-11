#include <cfloat>
#include <QTimer>
#include <QSettings>
#include <QStringList>

#include "config.h"
#include "multiple_timer.h"
#include "trading_calendar.h"
#include "option_pricing.h"
#include "option_helper.h"
#include "depth_market.h"
#include "risk_free.h"
#include "high_frequency.h"
#include "option_arbitrageur.h"

#include "market_watcher_interface.h"
#include "trade_executer_interface.h"

com::lazzyquant::market_watcher *pWatcher = nullptr;
com::lazzyquant::trade_executer *pExecuter = nullptr;

TradingCalendar tradingCalendar;

OptionArbitrageur::OptionArbitrageur(bool replayMode, const QString &replayDate, QObject *parent) :
    QObject(parent)
{
    pPricingEngine = nullptr;
    pDepthMarkets = nullptr;
    pStrategy = nullptr;

    loadOptionArbitrageurSettings();

    pExecuter = new com::lazzyquant::trade_executer(EXECUTER_DBUS_SERVICE, EXECUTER_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
    pWatcher = new com::lazzyquant::market_watcher(WATCHER_DBUS_SERVICE, WATCHER_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
    connect(pWatcher, SIGNAL(newMarketData(QString, uint, double, int, double, int, double, int)), this, SLOT(onMarketData(QString, uint, double, int, double, int, double, int)));

// 复盘模式和实盘模式共用的部分到此为止 ---------------------------------------
    if (replayMode) {
        QStringList options;
        const QStringList subscribeList = pWatcher->getSubscribeList();
        for (const auto &item : subscribeList) {
            if (isOption(item)) {
                options << item;
            }
        }
        pWatcher->setReplayDate(replayDate);
        if (!underlyingsForRiskFree.empty()) {
            setupRiskFree(options);
        } else {
            setupHighFreq(options);
        }
        pWatcher->startReplay(replayDate);
        qDebug() << "Relay mode is ready!";
        return;
    }
// 以下设置仅用于实盘模式 --------------------------------------------------

    updateRetryCounter = 0;
    updateOptions();

    QList<QTime> timePoints = { QTime(8, 30), QTime(17, 1), QTime(20, 30) };
    auto multiTimer = new MultipleTimer(timePoints, this);
    connect(multiTimer, &MultipleTimer::timesUp, this, &OptionArbitrageur::timesUp);
}

OptionArbitrageur::~OptionArbitrageur()
{
    qDebug() << "~OptionArbitrageur";

    if (pPricingEngine != nullptr)
        delete pPricingEngine;
    if (pDepthMarkets != nullptr)
        delete pDepthMarkets;
    if (pStrategy != nullptr)
        delete pStrategy;
}

void OptionArbitrageur::updateOptions()
{
    if (!pWatcher->isValid() || !pExecuter->isValid()) {
        if (updateRetryCounter < 3) {
            qWarning() << "Either Watcher or Executer is not ready! Will try update options later!";
            QTimer::singleShot(10000, this, &OptionArbitrageur::updateOptions);
            updateRetryCounter ++;
            return;
        } else {
            qWarning() << "Update options failed!";
            updateRetryCounter = 0;
            return;
        }
    }

    QTimer::singleShot(1000, [=]() -> void {
        pExecuter->updateInstrumentDataCache();
    });

    QTimer::singleShot(5000, [=]() -> void {
        const QStringList subscribedInstruments = pWatcher->getSubscribeList();
        const QStringList cachedInstruments = pExecuter->getCachedInstruments();

        QStringList instrumentsToSubscribe;
        for (const auto &item : cachedInstruments) {
            if (!subscribedInstruments.contains(item)) {
                for (const auto &underlyingID : qAsConst(underlyingIDs)) {
                    if (item.startsWith(underlyingID)) {
                        instrumentsToSubscribe << item;
                        break;
                    }
                }
            }
        }
        if (!instrumentsToSubscribe.empty()) {
            pWatcher->subscribeInstruments(instrumentsToSubscribe);
        }

        QStringList options;
        for (const auto &item : cachedInstruments) {
            if (isOption(item)) {
                options << item;
            }
        }
        if (!underlyingsForRiskFree.empty()) {
            setupRiskFree(options);
        } else {
            setupHighFreq(options);
        }
    });

    updateRetryCounter = 0;
}

void OptionArbitrageur::timesUp(int index)
{
    switch (index) {
    case 0:
        updateOptions();
        break;
    case 1:
        // Clear market data after maket closed
        pDepthMarkets->clearAll();
        break;
    case 2:
        updateOptions();
        break;
    default:
        qWarning() << "Should never see this! Something is wrong! index =" << index;
        break;
    }
}

void OptionArbitrageur::loadOptionArbitrageurSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, ORGANIZATION, "option_arbitrageur");
    riskFreeInterestRate = settings.value("RiskFreeInterestRate", 0.05).toDouble();
    qDebug() << "Risk-free interest rate =" << riskFreeInterestRate;

    settings.beginGroup("Underlyings");
    const auto underlyingList = settings.childKeys();
    for (const auto &key : underlyingList) {
        if (settings.value(key).toBool()) {
            underlyingIDs.insert(key);
        }
    }
    settings.endGroup();
    qDebug() << "Underlyings:" << underlyingIDs;

    settings.beginGroup("RiskFree");
    const auto riskFreeList = settings.childKeys();
    for (const auto &key : riskFreeList) {
        if (settings.value(key).toBool()) {
            underlyingsForRiskFree << key;
        }
    }
    settings.endGroup();
    qDebug() << "UnderlyingsForRiskFree:" << underlyingsForRiskFree;

    settings.beginGroup("HighFreq");
    const auto highFreqList = settings.childKeys();
    for (const auto &key : highFreqList) {
        if (settings.value(key).toBool()) {
            underlyingsForHighFreq << key;
        }
    }
    settings.endGroup();
    qDebug() << "UnderlyingsForHighFreq:" << underlyingsForHighFreq;
}

static inline QDate getEndDate(const QString &underlying)
{
    if (pExecuter->isValid()) {
        const QString dateStr = pExecuter->getExpireDate(underlying);
        if (dateStr != INVALID_DATE_STRING) {
            return QDate::fromString(dateStr, "yyyyMMdd");
        }
    }
    return getExpireDate(underlying);
}

static inline QMultiMap<QString, int> getUnderlyingKMap(const QStringList &underlyings, const QStringList &options)
{
    QMultiMap<QString, int> underlyingKMap;
    for (const auto &underlyingID : underlyings) {
        for (const auto &optionID : options) {
            if (optionID.startsWith(underlyingID)) {
                QString underlyingID;
                OPTION_TYPE type;
                int K;
                if (parseOptionID(optionID, underlyingID, type, K)) {
                    underlyingKMap.insert(underlyingID, K);
                }
            }
        }
    }
    return underlyingKMap;
}

void OptionArbitrageur::setupRiskFree(const QStringList &options)
{
    QMultiMap<QString, int> underlyingKMap = getUnderlyingKMap(underlyingsForRiskFree, options);

    if (pDepthMarkets != nullptr) {
        delete pDepthMarkets;
        pDepthMarkets = nullptr;
    }
    pDepthMarkets = new DepthMarketCollection(underlyingKMap);

    if (pStrategy != nullptr) {
        delete pStrategy;
        pStrategy = nullptr;
    }
    pStrategy = new RiskFree(2.6, pDepthMarkets);
}

void OptionArbitrageur::setupHighFreq(const QStringList &options)
{
    QMultiMap<QString, int> underlyingKMap = getUnderlyingKMap(underlyingsForHighFreq, options);

    preparePricing(underlyingKMap);

    if (pDepthMarkets != nullptr) {
        delete pDepthMarkets;
        pDepthMarkets = nullptr;
    }
    pDepthMarkets = new DepthMarketCollection(underlyingKMap);

    if (pStrategy != nullptr) {
        delete pStrategy;
        pStrategy = nullptr;
    }
    pStrategy = new HighFrequency(pPricingEngine, pDepthMarkets);
}

void OptionArbitrageur::preparePricing(const QMultiMap<QString, int> &underlyingKMap)
{
    QList<double> sigmaList;
    for (double sigma = 0.01; sigma < 1.0; sigma *= 1.1) {
        sigmaList << sigma;
    }
    qDebug() << "Use sigma:" << sigmaList;

    double maxPrice = 3400.0;   // FIXME
    double minPrice = 2300.0;   // FIXME

    const auto keys = underlyingKMap.uniqueKeys();
    if (pExecuter->isValid()) {
        maxPrice = -DBL_MAX;
        minPrice = DBL_MAX;
        for (const auto key : keys) {
            const double upperLimit = pExecuter->getUpperLimit(key);
            const double lowerLimit = pExecuter->getLowerLimit(key);
            if (upperLimit > maxPrice) {
                maxPrice = upperLimit;
            }
            if (lowerLimit < minPrice) {
                minPrice = lowerLimit;
            }
        }
    }

    QList<double> s0List;
    for (double s0 = minPrice; s0 < maxPrice; s0 += 4.0) {
        s0List << s0;
    }
    qDebug() << "Use s0:" << s0List;

    QDate startDate;
    if (pWatcher->isValid()) {
        startDate = QDate::fromString(pWatcher->getTradingDay(), "yyyyMMdd");
    } else {
        startDate = QDate::currentDate();
    }

    if (pPricingEngine != nullptr) {
        delete pPricingEngine;
        pPricingEngine = nullptr;
    }

    pPricingEngine = new OptionPricing(underlyingKMap);
    pPricingEngine->setBasicParam(riskFreeInterestRate, riskFreeInterestRate);
    pPricingEngine->setS0AndSigma(s0List, sigmaList);
    for (const auto &key : keys) {
        qDebug() << "Calculating:" << key;
        pPricingEngine->generate(key, startDate, getEndDate(key));
    }
}

/*!
 * \brief OptionArbitrageur::onMarketData
 * 处理市场数据, 寻找套利机会
 *
 * \param instrumentID 合约代码
 * \param time       时间
 * \param lastPrice  最新成交价
 * \param volume     成交量
 * \param askPrice1  卖一价
 * \param askVolume1 卖一量
 * \param bidPrice1  买一价
 * \param bidVolume1 买一量
 */
void OptionArbitrageur::onMarketData(const QString& instrumentID, uint time, double lastPrice, int volume,
                                     double askPrice1, int askVolume1, double bidPrice1, int bidVolume1)
{
    Q_UNUSED(volume)

    const DepthMarket latest_market_data = {
        time,
        lastPrice,
        askPrice1,
        askVolume1,
        bidPrice1,
        bidVolume1
    };

    DepthMarket * pDM = nullptr;

    int underlyingIdx;
    OPTION_TYPE type;
    int kIdx;

    const bool instrumentIsOption = isOption(instrumentID);
    if (instrumentIsOption) {
        // Try to parse instrumentID as option
        if (pDepthMarkets->parseOptionIdx(instrumentID, underlyingIdx, type, kIdx)) {
            if (type == CALL_OPT) {
                pDM = &(pDepthMarkets->ppCallOption[underlyingIdx][kIdx]);
            } else {
                pDM = &(pDepthMarkets->ppPutOption[underlyingIdx][kIdx]);
            }
        } else {
            return;
        }
    } else {
        // Underlying
        underlyingIdx = pDepthMarkets->getIdxByUnderlyingID(instrumentID);
        if (underlyingIdx != -1) {
            pDM = &(pDepthMarkets->pUnderlyingMarket[underlyingIdx]);
        } else {
            return;
        }
    }

    const bool significantChange = pDM->significantChange(latest_market_data);
    *pDM = latest_market_data;
    if (significantChange) {
        if (instrumentIsOption) {
            pStrategy->onOptionChanged(underlyingIdx, type, kIdx);
        } else {
            pStrategy->onUnderlyingChanged(underlyingIdx);
        }
    } else {
        // Market does not change much, do nothing
    }
}
