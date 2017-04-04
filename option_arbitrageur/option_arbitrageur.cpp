#include <QMap>
#include <QTimer>
#include <QSettings>
#include <QStringList>

#include "config.h"
#include "market.h"
#include "option_arbitrageur.h"

extern QList<Market> markets;

OptionArbitrageur::OptionArbitrageur(QObject *parent) :
    QObject(parent)
{
    loadCommonMarketData();
    loadOptionArbitrageurSettings();

    pExecuter = new com::lazzyquant::trade_executer(EXECUTER_DBUS_SERVICE, EXECUTER_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
    pWatcher = new com::lazzyquant::market_watcher(WATCHER_DBUS_SERVICE, WATCHER_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
    connect(pWatcher, SIGNAL(newMarketData(QString, uint, double, int, double, int, double, int, double, int, double, int)), this, SLOT(onMarketData(QString, uint, double, int, double, int, double, int, double, int, double, int)));

    QTimer::singleShot(1000, [=]() -> void { pExecuter->updateInstrumentsCache(QStringList{"m1"}); }); // FIXME 白糖以及其它
    QTimer::singleShot(5000, [=]() -> void {    // FIXME 4秒可能不够
        const QStringList subscribedInstruments = pWatcher->getSubscribeList();
        const QStringList cachedInstruments = pExecuter->getCachedInstruments();

        QStringList instrumentsToSubscribe;
        for (const auto &item : cachedInstruments) {
            if (!subscribedInstruments.contains(item)) {
                instrumentsToSubscribe << item;
            }
        }
        pWatcher->subscribeInstruments(instrumentsToSubscribe);
    });
}

OptionArbitrageur::~OptionArbitrageur()
{
    qDebug() << "~OptionArbitrageur";
}

void OptionArbitrageur::loadOptionArbitrageurSettings()
{
    QSettings watcherSettings(QSettings::IniFormat, QSettings::UserScope, ORGANIZATION, WATCHER_NAME);
    watcherSettings.beginGroup("SubscribeList");
    QStringList subscribeList = watcherSettings.childKeys();
    foreach (const QString &key, subscribeList) {
        if (!isOption(key) < 8 && watcherSettings.value(key).toBool()) {
            subscribeFutureIDs.insert(key);
        }
    }
    watcherSettings.endGroup();


    QSettings arbitrageurSettings(QSettings::IniFormat, QSettings::UserScope, ORGANIZATION, "option_arbitrageur");

    threshold = arbitrageurSettings.value("threshold", 1.0f).toDouble();
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
 * \param askPrice2  卖二价
 * \param askVolume2 卖二量
 * \param bidPrice2  买二价
 * \param bidVolume2 买二量
 */
void OptionArbitrageur::onMarketData(const QString& instrumentID, uint time, double lastPrice, int volume,
                                     double askPrice1, int askVolume1, double bidPrice1, int bidVolume1,
                                     double askPrice2, int askVolume2, double bidPrice2, int bidVolume2)
{
    Q_UNUSED(time)
    Q_UNUSED(lastPrice)
    Q_UNUSED(volume)

    const DepthMarket latest_market_data = {
        {askPrice1,  askPrice2 },
        {askVolume1, askVolume2},
        {bidPrice1,  bidPrice2 },
        {bidVolume1, bidVolume2}
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
 * \param futureID 期货合约代码
 */
void OptionArbitrageur::findInefficientPrices(const QString &futureID, OPTION_TYPE type, int exercisePrice)
{
    if (exercisePrice == 0) {
        findCheapCallOptions(futureID);
        findCheapPutOptions(futureID);
    } else {
        if (type == CALL_OPT) {
            checkCheapCallOptions(futureID, exercisePrice);
            findReversedCallOptions(futureID, exercisePrice);
        } else if (type == PUT_OPT) {
            checkCheapPutOptions(futureID, exercisePrice);
            findReversedPutOptions(futureID, exercisePrice);
        }
    }
}

/*!
 * \brief OptionArbitrageur::findCheapCallOptions
 * 寻找所有权利金小于实值额的看涨期权
 *
 * \param futureID 期货合约代码
 */
void OptionArbitrageur::findCheapCallOptions(const QString &futureID)
{
    const auto & callOptionData = option_market_data[futureID][CALL_OPT];
    const auto exercisePrices = callOptionData.keys();
    for (const auto exercisePrice : exercisePrices) {
        checkCheapCallOptions(futureID, exercisePrice);
    }
}

/*!
 * \brief OptionArbitrageur::checkCheapCallOptions
 * 检查看涨期权定价是否合理
 *
 * \param futureID 期货合约代码
 * \param exercisePrice 行权价
 */
void OptionArbitrageur::checkCheapCallOptions(const QString &futureID, int exercisePrice)
{
    const auto & callOptionMap = option_market_data[futureID][CALL_OPT];
    if (callOptionMap[exercisePrice].askVolumes[1] > 0 && future_market_data[futureID].bidVolumes[1] > 0) { // 保证至少两档流动性
        auto diff = future_market_data[futureID].bidPrices[1] - exercisePrice;
        if (diff > 0) {    // 实值期权, diff = 实值额
            auto premium = callOptionMap[exercisePrice].askPrices[1];
            if ((premium + 2 + 3) < diff) {    // (权利金 + 手续费) < 实值额
                // Found cheap call option
                pExecuter->buyLimit(makeOptionID(futureID, CALL_OPT, exercisePrice), 1, premium, 3);
                pExecuter->sellLimit(futureID, 1, future_market_data[futureID].bidPrices[1], 3);
            }
        }
    }
}

/*!
 * \brief OptionArbitrageur::findCheapPutOptions
 * 寻找权利金小于实值额的看跌期权
 *
 * \param futureID 期货合约代码
 */
void OptionArbitrageur::findCheapPutOptions(const QString &futureID)
{
    const auto & putOptionMap = option_market_data[futureID][PUT_OPT];
    const auto exercisePrices = putOptionMap.keys();
    for (const auto exercisePrice : exercisePrices) {
        checkCheapPutOptions(futureID, exercisePrice);
    }
}

/*!
 * \brief OptionArbitrageur::checkCheapPutOptions
 * 检查看跌期权定价是否合理
 *
 * \param futureID 期货合约代码
 * \param exercisePrice 行权价
 */
void OptionArbitrageur::checkCheapPutOptions(const QString &futureID, int exercisePrice)
{
    const auto & putOptionMap = option_market_data[futureID][PUT_OPT];
    if (putOptionMap[exercisePrice].askVolumes[1] > 0 && future_market_data[futureID].askVolumes[1] > 0) {  // 保证至少两档流动性
        auto diff = exercisePrice - future_market_data[futureID].askPrices[1];
        if (diff > 0) {    // 实值期权
            auto premium = putOptionMap[exercisePrice].askPrices[1];
            if ((premium + 2 + 3) < diff) {     // (权利金 + 手续费) < 实值额
                // Found cheap put option
                pExecuter->buyLimit(makeOptionID(futureID, PUT_OPT, exercisePrice), 1, premium, 3);
                pExecuter->buyLimit(futureID, 1, future_market_data[futureID].askPrices[1], 3);
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
    const auto & callOptionMap = option_market_data[futureID][CALL_OPT];
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
 * \param callOptionMap 看涨期权表引用
 * \param lowExercisePrice 低行权价
 * \param highExercisePrice 高行权价
 */
void OptionArbitrageur::checkReversedCallOptions(const QString &futureID, const QMap<int, DepthMarket> &callOptionMap, int lowExercisePrice, int highExercisePrice)
{
    if (callOptionMap[highExercisePrice].bidVolumes[1] > 0 && callOptionMap[lowExercisePrice].askVolumes[1] > 0) {  // 保证至少两档流动性
        auto lowPremium = callOptionMap[lowExercisePrice].askPrices[1];
        auto highPremium = callOptionMap[highExercisePrice].bidPrices[1];
        auto diff = highPremium - lowPremium;
        if (diff > 2) {
            // Found
            pExecuter->buyLimit(makeOptionID(futureID, CALL_OPT, lowExercisePrice), 1, lowPremium, 3);
            pExecuter->sellLimit(makeOptionID(futureID, CALL_OPT, highExercisePrice), 1, highPremium, 3);
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
    const auto & putOptionMap = option_market_data[futureID][PUT_OPT];
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
 * \param putOptionMap 看跌期权表引用
 * \param lowExercisePrice 低行权价
 * \param highExercisePrice 高行权价
 */
void OptionArbitrageur::checkReversedPutOptions(const QString &futureID, const QMap<int, DepthMarket> &putOptionMap, int lowExercisePrice, int highExercisePrice)
{
    if (putOptionMap[highExercisePrice].askVolumes[1] > 0 && putOptionMap[lowExercisePrice].bidVolumes[1] > 0) {  // 保证至少两档流动性
        auto lowPremium = putOptionMap[lowExercisePrice].bidPrices[1];
        auto highPremium = putOptionMap[highExercisePrice].askPrices[1];
        auto diff = lowPremium - highPremium;
        if (diff > 2) {
            // Found
            pExecuter->buyLimit(makeOptionID(futureID, PUT_OPT, highExercisePrice), 1, highPremium, 3);
            pExecuter->sellLimit(makeOptionID(futureID, PUT_OPT, lowExercisePrice), 1, lowPremium, 3);
        }
    }
}

/*!
 * \brief OptionArbitrageur::findCheapStrangles
 * 9、买入看涨期权，买入行权价更高的看跌期权
 * 无风险套利九：当到期日相同，行权价较高的看跌期权与行权价较低的看涨期权的权利金价格之和，小于两者行权价之差时，买入行权价较低的看涨期权，买入相同数量行权价较高的看跌期权。
 * 无风险收益=（高行权价－低行权价）－（看涨期权权利金价格+看跌期权权利金价格）
 * 例如，假设CF505C13200的权利金价格为170，CF505P13600的权利金价格为190，符合上述条件，那么买入1手CF505C13200，买入1手CF505P13600，同时行权，收益=（CF505P13600行权价- CF505C13200行权价）-（CF505P13600 权利金价格+CF505C13200权利金价格）=（13600-13200）+（170+190）=40，再乘以5等于200。
 * 此种套利出现的概率很低，临近期权到期日时有可能会出现。
 *
 * \param futureID 期货合约代码
 */
void OptionArbitrageur::findCheapStrangles(const QString &futureID)
{
    // TODO
}
