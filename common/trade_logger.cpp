#include "trade_logger.h"
#include "db_helper.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

TradeLogger::TradeLogger(const QString &name):
    name(name)
{
    createDbIfNotExist("tradelog");
    createTablesIfNotExist("tradelog", {name}, " (time BIGINT NULL, instrument VARCHAR(45) NULL, price FLOAT NULL, volume INT NULL, direction BOOLEAN NULL, opencloseflag BOOLEAN NULL)");
}

void TradeLogger::positionChanged(qint64 actionTime, const QString &instrumentID, int newPosition, double price)
{
    int oldPosition = positionMap[instrumentID];
    if (newPosition == oldPosition) {
        return;
    }

    if (newPosition >= 0 && oldPosition < 0) {
        closeShort(actionTime, instrumentID, price, -oldPosition);
        if (oldPosition != 0) {
            return;
        }
    } else if (newPosition <= 0 && oldPosition > 0) {
        closeLong(actionTime, instrumentID, price, oldPosition);
        if (oldPosition != 0) {
            return;
        }
    }

    int diff = newPosition - oldPosition;

    if (diff < 0 && newPosition > 0) {
        closeLong(actionTime, instrumentID, price, -diff);
    } else if (0 >= oldPosition && diff < 0) {
        openShort(actionTime, instrumentID, price, -diff);
    } else if (0 <= oldPosition && diff > 0) {
        openLong(actionTime, instrumentID, price, diff);
    } else if (diff > 0 && newPosition < 0) {
        closeShort(actionTime, instrumentID, price, diff);
    }
    positionMap[instrumentID] = newPosition;
}

void TradeLogger::openLong(qint64 actionTime, const QString &instrumentID, double price, int volume)
{
    saveActionToDB(actionTime, instrumentID, price, volume, true, true);
}

void TradeLogger::openShort(qint64 actionTime, const QString &instrumentID, double price, int volume)
{
    saveActionToDB(actionTime, instrumentID, price, volume, false, true);
}

void TradeLogger::closeLong(qint64 actionTime, const QString &instrumentID, double price, int volume)
{
    saveActionToDB(actionTime, instrumentID, price, volume, false, false);
}

void TradeLogger::closeShort(qint64 actionTime, const QString &instrumentID, double price, int volume)
{
    saveActionToDB(actionTime, instrumentID, price, volume, true, false);
}

void TradeLogger::saveActionToDB(qint64 actionTime, const QString &instrumentID, double price, int volume, bool direction, bool openCloseFlag)
{
    QSqlDatabase sqlDB = QSqlDatabase();
    QSqlQuery qry(sqlDB);
    QString tableOfDB = QString("tradelog.%1").arg(name);
    qry.prepare("INSERT INTO " + tableOfDB + " (time, instrument, price, volume, direction, opencloseflag) "
                                             "VALUES (?, ?, ?, ?, ?, ?)");
    qry.bindValue(0, actionTime);
    qry.bindValue(1, instrumentID);
    qry.bindValue(2, price);
    qry.bindValue(3, volume);
    qry.bindValue(4, direction);
    qry.bindValue(5, openCloseFlag);
    bool ok = qry.exec();
    if (!ok) {
        qCritical().noquote() << "Insert action into" << tableOfDB << "failed!";
        qCritical().noquote() << qry.lastError();
    }
}
