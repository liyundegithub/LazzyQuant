#include <QApplication>

#include "config.h"
#include "quant_trader_manager_dbus.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("lazzy_controller");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    QuantTraderManagerDbus manager;
    MainWindow w(&manager);
    w.show();
    return a.exec();
}
