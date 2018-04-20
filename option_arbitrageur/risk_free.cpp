#include "depth_market.h"
#include "risk_free.h"

#define TIME_DIFF 120

static inline int getVol(int liquidity)
{
    return liquidity / 2;
}

RiskFree::RiskFree(double threshold, DepthMarketCollection *pDMC) :
    BaseStrategy(pDMC),
    threshold(threshold)
{
    //
}

RiskFree::~RiskFree()
{
    //
}

void RiskFree::onUnderlyingChanged(int underlyingIdx)
{
    findCheapCallOptions(underlyingIdx);
    findCheapPutOptions(underlyingIdx);
}

void RiskFree::onOptionChanged(int underlyingIdx, OPTION_TYPE type, int kIdx)
{
    if (type == CALL_OPT) {
        checkCheapCallOptions(underlyingIdx, kIdx);
        findReversedCallOptions(underlyingIdx, kIdx);
    } else {
        checkCheapPutOptions(underlyingIdx, kIdx);
        findReversedPutOptions(underlyingIdx, kIdx);
    }
}

/*!
 * \brief RiskFree::findCheapCallOptions
 * 寻找所有权利金小于实值额的看涨期权.
 *
 * \param underlyingIdx 标的索引.
 */
void RiskFree::findCheapCallOptions(int underlyingIdx)
{
    const auto kIdxList = pDepthMarkets->getKIdxListByIdx(underlyingIdx);
    for (const auto kIdx : kIdxList) {
        checkCheapCallOptions(underlyingIdx, kIdx);
    }
}

/*!
 * \brief RiskFree::checkCheapCallOptions
 * 检查看涨期权定价是否合理.
 *
 * \param underlyingIdx 标的索引.
 * \param kIdx 行权价索引.
 */
void RiskFree::checkCheapCallOptions(int underlyingIdx, int kIdx)
{
    const auto underlying = pDepthMarkets->getUnderlyingDepthMarketByIdx(underlyingIdx);
    const auto callOption = pDepthMarkets->getOptionDepthMarketByIdx(underlyingIdx, CALL_OPT, kIdx);

    const auto liquidity = qMin(callOption.askVolume, underlying.bidVolume);
    auto vol = getVol(liquidity);
    if (vol > 0) {
        auto diff = underlying.bidPrice - pDepthMarkets->getKByIdx(kIdx);
        if (diff > 0.000001 && isTimeCloseEnouogh(underlying.time, callOption.time, TIME_DIFF)) {    // 实值期权, diff = 实值额
            auto premium = callOption.askPrice;
            if ((premium + threshold) < diff) {
                buyOption(underlyingIdx, CALL_OPT, kIdx, vol, premium);
                sellUnderlying(underlyingIdx, vol, underlying.bidPrice);

                qDebug().nospace() << "Found cheap call option!\n"
                    << pDepthMarkets->getUnderlyingIDByIdx(underlyingIdx) << "\n" << underlying << "\n"
                    << pDepthMarkets->makeOptionByIdx(underlyingIdx, CALL_OPT, kIdx) << "\n" << callOption;

                pDepthMarkets->takeLiquidityByIdx(underlyingIdx, CALL_OPT, kIdx, true);
                pDepthMarkets->takeLiquidityByIdx(underlyingIdx, false);
            }
        }
    }
}

/*!
 * \brief RiskFree::findCheapPutOptions
 * 寻找权利金小于实值额的看跌期权.
 *
 * \param underlyingIdx 标的索引.
 */
void RiskFree::findCheapPutOptions(int underlyingIdx)
{
    const auto kIdxList = pDepthMarkets->getKIdxListByIdx(underlyingIdx);
    for (const auto kIdx : kIdxList) {
        checkCheapPutOptions(underlyingIdx, kIdx);
    }
}

/*!
 * \brief RiskFree::checkCheapPutOptions
 * 检查看跌期权定价是否合理.
 *
 * \param underlyingIdx 标的索引.
 * \param kIdx 行权价索引.
 */
void RiskFree::checkCheapPutOptions(int underlyingIdx, int kIdx)
{
    const auto underlying = pDepthMarkets->getUnderlyingDepthMarketByIdx(underlyingIdx);
    const auto putOption = pDepthMarkets->getOptionDepthMarketByIdx(underlyingIdx, PUT_OPT, kIdx);

    const auto liquidity = qMin(putOption.askVolume, underlying.askVolume);
    auto vol = getVol(liquidity);
    if (vol > 0) {
        auto diff = pDepthMarkets->getKByIdx(kIdx) - underlying.askPrice;
        if (diff > 0.000001 && isTimeCloseEnouogh(underlying.time, putOption.time, TIME_DIFF)) {    // 实值期权.
            auto premium = putOption.askPrice;
            if ((premium + threshold) < diff) {
                buyOption(underlyingIdx, PUT_OPT, kIdx, vol, premium);
                buyUnderlying(underlyingIdx, vol, underlying.askPrice);

                qDebug().nospace() << "Found cheap put option!\n"
                    << pDepthMarkets->getUnderlyingIDByIdx(underlyingIdx) << "\n" << underlying << "\n"
                    << pDepthMarkets->makeOptionByIdx(underlyingIdx, PUT_OPT, kIdx) << "\n" << putOption;

                pDepthMarkets->takeLiquidityByIdx(underlyingIdx, PUT_OPT, kIdx, true);
                pDepthMarkets->takeLiquidityByIdx(underlyingIdx, true);
            }
        }
    }
}

/*!
 * \brief RiskFree::findReversedCallOptions
 * 无风险套利七：当到期日相同，行权价较高的看涨期权的权利金价格大于行权价较低的看涨期权权利金价格，买入行权价较低的看涨期权，卖出相同数量行权价较高的看涨期权.
 * 最小无风险收益=行权价较高的看涨期权权利金价格-行权价较低的看涨期权权利金价格.
 * 最大无风险收益=（行权价较高的看涨期权权利金价格-行权价较低的看涨期权权利金价格）+（高行权价-低行权价）.
 *
 * \param underlyingIdx 标的索引.
 * \param kIdxToCheck 行权价索引(最新价格有变动的那个期权)
 */
void RiskFree::findReversedCallOptions(int underlyingIdx, int kIdxToCheck)
{
    const auto kIdxList = pDepthMarkets->getKIdxListByIdx(underlyingIdx);
    for (const auto kIdx : kIdxList) {
        auto epdiff = kIdx - kIdxToCheck;
        if (epdiff == 0) {
            continue;
        } else if (epdiff > 0) {
            checkReversedCallOptions(underlyingIdx, kIdxToCheck, kIdx);
        } else { /* K < kToCheck */
            checkReversedCallOptions(underlyingIdx, kIdx, kIdxToCheck);
        }
    }
}

/*!
 * \brief RiskFree::checkReversedCallOptions
 * 检查是否高行权价的看涨期权权利金价格大于相同到期日低行权价看涨期权权利金价格.
 *
 * \param underlyingIdx 标的索引.
 * \param lowKIdx 低行权价索引.
 * \param highKIdx 高行权价索引.
 */
void RiskFree::checkReversedCallOptions(int underlyingIdx, int lowKIdx, int highKIdx)
{
    const auto lowKOption = pDepthMarkets->getOptionDepthMarketByIdx(underlyingIdx, CALL_OPT, lowKIdx);
    const auto highKOption = pDepthMarkets->getOptionDepthMarketByIdx(underlyingIdx, CALL_OPT, highKIdx);

    const auto liquidity = qMin(highKOption.bidVolume, lowKOption.askVolume);
    auto vol = getVol(liquidity);
    if (vol > 0) {
        auto lowPremium = lowKOption.askPrice;
        auto highPremium = highKOption.bidPrice;
        auto diff = highPremium - lowPremium;
        if (diff > 1.0 && isTimeCloseEnouogh(lowKOption.time, highKOption.time, TIME_DIFF)) {
            buyOption(underlyingIdx, CALL_OPT, lowKIdx, vol, lowPremium);
            sellOption(underlyingIdx, CALL_OPT, highKIdx, vol, highPremium);

            qDebug().nospace() << "Found reversed call options!\n"
                << pDepthMarkets->makeOptionByIdx(underlyingIdx, CALL_OPT, lowKIdx) << "\n" << lowKOption << "\n"
                << pDepthMarkets->makeOptionByIdx(underlyingIdx, CALL_OPT, highKIdx) << "\n" << highKOption;

            pDepthMarkets->takeLiquidityByIdx(underlyingIdx, CALL_OPT, lowKIdx, true);
            pDepthMarkets->takeLiquidityByIdx(underlyingIdx, CALL_OPT, highKIdx, false);
        }
    }
}

/*!
 * \brief RiskFree::findReversedPutOptions
 * 无风险套利八：当到期日相同，行权价较低的看跌期权的权利金价格大于行权价较高的看跌期权权利金价格，买入行权价较高的看涨期权，卖出相同数量行权价较低的看涨期权.
 * 最小无风险收益=行权价较低的看跌期权权利金价格-行权价较高的看跌期权权利金价格.
 * 最大无风险收益=（行权价较低的看跌期权权利金价格-行权价较高的看跌期权权利金价格）+（高行权价-低行权价）.
 *
 * \param underlyingIdx 标的索引.
 * \param kIdxToCheck 行权价索引(最新价格有变动的那个期权)
 */
void RiskFree::findReversedPutOptions(int underlyingIdx, int kIdxToCheck)
{
    const auto kIdxList = pDepthMarkets->getKIdxListByIdx(underlyingIdx);
    for (const auto kIdx : kIdxList) {
        auto epdiff = kIdx - kIdxToCheck;
        if (epdiff == 0) {
            continue;
        } else if (epdiff > 0) {
            checkReversedPutOptions(underlyingIdx, kIdxToCheck, kIdx);
        } else { /* K < kToCheck */
            checkReversedPutOptions(underlyingIdx, kIdx, kIdxToCheck);
        }
    }
}

/*!
 * \brief RiskFree::checkReversedPutOptions
 * 检查是否低行权价的看跌期权权利金价格大于相同到期日高行权价看跌期权权利金价格.
 *
 * \param underlyingIdx 标的索引.
 * \param lowKIdx 低行权价索引.
 * \param highKIdx 高行权价索引.
 */
void RiskFree::checkReversedPutOptions(int underlyingIdx, int lowKIdx, int highKIdx)
{
    const auto lowKOption = pDepthMarkets->getOptionDepthMarketByIdx(underlyingIdx, PUT_OPT, lowKIdx);
    const auto highKOption = pDepthMarkets->getOptionDepthMarketByIdx(underlyingIdx, PUT_OPT, highKIdx);

    const auto liquidity = qMin(highKOption.askVolume, lowKOption.bidVolume);
    auto vol = getVol(liquidity);
    if (vol > 0) {
        auto lowPremium = lowKOption.bidPrice;
        auto highPremium = highKOption.askPrice;
        auto diff = lowPremium - highPremium;
        if (diff > 1.0 && isTimeCloseEnouogh(lowKOption.time, highKOption.time, TIME_DIFF)) {
            buyOption(underlyingIdx, PUT_OPT, highKIdx, vol, highPremium);
            sellOption(underlyingIdx, PUT_OPT, lowKIdx, vol, lowPremium);

            qDebug().nospace() << "Found reversed put options!\n"
                << pDepthMarkets->makeOptionByIdx(underlyingIdx, PUT_OPT, highKIdx) << "\n" << highKOption << "\n"
                << pDepthMarkets->makeOptionByIdx(underlyingIdx, PUT_OPT, lowKIdx) << "\n" << lowKOption;

            pDepthMarkets->takeLiquidityByIdx(underlyingIdx, PUT_OPT, lowKIdx, false);
            pDepthMarkets->takeLiquidityByIdx(underlyingIdx, PUT_OPT, highKIdx, true);
        }
    }
}
