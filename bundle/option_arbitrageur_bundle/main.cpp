#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "market.h"
#include "message_handler.h"
#include "parked_order.h"
#include "option_arbitrageur_options.h"
#include "option_arbitrageur_bundle.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("option_arbitrageur_bundle");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    loadCommonMarketData();
    qMetaTypeId<ParkedOrder>();

    QCommandLineParser parser;
    parser.setApplicationDescription("Option arbitrageur bundled with market watcher, tick replayer and trade executer.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions(optionArbitrageurOptions);

    parser.process(a);
    OptionArbitrageurOptions options = getOptionArbitrageurOptions(parser);
    QString source = parser.value("source");

    setupMessageHandler(true, options.log2File, "option_arbitrageur_bundle", !options.replayMode);
    OptionArbitrageurBundle bundle(options);
    int ret = a.exec();
    restoreMessageHandler();

    return ret;
}
