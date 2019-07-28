#include "config_struct.h"
#include "common_replayer.h"
#include "widget.h"
#include "replay_controller.h"

#include "tick_replayer_adaptor.h"

#include <QTimeZone>
#include <QCommandLineParser>

ReplayController::ReplayController(CommonReplayer *replayer)
{
    widget = new Widget(replayer);
    widget->show();
    this->replayerObj = replayer;
}

ReplayController::~ReplayController()
{
    delete widget;
    delete replayerObj;
}

void ReplayController::setupReplayRange(QCommandLineParser &parser)
{
    auto replayStartTime = getReplayDateTime(parser, "start");
    auto replayStopTime = getReplayDateTime(parser, "stop");

    if (replayStartTime.isValid()) {
        widget->setStart(replayStartTime);
    }
    if (replayStopTime.isValid()) {
        widget->setStop(replayStopTime);
    }
}

void ReplayController::setupDbus(const CONFIG_ITEM &config)
{
    new Tick_replayerAdaptor(replayerObj);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(config.dbusObject, replayerObj);
    dbus.registerService(config.dbusService);
}

QDateTime ReplayController::getReplayDateTime(QCommandLineParser &parser, const QString &option)
{
    QDateTime dateTime;
    if (parser.isSet(option)) {
        QString replayStartDateTime = parser.value(option);
        dateTime = QDateTime::fromString(replayStartDateTime, "yyyyMMddHHmmss");
        dateTime.setTimeZone(QTimeZone::utc());
    }
    return dateTime;
}
