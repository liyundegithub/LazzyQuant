#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <QList>

class QObject;

class ConnectionManager
{
    QList<QObject *> senders;
public:
    ConnectionManager(const QList<QObject *> &inputs, const QList<QObject *> &strategies);
    ~ConnectionManager();
};

#endif // CONNECTION_MANAGER_H
