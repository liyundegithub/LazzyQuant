#include <QApplication>
#include <QCommandLineParser>

#include "config.h"
#include "ctp_replayer.h"
#include "replay_controller.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("ctp_replayer");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Replay controller.");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions(replayRangeOptions);
    parser.process(a);

    TickReplayer *replayer = new CtpReplayer(replayerConfigs[0]);  // Will be deleted in ~ReplayController
    ReplayController controller(replayer);
    controller.setupReplayRange(parser);
    controller.setupDbus(replayerConfigs[0]);

    return a.exec();
}
