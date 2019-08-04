#ifndef REPLAY_CONTROLLER_H
#define REPLAY_CONTROLLER_H

#include <QList>
#include <QDateTime>
#include <QCommandLineOption>

class QCommandLineParser;

class CommonReplayer;
class ControlWidget;
struct CONFIG_ITEM;

class ReplayController {
    QObject *replayerObj;
    ControlWidget *controlWidget;

public:
    explicit ReplayController(CommonReplayer *replayer);
    ~ReplayController();

    void setupReplayRange(QCommandLineParser &parser);
    void setupDbus(const CONFIG_ITEM &config);

private:
    QDateTime getReplayDateTime(QCommandLineParser &parser, const QString &option);

};

const QList<QCommandLineOption> replayRangeOptions = {
    {{"r", "start"}, "Replay start date", "yyyyMMddHHmmss"},
    {{"o", "stop"},  "Replay stop date",  "yyyyMMddHHmmss"},
};


#endif // REPLAY_CONTROLLER_H
