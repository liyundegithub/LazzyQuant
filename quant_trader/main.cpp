#include <QCoreApplication>

#include "config.h"
#include "quant_trader.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("quant_trader");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    QuantTrader quantTrader;
    return a.exec();
}
