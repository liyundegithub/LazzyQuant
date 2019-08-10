#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <QList>
#include <QObject>
#include <QMetaObject>

class ConnectionManager
{
    QList<QMetaObject::Connection> connections;

public:
    ConnectionManager(const QObjectList &inputs, const QObjectList &strategies);
    ~ConnectionManager();
};

#endif // CONNECTION_MANAGER_H
