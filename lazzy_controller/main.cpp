#include <QApplication>

#include "config.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("lazzy_controller");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    MainWindow w;
    w.show();
    return a.exec();
}
