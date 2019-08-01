#include "multiple_timer.h"

#include <QList>
#include <QTime>
#include <QTimerEvent>

MultipleTimer::MultipleTimer(const QList<QTime> &timeList, QObject *parent)
    :  QObject(parent), timePoints(timeList)
{
    std::sort(timePoints.begin(), timePoints.end());
    auto end_unique = std::unique(timePoints.begin(), timePoints.end());
    timePoints.erase(end_unique, timePoints.end());

    setNextTimePoint();
}

QList<QTime> MultipleTimer::getTimePoints() const
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
            timerId = this->startTimer(diff);
            timeIndex = i;
            break;
        }
    }
    if (i == size) {
        int diff = QTime::currentTime().msecsTo(timePoints[0]);
        timerId = this->startTimer(diff + 86400000);   // diff should be negative, there are 86400 seconds in a day
        timeIndex = 0;
    }
}

void MultipleTimer::timerEvent(QTimerEvent * event)
{
    killTimer(event->timerId());
    emit timesUp(timeIndex);
    setNextTimePoint();
}

void MultipleTimer::stop()
{
    if (timerId != 0) {
        killTimer(timerId);
    }
}
