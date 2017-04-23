#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "option_arbitrageur.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("option_arbitrageur");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Find arbitrageur opportunity and call executer automatically.");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({
        // Number allows to trade (-n)
        {{"n", "number"},
            QCoreApplication::translate("main", "Number allows to trade")},
    });

    parser.process(a);
    bool ok;
    int allowTradeNumber = parser.value("number").toInt(&ok);
    if (!ok || allowTradeNumber <= 0) {
        allowTradeNumber = 1;
    }

    OptionArbitrageur arbitrageur(allowTradeNumber);
    return a.exec();
}
