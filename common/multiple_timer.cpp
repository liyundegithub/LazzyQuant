#include "multiple_timer.h"

#include <QList>
#include <QTime>
#include <QTimer>

MultipleTimer::MultipleTimer(QObject *parent)
    : QObject(parent)
{
    timeIndex = -1;
    busyTimer = new QTimer(this);
    busyTimer->setSingleShot(true);
    connect(busyTimer, SIGNAL(timeout()), this, SLOT(onBusyTimer()));
}

MultipleTimer::MultipleTimer(const QList<QTime> &timeList, QObject *parent)
    :  QObject(parent), timePoints(timeList)
{
    timeIndex = -1;
    busyTimer = new QTimer(this);
    busyTimer->setSingleShot(true);
    connect(busyTimer, SIGNAL(timeout()), this, SLOT(onBusyTimer()));
    setNextTimePoint();
}

MultipleTimer::~MultipleTimer()
{
    busyTimer->stop();
}

QList<QTime> MultipleTimer::getTimePoints()
{
    return timePoints;
}

void MultipleTimer::setNextTimePoint()
{
    const int size = timePoints.size();
    if (size == 0) {
        return;
    }
    int i;
    for (i = 0; i < size; i++) {
        int diff = QTime::currentTime().msecsTo(timePoints[i]);
        if (diff > 1000) {
            busyTimer->start(diff);
            timeIndex = i;
            break;
        }
    }
    if (i == size) {
        int diff = QTime::currentTime().msecsTo(timePoints[0]);
        busyTimer->start(diff + 86400000);   // diff should be negative, there are 86400 seconds in a day
        timeIndex = 0;
    }
}

void MultipleTimer::onBusyTimer()
{
    emit timesUp(timeIndex);
    setNextTimePoint();
}

void MultipleTimer::stop()
{
    busyTimer->stop();
}
