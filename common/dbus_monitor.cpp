#include "dbus_monitor.h"

#include <QTimer>
#include <QDBusAbstractInterface>

DBusMonitor::DBusMonitor(const QObjectList &dbusObjList, int interval, QObject *parent) :
    QObject(parent)
{
    for (QObject *obj : dbusObjList) {
        auto *itf = qobject_cast<QDBusAbstractInterface*>(obj);
        if (itf) {
            dbusItfList.append(itf);
        }
    }

    timer = new QTimer(this);
    timer->setInterval(interval);
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer->start();
}

void DBusMonitor::setEnabled(bool b)
{
    if (b) {
        timer->start();
    } else {
        timer->stop();
    }
}

void DBusMonitor::onTimer()
{
    QList<bool> statusList;
    for (QDBusAbstractInterface *itf : qAsConst(dbusItfList)) {
        statusList << itf->isValid();
    }
    emit dbusStatus(statusList);
}
