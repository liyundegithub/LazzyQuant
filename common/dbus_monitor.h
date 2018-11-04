#ifndef DBUS_MONITOR_H
#define DBUS_MONITOR_H

#include <QObject>
#include <QList>

class QDBusAbstractInterface;

class DBusMonitor : public QObject
{
    Q_OBJECT

    QList<QDBusAbstractInterface*> dbusItfList;

public:
    explicit DBusMonitor(const QList<QObject*> &dbusObjList, int interval = 1000, QObject *parent = nullptr);

signals:
    void dbusStatus(QList<bool>);

private slots:
    void onTimer();
};

#endif // DBUS_MONITOR_H
