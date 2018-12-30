#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "message_handler.h"
#include "parked_order.h"
#include "ctp_executer.h"
#include "trade_executer_adaptor.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("trade_executer");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    qDBusRegisterMetaType<ParkedOrder>();
    qDBusRegisterMetaType<QList<ParkedOrder>>();

    QCommandLineParser parser;
    parser.setApplicationDescription("Send trade operations to counter");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({
        {{"f", "logtofile"},
            QCoreApplication::translate("main", "Save log to a file")},
        {{"s", "suffix"},
            QCoreApplication::translate("main", "Suffix for dbus object and service"), "letters or numbers"},
    });

    parser.process(a);
    bool log2File = parser.isSet("logtofile");
    QString suffix = parser.value("suffix");
    setupMessageHandler(true, log2File, "trade_executer");

    QList<CtpExecuter*> executerList;
    for (const auto & config : executerConfigs) {
        CtpExecuter *pExecuter = new CtpExecuter(config);
        new Trade_executerAdaptor(pExecuter);
        QDBusConnection dbus = QDBusConnection::sessionBus();
        dbus.registerObject(config.dbusObject + suffix, pExecuter);
        dbus.registerService(config.dbusService + suffix);
        executerList.append(pExecuter);
    }

    int ret = a.exec();

    for (auto pExecuter : executerList) {
        delete pExecuter;
    }
    restoreMessageHandler();
    return ret;
}
