#ifndef MULTIPLE_TIMER_H
#define MULTIPLE_TIMER_H

#include <QObject>

#include <QList>
#include <QTime>

class MultipleTimer : public QObject
{
    Q_OBJECT

public:
    explicit MultipleTimer(const QList<QTime> &timeList, QObject *parent = nullptr);

    QList<QTime> getTimePoints() const;

protected:
    QList<QTime> timePoints;
    int timeIndex = -1;
    int timerId = 0;

    void setNextTimePoint();
    void timerEvent(QTimerEvent *event) override;

signals:
    void timesUp(int);

public slots:
    void stop();

};

#endif // MULTIPLE_TIMER_H
