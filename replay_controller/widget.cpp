#include <QTimer>
#include <QCoreApplication>

#include "widget.h"
#include "ui_widget.h"
#include "common_replayer.h"

Widget::Widget(CommonReplayer *replayer, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    replayer(replayer)
{
    ui->setupUi(this);

    timer = new QTimer(this);
    timer->setInterval(2000);
    timer->setSingleShot(false);
    timer->setTimerType(Qt::PreciseTimer);
    connect(timer, &QTimer::timeout, this, &Widget::onTimer);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::setStart(const QDateTime &startDateTime)
{
    ui->startDateTimeEdit->setDateTime(startDateTime);
    ui->currentDateTimeEdit->setDateTime(startDateTime);
}

void Widget::setStop(const QDateTime &stopDateTime)
{
    ui->stopDateTimeEdit->setDateTime(stopDateTime);
}

void Widget::onTimer()
{
    bool haveData1 = false;
    bool haveData2 = false;
    while ((!haveData1) && (!haveData2)) {
        QCoreApplication::processEvents();
        if (currentTime >= endTime || forceStop) {
            on_stopButton_clicked();
            break;
        }
        if (forcePause) {
            on_pauseButton_clicked();
            break;
        }
        int targetTime = (currentTime < startTime) ? startTime : currentTime;
        targetTime += unit;
        targetTime = targetTime / unit * unit;
        targetTime += 30;

        int targetDate = targetTime / (24 * 3600) * (24 * 3600);
        if (currentDate != targetDate) {
            haveData1 = replayer->replayTo(targetDate);
            auto td = QDateTime::fromSecsSinceEpoch(targetDate, Qt::UTC);
            bool haveData3 = replayer->prepareReplay(td.toString(QStringLiteral("yyyyMMdd")));
            if (!haveData3) {
                targetTime = targetDate + 24 * 3600 - 1;
            }
            currentDate = targetDate;
        }
        haveData2 = replayer->replayTo(targetTime);
        currentTime = targetTime;
        ui->currentDateTimeEdit->setDateTime(QDateTime::fromSecsSinceEpoch(currentTime, Qt::UTC));
    }
}

void Widget::on_playButton_clicked()
{
    if (ui->startDateTimeEdit->isEnabled()) {
        startTime = ui->startDateTimeEdit->dateTime().toSecsSinceEpoch();
        endTime = ui->stopDateTimeEdit->dateTime().toSecsSinceEpoch();
        ui->startDateTimeEdit->setEnabled(false);
        ui->stopDateTimeEdit->setEnabled(false);
        currentDate = 0;
        currentTime = 0;
    }
    switch(ui->periodBox->currentIndex()) {
    case 0:
        unit = 60;
        break;
    case 1:
        unit = 5 * 60;
        break;
    case 2:
        unit = 15 * 60;
        break;
    case 3:
        unit = 30 * 60;
        break;
    case 4:
        unit = 60 * 60;
        break;
    case 5:
        unit = 24 * 60 * 60;
        break;
    default:
        unit = 1;
        break;
    }
    ui->periodBox->setEnabled(false);
    forcePause = false;
    forceStop = false;
    timer->start();
}

void Widget::on_pauseButton_clicked()
{
    timer->stop();
    ui->periodBox->setEnabled(true);
    forcePause = true;
}

void Widget::on_stopButton_clicked()
{
    timer->stop();
    ui->startDateTimeEdit->setEnabled(true);
    ui->stopDateTimeEdit->setEnabled(true);
    ui->periodBox->setEnabled(true);
    forceStop = true;
}

void Widget::on_speedSlider_valueChanged(int value)
{
    timer->setInterval(2000 / value);
}
