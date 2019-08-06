#include <QCoreApplication>

#include "config.h"
#include "ctp_replayer.h"
#include "tick_replayer_adaptor.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("ctp_replayer");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    CtpReplayer replayer(replayerConfigs[0]);

    new Tick_replayerAdaptor(&replayer);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(replayerConfigs[0].dbusObject, &replayer);
    dbus.registerService(replayerConfigs[0].dbusService);

    return a.exec();
}
