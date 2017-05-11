#include "option_helper.h"
#include "trading_calendar.h"

#include <QDebug>

extern TradingCalendar tradingCalendar;

/*!
 * \brief getExpireDate
 * 按照合约规则计算期权到期日
 *
 * \param instrumentID 期权合约代码或标的合约代码
 * \return 到期日
 */
QDate getExpireDate(const QString &instrumentID)
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

    qDebug() << "Should never see this!";
    return QDate();
}

int getOptionTradingDays(const QString &instrumentID, const QDate &startDate)
{
    const auto endDate = getExpireDate(instrumentID);
    return tradingCalendar.getTradingDays(startDate, endDate);
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
