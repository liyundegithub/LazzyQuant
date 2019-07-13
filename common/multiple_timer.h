#ifndef MULTIPLE_TIMER_H
#define MULTIPLE_TIMER_H

#include <QObject>

template <typename T> class QList;
class QTime;
class QTimer;

class MultipleTimer : public QObject
{
    Q_OBJECT

public:
    explicit MultipleTimer(QObject *parent = nullptr);
    explicit MultipleTimer(const QList<QTime> &timeList, QObject *parent = nullptr);
    ~MultipleTimer();

    QList<QTime> getTimePoints();

    // TODO void addTimePoint(const QTime &timePoint);
    // TODO void removeTimePoint(const QTime &timePoint);

protected:
    QTimer *busyTimer;
    QList<QTime> timePoints;
    int timeIndex;

    void setNextTimePoint();

signals:
    void timesUp(int);

private slots:
    void onBusyTimer();

public slots:
    void stop();
};

#endif // MULTIPLE_TIMER_H
