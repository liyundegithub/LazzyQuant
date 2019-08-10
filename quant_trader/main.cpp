#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "message_handler.h"
#include "quant_trader_options.h"
#include "quant_trader_dbus.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("quant_trader");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Collect K lines, calculate indicators/strategies and call TradeExecuter automatically.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions(quantTraderOptions);
    parser.process(a);

    auto options = getQuantTraderOptions(parser);
    setupMessageHandler(true, options.log2File, "quant_trader", !options.replayMode);
    QuantTraderDbus quantTraderDbus(options);
    int ret = a.exec();
    restoreMessageHandler();

    return ret;
}
