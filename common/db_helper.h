#ifndef DB_HELPER_H
#define DB_HELPER_H

class QString;
class QStringList;

bool checkAndReopenDbIfNotAlive();
bool createDbIfNotExist(const QString &dbName);
bool createTablesIfNotExist(const QString &dbName, const QStringList &tableNames, const QString &format);

#endif // DB_HELPER_H
