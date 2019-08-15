#ifndef OPTION_ARBITRAGEUR_OPTIONS_H
#define OPTION_ARBITRAGEUR_OPTIONS_H

#include <QString>
#include <QList>
#include <QCommandLineOption>
#include <QCommandLineParser>

const QList<QCommandLineOption> optionArbitrageurOptions = {
    {{"r", "replay"},       "Replay mode"},
    {{"s", "start"},        "Replay start date", "yyyyMMdd"},
    {{"e", "stop"},         "Replay stop date", "yyyyMMdd"},
    {{"u", "updateonly"},   "Update subscribe list only, don't trade, should not be used with -r"},
    {{"f", "logtofile"},    "Save log to a file"},
};

struct OptionArbitrageurOptions {
    bool replayMode;
    QString replayStartDate;
    QString replayStopDate;
    bool updateOnly;
    bool log2File;

    bool isReplayReady() const {
        return replayMode && replayStartDate.length() == 8 && replayStopDate.length() == 8;
    }
};

static OptionArbitrageurOptions getOptionArbitrageurOptions(const QCommandLineParser &parser)
{
    OptionArbitrageurOptions options;
    options.replayMode = parser.isSet("replay");
    options.replayStartDate = parser.value("start");
    options.replayStopDate = parser.value("stop");
    options.updateOnly = parser.isSet("updateonly");
    options.log2File = parser.isSet("logtofile");
    return options;
}

#endif // OPTION_ARBITRAGEUR_OPTIONS_H
