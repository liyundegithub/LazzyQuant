#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class LazzyQuantProxy;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(LazzyQuantProxy *proxy, QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onModuleState(bool watcherStatus, bool replayerStatus, bool executerStatus, bool traderStatus);
    void onNewBar(const QString &instrumentID, const QString &timeFrame);

private:
    void updateStrategy1Position();
    void updateStrategy2Position();

private slots:
    void on_comboBoxId1_currentTextChanged(const QString &arg1);
    void on_pushButtonEnable1_clicked();
    void on_pushButtonDisable1_clicked();

    void on_comboBoxId2_currentTextChanged(const QString &arg1);
    void on_pushButtonEnable2_clicked();
    void on_pushButtonDisable2_clicked();

private:
    Ui::MainWindow *ui;
    LazzyQuantProxy *proxy;

    bool prevTraderStatus = false;

    QString strategyId1;
    QString instrument1;
    QString strategyId2;
    QString instrument2;
};

#endif // MAINWINDOW_H
