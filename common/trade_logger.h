#ifndef TRADE_LOGGER_H
#define TRADE_LOGGER_H

#include <QMap>

class TradeLogger
{
    QString name;
    QMap<QString, int> positionMap;

public:
    explicit TradeLogger(const QString &name);

    void positionChanged(int actionTime, const QString &instrumentID, int newPosition, double price);
    void openLong(int actionTime, const QString &instrumentID, double price, int volume);
    void openShort(int actionTime, const QString &instrumentID, double price, int volume);
    void closeLong(int actionTime, const QString &instrumentID, double price, int volume);
    void closeShort(int actionTime, const QString &instrumentID, double price, int volume);

protected:
    void saveActionToDB(int actionTime, const QString &instrumentID, double price, int volume, bool direction, bool openCloseFlag);
};

#endif // TRADE_LOGGER_H
