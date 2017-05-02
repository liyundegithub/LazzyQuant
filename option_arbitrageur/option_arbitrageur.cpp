#include <QMap>
#include <QTimer>
#include <QSettings>
#include <QStringList>

#include "config.h"
#include "multiple_timer.h"
#include "trading_calendar.h"
#include "option_pricing.h"
#include "option_helper.h"
#include "option_arbitrageur.h"

TradingCalendar tradingCalendar;

static inline void makeInvalid(DepthMarket &depthMarket)
{
    depthMarket.time = 0;
}

QDebug operator<<(QDebug dbg, const DepthMarket &depthMarket)
{
    dbg.nospace() << "Ask 1:\t" << depthMarket.askPrice << '\t' << depthMarket.askVolume << '\n'
                  << " ------ " << QTime(0, 0).addSecs(depthMarket.time).toString() <<  " ------ " << '\n'
                  << "Bid 1:\t" << depthMarket.bidPrice << '\t' << depthMarket.bidVolume;
    return dbg.space();
}

static inline bool isTimeCloseEnouogh(const DepthMarket &md1, const DepthMarket &md2)
{
    // 两个无符号数不能用qAbs(md1.time - md2.time) < 120;
    if (md1.time > md2.time) {
        return (md1.time - md2.time) < 120;
    } else {
        return (md2.time - md1.time) < 120;
    }
}

static inline int getVol(const int liquidity)
{
    if (liquidity > 1) {
        return 1;
    } else {
        return 0;
    }
}

OptionArbitrageur::OptionArbitrageur(int number, const bool replayMode, const QString replayDate, QObject *parent) :
    QObject(parent)
{
    loadOptionArbitrageurSettings();

    Q_ASSERT(allowTradeNumber > 0);
    if (number > 0) {
        qDebug() << "Command line overrides ini file settings";
        allowTradeNumber = number;
    }
    qDebug() << "allowTradeNumber =" << allowTradeNumber;

    pExecuter = new com::lazzyquant::trade_executer(EXECUTER_DBUS_SERVICE, EXECUTER_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
    pWatcher = new com::lazzyquant::market_watcher(WATCHER_DBUS_SERVICE, WATCHER_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
    connect(pWatcher, SIGNAL(newMarketData(QString, uint, double, int, double, int, double, int)), this, SLOT(onMarketData(QString, uint, double, int, double, int, double, int)));

// 复盘模式和实盘模式共用的部分到此为止 ---------------------------------------
    if (replayMode) {
        qDebug() << "Relay mode is ready!";
        QStringList options;
        QStringList subscribeList = pWatcher->getSubscribeList();
        for (const auto &item : subscribeList) {
            if (isOption(item)) {
                options << item;
            }
        }
        pWatcher->setReplayDate(replayDate);
        preparePricing(options);
        pWatcher->startReplay(replayDate);
        return;
    }
// 以下设置仅用于实盘模式 --------------------------------------------------

    updateRetryCounter = 0;
    updateOptions();

    QList<QTime> timePoints = { QTime(8, 50), QTime(17, 1), QTime(20, 50) };
    auto multiTimer = new MultipleTimer(timePoints, this);
    connect(multiTimer, &MultipleTimer::timesUp, this, &OptionArbitrageur::timesUp);
}

OptionArbitrageur::~OptionArbitrageur()
{
    qDebug() << "~OptionArbitrageur";
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
        pExecuter->updateInstrumentsCache(underlyingIDs.toList());
    });

    QTimer::singleShot(5000, [=]() -> void {    // FIXME 4秒可能不够
        const QStringList subscribedInstruments = pWatcher->getSubscribeList();
        const QStringList cachedInstruments = pExecuter->getCachedInstruments();

        QStringList instrumentsToSubscribe;
        for (const auto &item : cachedInstruments) {
            if (!subscribedInstruments.contains(item)) {
                instrumentsToSubscribe << item;
            }
        }
        if (instrumentsToSubscribe.length() > 0) {
            pWatcher->subscribeInstruments(instrumentsToSubscribe);
        }

        QStringList options;
        for (const auto &item : cachedInstruments) {
            if (isOption(item)) {
                options << item;
            }
        }
        preparePricing(options);
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
        future_market_data.clear();
        option_market_data.clear();
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
    allowTradeNumber = settings.value("AllowTradeNumber", 1).toInt();
    qDebug() << "AllowTradeNumber =" << allowTradeNumber;

    settings.beginGroup("Underlyings");
    const auto subscribeList = settings.childKeys();
    for (const auto &key : subscribeList) {
        if (settings.value(key).toBool()) {
            underlyingIDs.insert(key);
        }
    }
    settings.endGroup();

    settings.beginGroup("Thresholds");
    for (const auto &underlying : qAsConst(underlyingIDs)) {
        bool ok;
        double t = settings.value(underlying).toDouble(&ok);
        if (!ok) {
            t = 1000.0;
        }
        thresholdMap[underlying] = t;
    }
    settings.endGroup();
}

void OptionArbitrageur::preparePricing(const QStringList &options)
{
    if (!pricingMap.isEmpty()) {
        for (const auto pPricing : pricingMap) {
            delete pPricing;
        }
        pricingMap.clear();
    }

    QList<double> sigmaList;
    for (double sigma = 0.01; sigma < 1.0; sigma *= 1.4) {
        sigmaList << sigma;
    }

    QDate startDate;
    if (pWatcher->isValid()) {
        startDate = QDate::fromString(pWatcher->getTradingDay(), "yyyyMMdd");
    } else {
        startDate = QDate::currentDate();
    }

    for (const auto &underlying : qAsConst(underlyingIDs)) {
        double upperLimit;
        double lowerLimit;
        QDate endDate;

        if (pExecuter->isValid()) {
            upperLimit = pExecuter->getUpperLimit(underlying);
            lowerLimit = pExecuter->getLowerLimit(underlying);
            endDate = QDate::fromString(pExecuter->getExpireDate(underlying), "yyyyMMdd");
        } else {
            upperLimit = 10000.0;   // FIXME
            lowerLimit = 1000.0;    // FIXME
            endDate = getExpireDate(underlying);
        }

        QList<double> s0List;
        for (double s0 = (lowerLimit - 1.0); s0 < (upperLimit + 5.0); s0 += 4.0) {
            s0List << s0;
        }
        QSet<double> kSet;
        for (const auto &optionID : options) {
            if (optionID.startsWith(underlying)) {
                QString futureID;
                OPTION_TYPE type;
                int exercisePrice;
                if (parseOptionID(optionID, futureID, type, exercisePrice)) {
                    kSet.insert(exercisePrice);
                }
            }
        }

        auto *pPricing = new OptionPricing(0.04, 0.04);
        pPricing->generate(kSet.toList(), s0List, sigmaList, endDate, startDate);
        pricingMap[underlying] = pPricing;
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
    Q_UNUSED(lastPrice)
    Q_UNUSED(volume)

    const DepthMarket latest_market_data = {
        time,
        askPrice1,
        askVolume1,
        bidPrice1,
        bidVolume1
    };

    if (isOption(instrumentID)) {
        // option
        QString futureID;
        OPTION_TYPE type;
        int exercisePrice;
        if (parseOptionID(instrumentID, futureID, type, exercisePrice)) {
            option_market_data[futureID][type][exercisePrice] = latest_market_data;
            findInefficientPrices(futureID, type, exercisePrice);
        }
    } else {
        // future
        future_market_data[instrumentID] = latest_market_data;
        findInefficientPrices(instrumentID);
    }
}

/*!
 * \brief OptionArbitrageur::findInefficientPrices
 * 寻找市场中期权的无效率价格
 *
 * \param futureID 标的期货合约代码
 * \param type 期权类型(看涨/看跌)
 * \param exercisePrice 行权价
 */
void OptionArbitrageur::findInefficientPrices(const QString &futureID, OPTION_TYPE type, int exercisePrice)
{
    const auto threshold = thresholdMap[futureID];

    if (exercisePrice == 0) {
        findCheapCallOptions(futureID, threshold);
        findCheapPutOptions(futureID, threshold);
    } else {
        if (type == CALL_OPT) {
            checkCheapCallOptions(futureID, exercisePrice, threshold);
            findReversedCallOptions(futureID, exercisePrice);
        } else if (type == PUT_OPT) {
            checkCheapPutOptions(futureID, exercisePrice, threshold);
            findReversedPutOptions(futureID, exercisePrice);
        }
    }
}

/*!
 * \brief OptionArbitrageur::findCheapCallOptions
 * 寻找所有权利金小于实值额的看涨期权
 *
 * \param futureID 标的期货合约代码
 * \param threshold 套利阈值
 */
void OptionArbitrageur::findCheapCallOptions(const QString &futureID, double threshold)
{
    auto & callOptionData = option_market_data[futureID][CALL_OPT];
    const auto exercisePrices = callOptionData.keys();
    for (const auto exercisePrice : exercisePrices) {
        checkCheapCallOptions(futureID, exercisePrice, threshold);
    }
}

/*!
 * \brief OptionArbitrageur::checkCheapCallOptions
 * 检查看涨期权定价是否合理
 *
 * \param futureID 标的期货合约代码
 * \param exercisePrice 行权价
 * \param threshold 套利阈值
 */
void OptionArbitrageur::checkCheapCallOptions(const QString &futureID, int exercisePrice, double threshold)
{
    auto & underlying = future_market_data[futureID];
    auto & callOption = option_market_data[futureID][CALL_OPT][exercisePrice];

    const auto liquidity = qMin(callOption.askVolume, underlying.bidVolume);
    auto vol = getVol(liquidity);
    vol = qMin(vol, allowTradeNumber);
    if (vol > 0) {
        auto diff = underlying.bidPrice - exercisePrice;
        if (diff > 0.1 && isTimeCloseEnouogh(underlying, callOption)) {    // 实值期权, diff = 实值额
            auto premium = callOption.askPrice;
            if ((premium + threshold) < diff) {
                pExecuter->buyLimit(makeOptionID(futureID, CALL_OPT, exercisePrice), vol, premium);
                pExecuter->sellLimit(futureID, vol, underlying.bidPrice);

                manageMoney(vol);

                qDebug() << "Found cheap call option";
                qDebug() << futureID; qDebug() << underlying;
                qDebug() << makeOptionID(futureID, CALL_OPT, exercisePrice); qDebug() << callOption;

                makeInvalid(callOption);
                makeInvalid(underlying);
            }
        }
    }
}

/*!
 * \brief OptionArbitrageur::findCheapPutOptions
 * 寻找权利金小于实值额的看跌期权
 *
 * \param futureID 标的期货合约代码
 * \param threshold 套利阈值
 */
void OptionArbitrageur::findCheapPutOptions(const QString &futureID, double threshold)
{
    auto & putOptionMap = option_market_data[futureID][PUT_OPT];
    const auto exercisePrices = putOptionMap.keys();
    for (const auto exercisePrice : exercisePrices) {
        checkCheapPutOptions(futureID, exercisePrice, threshold);
    }
}

/*!
 * \brief OptionArbitrageur::checkCheapPutOptions
 * 检查看跌期权定价是否合理
 *
 * \param futureID 标的期货合约代码
 * \param exercisePrice 行权价
 * \param threshold 套利阈值
 */
void OptionArbitrageur::checkCheapPutOptions(const QString &futureID, int exercisePrice, double threshold)
{
    auto & underlying = future_market_data[futureID];
    auto & putOption = option_market_data[futureID][PUT_OPT][exercisePrice];

    const auto liquidity = qMin(putOption.askVolume, underlying.askVolume);
    auto vol = getVol(liquidity);
    vol = qMin(vol, allowTradeNumber);
    if (vol > 0) {
        auto diff = exercisePrice - underlying.askPrice;
        if (diff > 0.1 && isTimeCloseEnouogh(underlying, putOption)) {    // 实值期权
            auto premium = putOption.askPrice;
            if ((premium + threshold) < diff) {
                pExecuter->buyLimit(makeOptionID(futureID, PUT_OPT, exercisePrice), vol, premium);
                pExecuter->buyLimit(futureID, vol, underlying.askPrice);

                manageMoney(vol);

                qDebug() << "Found cheap put option";
                qDebug() << futureID; qDebug() << underlying;
                qDebug() << makeOptionID(futureID, PUT_OPT, exercisePrice); qDebug() << putOption;

                makeInvalid(putOption);
                makeInvalid(underlying);
            }
        }
    }
}

/*!
 * \brief OptionArbitrageur::findReversedCallOptions
 * 无风险套利七：当到期日相同，行权价较高的看涨期权的权利金价格大于行权价较低的看涨期权权利金价格，买入行权价较低的看涨期权，卖出相同数量行权价较高的看涨期权。
 * 最小无风险收益=行权价较高的看涨期权权利金价格-行权价较低的看涨期权权利金价格。
 * 最大无风险收益=（行权价较高的看涨期权权利金价格-行权价较低的看涨期权权利金价格）+（高行权价-低行权价）。
 *
 * \param futureID 标的期货合约代码
 * \param exercisePriceToCheck 行权价(最新价格有变动的那个期权)
 */
void OptionArbitrageur::findReversedCallOptions(const QString &futureID, int exercisePriceToCheck)
{
    auto & callOptionMap = option_market_data[futureID][CALL_OPT];
    const auto exercisePrices = callOptionMap.keys();
    for (const auto exercisePrice : exercisePrices) {
        auto epdiff = exercisePrice - exercisePriceToCheck;
        if (epdiff == 0) {
            continue;
        } else if (epdiff > 0) {
            checkReversedCallOptions(futureID, callOptionMap, exercisePriceToCheck, exercisePrice);
        } else { /* exercisePrice < exercisePriceToCheck */
            checkReversedCallOptions(futureID, callOptionMap, exercisePrice, exercisePriceToCheck);
        }
    }
}

/*!
 * \brief OptionArbitrageur::checkReversedCallOptions
 * 检查是否高行权价的看涨期权权利金价格大于相同到期日低行权价看涨期权权利金价格
 *
 * \param futureID 标的期货合约代码
 * \param callOptionMap 看涨期权表引用
 * \param lowExercisePrice 低行权价
 * \param highExercisePrice 高行权价
 */
void OptionArbitrageur::checkReversedCallOptions(const QString &futureID, QMap<int, DepthMarket> &callOptionMap, int lowExercisePrice, int highExercisePrice)
{
    const auto liquidity = qMin(callOptionMap[highExercisePrice].bidVolume, callOptionMap[lowExercisePrice].askVolume);
    auto vol = getVol(liquidity);
    vol = qMin(vol, allowTradeNumber);
    if (vol > 0) {
        auto lowPremium = callOptionMap[lowExercisePrice].askPrice;
        auto highPremium = callOptionMap[highExercisePrice].bidPrice;
        auto diff = highPremium - lowPremium;
        if (diff > 1 && isTimeCloseEnouogh(callOptionMap[lowExercisePrice], callOptionMap[highExercisePrice])) {
            pExecuter->buyLimit(makeOptionID(futureID, CALL_OPT, lowExercisePrice), vol, lowPremium);
            pExecuter->sellLimit(makeOptionID(futureID, CALL_OPT, highExercisePrice), vol, highPremium);

            manageMoney(vol);

            qDebug() << "Found reversed call options";
            qDebug() << makeOptionID(futureID, CALL_OPT, lowExercisePrice); qDebug() << callOptionMap[lowExercisePrice];
            qDebug() << makeOptionID(futureID, CALL_OPT, highExercisePrice); qDebug() << callOptionMap[highExercisePrice];

            makeInvalid(callOptionMap[lowExercisePrice]);
            makeInvalid(callOptionMap[highExercisePrice]);
        }
    }
}

/*!
 * \brief OptionArbitrageur::findReversedPutOptions
 * 无风险套利八：当到期日相同，行权价较低的看跌期权的权利金价格大于行权价较高的看跌期权权利金价格，买入行权价较高的看涨期权，卖出相同数量行权价较低的看涨期权。
 * 最小无风险收益=行权价较低的看跌期权权利金价格-行权价较高的看跌期权权利金价格。
 * 最大无风险收益=（行权价较低的看跌期权权利金价格-行权价较高的看跌期权权利金价格）+（高行权价-低行权价）。
 *
 * \param futureID 标的期货合约代码
 * \param exercisePrice 行权价(最新价格有变动的那个期权)
 */
void OptionArbitrageur::findReversedPutOptions(const QString &futureID, int exercisePriceToCheck)
{
    auto & putOptionMap = option_market_data[futureID][PUT_OPT];
    const auto exercisePrices = putOptionMap.keys();
    for (const auto exercisePrice : exercisePrices) {
        auto epdiff = exercisePrice - exercisePriceToCheck;
        if (epdiff == 0) {
            continue;
        } else if (epdiff > 0) {
            checkReversedPutOptions(futureID, putOptionMap, exercisePriceToCheck, exercisePrice);
        } else { /* exercisePrice < exercisePriceToCheck */
            checkReversedPutOptions(futureID, putOptionMap, exercisePrice, exercisePriceToCheck);
        }
    }
}

/*!
 * \brief OptionArbitrageur::checkReversedPutOptions
 * 检查是否低行权价的看涨期权权利金价格大于相同到期日高行权价看涨期权权利金价格
 *
 * \param futureID 标的期货合约代码
 * \param putOptionMap 看跌期权表引用
 * \param lowExercisePrice 低行权价
 * \param highExercisePrice 高行权价
 */
void OptionArbitrageur::checkReversedPutOptions(const QString &futureID, QMap<int, DepthMarket> &putOptionMap, int lowExercisePrice, int highExercisePrice)
{
    const auto liquidity = qMin(putOptionMap[highExercisePrice].askVolume, putOptionMap[lowExercisePrice].bidVolume);
    auto vol = getVol(liquidity);
    vol = qMin(vol, allowTradeNumber);
    if (vol > 0) {
        auto lowPremium = putOptionMap[lowExercisePrice].bidPrice;
        auto highPremium = putOptionMap[highExercisePrice].askPrice;
        auto diff = lowPremium - highPremium;
        if (diff > 1 && isTimeCloseEnouogh(putOptionMap[lowExercisePrice], putOptionMap[highExercisePrice])) {
            pExecuter->buyLimit(makeOptionID(futureID, PUT_OPT, highExercisePrice), vol, highPremium);
            pExecuter->sellLimit(makeOptionID(futureID, PUT_OPT, lowExercisePrice), vol, lowPremium);

            manageMoney(vol);

            qDebug() << "Found reversed put options";
            qDebug() << makeOptionID(futureID, PUT_OPT, highExercisePrice); qDebug() << putOptionMap[highExercisePrice];
            qDebug() << makeOptionID(futureID, PUT_OPT, lowExercisePrice); qDebug() << putOptionMap[lowExercisePrice];

            makeInvalid(putOptionMap[highExercisePrice]);
            makeInvalid(putOptionMap[lowExercisePrice]);
        }
    }
}

void OptionArbitrageur::fishing(const QStringList &options, int vol, double price)
{
    for (const auto &optionID : options) {
        pExecuter->parkBuyLimit(optionID, vol, price);
    }
}

void OptionArbitrageur::manageMoney(int vol)
{
    allowTradeNumber -= vol;
    if (allowTradeNumber <= 0) {
        qDebug() << "Enough trades made! Let's quit!";
        QCoreApplication::quit();
    }
}
