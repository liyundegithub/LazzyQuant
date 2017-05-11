#ifndef OPTION_HELPER_H
#define OPTION_HELPER_H

#include "utility.h"

#include <QDate>

QDate getExpireDate(const QString &instrumentID);
int getOptionTradingDays(const QString &instrumentID, const QDate &startDate = QDate::currentDate());

double getCommodityOptionMargin(double optionSettlement, double underlyingSettlement, double underlyingPrice, double marginRatio, OPTION_TYPE type, double K, int multiplier);

#endif // OPTION_HELPER_H
