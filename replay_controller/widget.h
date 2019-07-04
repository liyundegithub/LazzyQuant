#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

class QTimer;
class CommonReplayer;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(CommonReplayer *replayer, QWidget *parent = nullptr);
    ~Widget();

    void onTimer();

private slots:
    void on_playButton_clicked();
    void on_pauseButton_clicked();
    void on_stopButton_clicked();

    void on_speedSlider_valueChanged(int value);

private:
    Ui::Widget *ui;
    CommonReplayer *replayer;
    QTimer *timer;
    int startTime;
    int endTime;
    int currentDate = 0;    // unix timestamp
    int currentTime = 0;    // unix timestamp
    int unit = 1;
    bool forcePause = false;
    bool forceStop = false;
};

#endif // WIDGET_H
