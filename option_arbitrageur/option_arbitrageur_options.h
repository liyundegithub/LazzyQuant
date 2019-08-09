#ifndef OPTION_ARBITRAGEUR_OPTIONS_H
#define OPTION_ARBITRAGEUR_OPTIONS_H

#include <QString>
#include <QList>
#include <QCommandLineOption>
#include <QCommandLineParser>

const QList<QCommandLineOption> optionArbitrageurOptions = {
    {{"r", "replay"}, "Replay on a specified date", "yyyyMMdd"},
    {{"u", "update"}, "Update subscribe list, should not be used with -r"},
    {{"f", "logtofile"}, "Save log to a file"},
};

struct OptionArbitrageurOptions {
    bool replayMode;
    QString replayDate;
    bool update;
    bool log2File;
};

OptionArbitrageurOptions getOptionArbitrageurOptions(const QCommandLineParser &parser)
{
    OptionArbitrageurOptions options;
    options.replayMode = parser.isSet("replay");
    options.replayDate = parser.value("replay");
    options.update = parser.isSet("update");
    options.log2File = parser.isSet("logtofile");
    return options;
}

#endif // OPTION_ARBITRAGEUR_OPTIONS_H
