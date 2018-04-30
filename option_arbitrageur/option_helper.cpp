#include "option_helper.h"
#include "depth_market.h"

#include <QDebug>

OptionHelper::OptionHelper(QObject *pExecuter) :
    pExecuter(pExecuter)
{
}

QDate OptionHelper::getEndDate(const QString &underlying)
{
    if (pExecuter) {
        QString executerStatus;
        pExecuter->metaObject()->invokeMethod(pExecuter, "getStatus", Q_RETURN_ARG(QString, executerStatus));
        if (executerStatus == "Ready") {
            QString dateStr;
            pExecuter->metaObject()->invokeMethod(pExecuter, "getExpireDate", Q_RETURN_ARG(QString, dateStr), Q_ARG(QString, underlying));
            if (dateStr != INVALID_DATE_STRING) {
                return QDate::fromString(dateStr, QStringLiteral("yyyyMMdd"));
            }
        }
    }
    return getExpireDate(underlying);
}

/*!
 * \brief OptionHelper::getExpireDate
 * 按照合约规则计算期权到期日
 *
 * \param instrumentID 期权合约代码或标的合约代码
 * \return 到期日
 */
QDate OptionHelper::getExpireDate(const QString &instrumentID)
{
    if (instrumentID.startsWith("SR")) {
        const auto y_s = instrumentID.mid(2, 1);
        const auto m_s = instrumentID.mid(3, 2);
        const auto year = y_s.toInt() + 2010;   // FIXME after year 2020
        const auto month = m_s.toInt();

        auto firstDayOfLastMonth = QDate(year, month, 1);
        auto firstDayOfPrevMonth = firstDayOfLastMonth.addMonths(-1);
        auto firstDayOfPrev2Month = firstDayOfLastMonth.addMonths(-2);
        auto lastDayOfPrev2Month = firstDayOfPrevMonth.addDays(-1);

        int sum = 0;
        for (QDate date = lastDayOfPrev2Month; date > firstDayOfPrev2Month; date = date.addDays(-1)) {
            if (tradingCalendar.isTradingDay(date)) {
                sum ++;
                if (sum == 5) {
                    return date;
                }
            }
        }
    } else if (instrumentID.startsWith("m")) {
        const auto y_s = instrumentID.mid(1, 2);
        const auto m_s = instrumentID.mid(3, 2);
        const auto year = y_s.toInt() + 2000;
        const auto month = m_s.toInt();

        auto firstDayOfLastMonth = QDate(year, month, 1);
        auto firstDayOfPrevMonth = firstDayOfLastMonth.addMonths(-1);

        int sum = 0;
        for (QDate date = firstDayOfPrevMonth; date < firstDayOfLastMonth; date = date.addDays(1)) {
            if (tradingCalendar.isTradingDay(date)) {
                sum ++;
                if (sum == 5) {
                    return date;
                }
            }
        }
    }

    qCritical() << "Should never see this!";
    return QDate();
}

int OptionHelper::getOptionTradingDays(const QString &instrumentID, const QDate &startDate)
{
    // FIXME 应该获取期权而非期货的ExpireDate
    const auto endDate = getExpireDate(instrumentID);
    return tradingCalendar.getTradingDays(startDate, endDate);
}

/*!
 * \brief hasSensibleQuote
 * 判断期权合约是否有合理的报价
 *
 * \param optionID 期权合约代码
 * \param md 市场报价信息
 * \return 市场报价是否合理
 */
bool hasSensibleQuote(const QString &optionID, const DepthMarket &md)
{
    double maxSpread = 0.0000001;
    if (optionID.startsWith("m1")) {
        /*
         * 期权合约没有买入委托，但存在报价数量不小于10手且卖出报价小于等于2元/吨的委托时，不接受询价。
         */
        if (md.bidVolume < 0.01 && md.askVolume > 0 && md.askPrice < 2.01) {
            return true;
        }
        /*
         期权合约存在报价数量不小于10手且报价价差（单位：元/吨）满足如下要求的委托时，不接受询价：
        （1）当0＜买入报价＜50，最大买卖价差=Max{买入报价的16%,6}；
        （2）当50≤买入报价＜400，最大买卖价差=Max{买入报价的12%,8}；
        （3）当400≤买入报价，最大买卖价差=Max{买入报价的10%,48}。
        */
        if (0.0 < md.bidPrice && md.bidPrice < 50.0) {
            maxSpread += qMax(md.bidPrice * 0.16, 6.0);
        } else if (50.0 <= md.bidPrice && md.bidPrice < 400.0) {
            maxSpread += qMax(md.bidPrice * 0.12, 8.0);
        } else if (400.0 <= md.bidPrice) {
            maxSpread += qMax(md.bidPrice * 0.10, 48.0);
        } else {
            // Error
        }
    } else if (optionID.startsWith("SR")) {
        if (md.bidPrice < 50.0) {
            maxSpread += 8.0;
        } else if (50.0 <= md.bidPrice && md.bidPrice < 100.0) {
            maxSpread += 10.0;
        } else if (100.0 <= md.bidPrice && md.bidPrice < 200.0) {
            maxSpread += 20.0;
        } else if (200.0 <= md.bidPrice && md.bidPrice < 300.0) {
            maxSpread += 30.0;
        } else if (300.0 <= md.bidPrice && md.bidPrice < 500.0) {
            maxSpread += 50.0;
        } else if (500.0 <= md.bidPrice) {
            maxSpread += 75.0;
        } else {
            // Error
        }
    }

    return md.askVolume > 0.01 && md.bidVolume > 0.01 && (md.askPrice - md.bidPrice) < maxSpread;
}

/*!
 * \brief getOptionMargin
 * 计算期权卖方交纳交易保证金，交易保证金的收取标准为下列两者中较大者：
 * （1）期权合约结算价×标的期货合约交易单位+标的期货合约交易保证金－（1/2）×期权虚值额；
 * （2）期权合约结算价×标的期货合约交易单位+（1/2）×标的期货合约交易保证金。
 *  期权虚值额计算如下：
 * 看涨期权虚值额：Max（期权合约行权价格-标的期货合约结算价，0）×标的期货合约交易单位；
 * 看跌期权虚值额：Max（标的期货合约结算价-期权合约行权价格）×标的期货合约交易单位
 *
 * 本函数适用于白糖和豆粕期货期权
 *
 * \param optionSettlement      期权结算价
 * \param underlyingSettlement  标的结算价
 * \param underlyingPrice       标的价格
 * \param marginRatio           标的合约保证金率
 * \param type                  看涨/看跌
 * \param K                     行权价
 * \param multiplier            合约乘数
 * \return                      期权卖方交纳交易保证金
 */
double getCommodityOptionMargin(double optionSettlement, double underlyingSettlement, double underlyingPrice, double marginRatio, OPTION_TYPE type, double K, int multiplier)
{
    double v = 0.0;
    if (type == CALL_OPT) {
        v = qMax(K - underlyingSettlement, 0.0) * multiplier;
    } else {
        v = qMax(underlyingSettlement - K, 0.0) * multiplier;
    }

    double m1 = underlyingPrice * marginRatio * multiplier - 0.5 * v;
    double m2 = underlyingPrice * marginRatio * multiplier * 0.5;

    return optionSettlement * multiplier + qMax(m1, m2);
}
