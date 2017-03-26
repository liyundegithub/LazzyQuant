#ifndef UTILITY_H
#define UTILITY_H

enum OPTION_DIR {
    CALL_OPT,
    PUT_OPT,
};

class QString;

QString getInstrumentName(const QString &instrumentID);
QString getFutureIDFromOptionID(const QString &instrumentID);
QString makeOptionID(const QString &futureID, const OPTION_DIR dir, const int exercisePrice);

#endif // UTILITY_H
