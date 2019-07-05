#include <QApplication>

#include "config.h"
#include "sinyee_replayer.h"
#include "tick_replayer_adaptor.h"
#include "widget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("sinyee_replayer");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    SinYeeReplayer replayer(replayerConfigs[0]);
    Widget w(&replayer);
    w.show();

    new Tick_replayerAdaptor(&replayer);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(replayerConfigs[0].dbusObject, &replayer);
    dbus.registerService(replayerConfigs[0].dbusService);

    return a.exec();
}
