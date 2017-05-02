#ifndef OPTION_HELPER_H
#define OPTION_HELPER_H

#include <QDate>

QDate getExpireDate(const QString &instrumentID);
int getOptionTradingDays(const QString &instrumentID, const QDate &startDate = QDate::currentDate());

#endif // OPTION_HELPER_H
