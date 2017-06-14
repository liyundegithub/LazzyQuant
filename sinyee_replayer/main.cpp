#include <QCoreApplication>

#include "config.h"
#include "sinyee_replayer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SinYeeReplayer replayer(replayerConfigs[0]);

    return a.exec();
}
