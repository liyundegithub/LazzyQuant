#include <QApplication>

#include "config.h"
#include "lazzyquantproxy.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("lazzy_controller");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    LazzyQuantProxy proxy;
    MainWindow w(&proxy);
    w.show();
    QObject::connect(&proxy, SIGNAL(newBarFormed(QString,QString)), &w, SLOT(onNewBar(QString,QString)));
    QObject::connect(&proxy, SIGNAL(moduleStatus(bool,bool,bool,bool)), &w, SLOT(onModuleState(bool,bool,bool,bool)));
    return a.exec();
}
