#include "dbus_monitor.h"

#include <QTimer>
#include <QDBusAbstractInterface>

DBusMonitor::DBusMonitor(const QList<QObject*> &dbusObjList, int interval, QObject *parent) :
    QObject(parent)
{
    for (QObject *obj : dbusObjList) {
        QDBusAbstractInterface *itf = dynamic_cast<QDBusAbstractInterface*>(obj);
        if (itf) {
            dbusItfList.append(itf);
        }
    }

    QTimer *timer = new QTimer(this);
    timer->setInterval(interval);
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer->start();
}

void DBusMonitor::onTimer()
{
    QList<bool> statusList;
    for (QDBusAbstractInterface *itf : qAsConst(dbusItfList)) {
        statusList << itf->isValid();
    }
    emit dbusStatus(statusList);
}
