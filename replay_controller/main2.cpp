#include <QApplication>

#include "config.h"
#include "sinyee_replayer.h"
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

    return a.exec();
}
