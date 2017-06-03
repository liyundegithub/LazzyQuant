#include <QCoreApplication>

#include "sinyee_replayer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SinYeeReplayer replayer;

    return a.exec();
}
