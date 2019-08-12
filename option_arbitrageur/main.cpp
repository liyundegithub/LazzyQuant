#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>

#include "config.h"
#include "message_handler.h"
#include "option_arbitrageur_options.h"
#include "option_arbitrageur_dbus.h"
#include "option_arbitrageur.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("option_arbitrageur");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Find arbitrageur opportunity and call executer automatically.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions(optionArbitrageurOptions);
    parser.process(a);

    auto options = getOptionArbitrageurOptions(parser);
    if (options.replayMode && options.updateOnly) {
        qCritical().noquote() << "Can not do update in replay mode!";
        return -1;
    }

    setupMessageHandler(true, options.log2File, "option_arbitrageur", !options.replayMode);
    OptionArbitrageurDbus optionArbitrageurDbus(options);
    int ret = a.exec();
    restoreMessageHandler();

    return ret;
}
