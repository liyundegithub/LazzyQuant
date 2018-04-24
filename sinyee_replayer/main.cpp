#include <QCoreApplication>

#include "config.h"
#include "sinyee_replayer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("sinyee_replayer");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    SinYeeReplayer replayer(replayerConfigs[0]);

    return a.exec();
}
