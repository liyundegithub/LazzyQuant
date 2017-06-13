#include <QCoreApplication>

#include "config.h"
#include "sinyee_replayer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SinYeeReplayer replayer(replayerConfig);

    return a.exec();
}
