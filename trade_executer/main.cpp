#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "message_handler.h"
#include "ctp_executer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("trade_executer");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Send trade operations to counter");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({
        {{"f", "logtofile"},
            QCoreApplication::translate("main", "Save log to a file")},
    });

    parser.process(a);
    bool log2File = parser.isSet("logtofile");
    setupMessageHandler(true, log2File, "trade_executer");

    QList<CtpExecuter*> executerList;
    for (const auto & config : executerConfigs) {
        CtpExecuter *pExecuter = new CtpExecuter(config);
        executerList.append(pExecuter);
    }

    int ret = a.exec();

    for (const auto & pExecuter : executerList) {
        delete pExecuter;
    }
    restoreMessageHandler();
    return ret;
}
