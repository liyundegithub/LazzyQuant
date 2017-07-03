#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <QMetaObject>

template <typename T> class QList;
class QObject;

class ConnectionManager
{
    QList<QMetaObject::Connection> connections;
public:
    ConnectionManager(const QList<QObject *> &inputs, const QList<QObject *> &strategies);
    ~ConnectionManager();
};

#endif // CONNECTION_MANAGER_H
