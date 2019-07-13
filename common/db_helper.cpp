#include "db_helper.h"

#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

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
    int ret = true;
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
