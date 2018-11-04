#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QuantTraderManager;
class DBusMonitor;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QuantTraderManager *manager, QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onDbusState(const QList<bool> &statusList);
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
    QObject *pTrader;
    const QMetaObject *traderMetaObj;
    DBusMonitor *monitor;

    bool prevTraderStatus = false;

    QString strategyId1;
    QString instrument1;
    QString strategyId2;
    QString instrument2;
};

#endif // MAINWINDOW_H
