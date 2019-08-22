#include "db_helper.h"

#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

/*!
 * \brief checkAndReopenDbIfNotAlive
 * 检查数据库连接状态, 如果连接已经失效, 断开重连.
 *
 * \return 数据库连接状态, true正常, false不正常.
 */
bool checkAndReopenDbIfNotAlive()
{
    QSqlDatabase sqlDB = QSqlDatabase::database();
    QSqlQuery qry(sqlDB);
    bool ret = qry.exec("SHOW PROCESSLIST");
    if (!ret) {
        qWarning().noquote() << "Execute query failed! Will re-open database!";
        qWarning().noquote() << qry.lastError();
        sqlDB.close();
        if (sqlDB.open()) {
            ret = qry.exec("SHOW PROCESSLIST");
        } else {
            qCritical().noquote() << "Re-open database failed!";
            qCritical().noquote() << qry.lastError();
        }
    }
    if (ret) {
        while (qry.next()) {
            qDebug() << qry.value(0).toLongLong() << qry.value(1).toString();
        }
    }
    return ret;
}

bool createDbIfNotExist(const QString &dbName)
{
    QSqlDatabase sqlDB = QSqlDatabase::database();
    QSqlQuery qry(sqlDB);
    if (!qry.exec("SHOW DATABASES")) {
        qCritical().noquote() << "Show databases failed!";
        qCritical().noquote() << qry.lastError();
        return false;
    }
    QStringList existDbNames;
    while (qry.next()) {
        QString existDbName = qry.value(0).toString();
        existDbNames << existDbName;
    }
    if (!existDbNames.contains(dbName, Qt::CaseInsensitive)) {
        if (!qry.exec("CREATE DATABASE " + dbName)) {
            qCritical().noquote() << "Create database" << dbName << "failed!";
            qCritical().noquote() << qry.lastError();
            return false;
        }
    }
    return true;
}

bool createTablesIfNotExist(const QString &dbName, const QStringList &tableNames, const QString &format)
{
    bool ret = true;
    QSqlDatabase sqlDB = QSqlDatabase::database();
    QSqlQuery qry(sqlDB);
    QString oldDbName = sqlDB.databaseName();
    if (oldDbName.toLower() != dbName.toLower()) {
        sqlDB.close();
        sqlDB.setDatabaseName(dbName);
        sqlDB.open();
    }
    const auto existTables = sqlDB.tables();
    for (const auto &tableToCreate : tableNames) {
        if (!existTables.contains(tableToCreate, Qt::CaseInsensitive)) {
            bool ok = qry.exec(QString("CREATE TABLE %1.%2 %3").arg(dbName, tableToCreate, format));
            if (!ok) {
                ret = false;
                qCritical().noquote().nospace() << "Create table " << dbName << "." << tableToCreate << " failed!";
                qCritical().noquote() << qry.lastError();
                break;
            }
        }
    }

    if (oldDbName.toLower() != dbName.toLower()) {
        sqlDB.close();
        sqlDB.setDatabaseName(oldDbName);
        sqlDB.open();
    }
    return ret;
}
