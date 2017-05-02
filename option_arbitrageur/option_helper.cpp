#include "option_helper.h"
#include "trading_calendar.h"

#include <QDebug>

extern TradingCalendar tradingCalendar;

QDate getExpireDate(const QString &instrumentID)
{
    const int len = instrumentID.length();

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
    return tradingCalendar.getTradingDays(endDate, startDate);
}
