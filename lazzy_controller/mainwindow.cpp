#include "dbus_monitor.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "quant_trader_manager.h"

#include <QDebug>

MainWindow::MainWindow(QuantTraderManager *manager, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    pTrader(manager->getTrader()),
    traderMetaObj(pTrader->metaObject())
{
    ui->setupUi(this);

    monitor = new DBusMonitor({manager->getDataSource(), manager->getTrader(), manager->getExecuter()}, 1000, this);
    connect(monitor, SIGNAL(dbusStatus(QList<bool>)), this, SLOT(onDbusState(QList<bool>)));

    connect(pTrader, SIGNAL(newBarFormed(QString,QString)), this, SLOT(onNewBar(QString,QString)));

    statusBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

inline static QString getStatusString(bool status)
{
    const QString onlineString = QString("正常常.").left(2);
    const QString offlineString = QString("不在线线.").left(3);
    return (status ? onlineString : offlineString);
}

inline static QString getStatusStyleSheet(bool status)
{
    return QString("color: %1;").arg(status ? "green" : "black");
}

void MainWindow::onDbusState(const QList<bool> &statusList)
{
    ui->labelReplayer1->setText(getStatusString(statusList[0]));
    ui->labelExecuter1->setText(getStatusString(statusList[2]));
    ui->labelTrader1->setText(getStatusString(statusList[1]));

    ui->labelReplayer2->setStyleSheet(getStatusStyleSheet(statusList[0]));
    ui->labelExecuter2->setStyleSheet(getStatusStyleSheet(statusList[2]));
    ui->labelTrader2->setStyleSheet(getStatusStyleSheet(statusList[1]));

    if (prevTraderStatus && !statusList[1]) {
        prevTraderStatus = false;
        ui->groupBox1->setEnabled(false);
        ui->groupBox2->setEnabled(false);
    }
    if (!prevTraderStatus && statusList[1]) {
        prevTraderStatus = true;
        QStringList strategyIds;
        traderMetaObj->invokeMethod(pTrader, "getStrategyId", Qt::DirectConnection, Q_RETURN_ARG(QStringList, strategyIds));
        int strategyCount = strategyIds.size();
        ui->groupBox1->setEnabled(strategyCount > 0);
        ui->groupBox2->setEnabled(strategyCount > 1);
        if (strategyCount > 0) {
            ui->comboBoxId1->blockSignals(true);
            ui->comboBoxId1->clear();
            ui->comboBoxId1->addItems(strategyIds);
            ui->comboBoxId1->setCurrentIndex(0);
            ui->comboBoxId1->blockSignals(false);
            on_comboBoxId1_currentTextChanged(strategyIds[0]);
        }
        if (strategyCount > 1) {
            ui->comboBoxId2->blockSignals(true);
            ui->comboBoxId2->clear();
            ui->comboBoxId2->addItems(strategyIds);
            ui->comboBoxId2->setCurrentIndex(1);
            ui->comboBoxId2->blockSignals(false);
            on_comboBoxId2_currentTextChanged(strategyIds[1]);
        }
    }
}

void MainWindow::onNewBar(const QString &instrumentID, const QString &timeFrame)
{
    Q_UNUSED(timeFrame)
    if (instrument1 == instrumentID) {
        updateStrategy1Position();
    }
    if (instrument2 == instrumentID) {
        updateStrategy2Position();
    }
}

void MainWindow::updateStrategy1Position()
{
    int p1;
    traderMetaObj->invokeMethod(pTrader, "getPositionByStrategyId", Qt::DirectConnection, Q_RETURN_ARG(int, p1), Q_ARG(QString, strategyId1));
    ui->lcdNumberPosition1->display(p1);
}

void MainWindow::updateStrategy2Position()
{
    int p2;
    traderMetaObj->invokeMethod(pTrader, "getPositionByStrategyId", Qt::DirectConnection, Q_RETURN_ARG(int, p2), Q_ARG(QString, strategyId2));
    ui->lcdNumberPosition2->display(p2);
}

void MainWindow::on_comboBoxId1_currentTextChanged(const QString &arg1)
{
    traderMetaObj->invokeMethod(pTrader, "getInstrumentByStrategyId", Qt::DirectConnection, Q_RETURN_ARG(QString, instrument1), Q_ARG(QString, arg1));
    if (!instrument1.isEmpty()) {
        strategyId1 = arg1;
        ui->labelInstrument1->setText(instrument1);
        bool isStrategyEnabled;
        traderMetaObj->invokeMethod(pTrader, "getStrategyEnabled", Qt::DirectConnection, Q_RETURN_ARG(bool, isStrategyEnabled), Q_ARG(QString, strategyId1));
        ui->pushButtonEnable1->setDisabled(isStrategyEnabled);
        ui->pushButtonDisable1->setEnabled(isStrategyEnabled);
        updateStrategy1Position();
    } else {
        strategyId1 = QString();
        ui->labelInstrument1->setText(QString());
        ui->pushButtonEnable1->setDisabled(true);
        ui->pushButtonDisable1->setEnabled(true);
    }
}

void MainWindow::on_pushButtonEnable1_clicked()
{
    traderMetaObj->invokeMethod(pTrader, "setStrategyEnabled", Qt::DirectConnection, Q_ARG(QString, strategyId1), Q_ARG(bool, true));
    ui->pushButtonEnable1->setDisabled(true);
    ui->pushButtonDisable1->setEnabled(true);
}

void MainWindow::on_pushButtonDisable1_clicked()
{
    traderMetaObj->invokeMethod(pTrader, "setStrategyEnabled", Qt::DirectConnection, Q_ARG(QString, strategyId1), Q_ARG(bool, false));
    ui->pushButtonEnable1->setDisabled(false);
    ui->pushButtonDisable1->setEnabled(false);
}

void MainWindow::on_comboBoxId2_currentTextChanged(const QString &arg1)
{
    traderMetaObj->invokeMethod(pTrader, "getInstrumentByStrategyId", Qt::DirectConnection, Q_RETURN_ARG(QString, instrument2), Q_ARG(QString, arg1));
    if (!instrument2.isEmpty()) {
        strategyId2 = arg1;
        ui->labelInstrument2->setText(instrument2);
        bool isStrategyEnabled;
        traderMetaObj->invokeMethod(pTrader, "getStrategyEnabled", Qt::DirectConnection, Q_RETURN_ARG(bool, isStrategyEnabled), Q_ARG(QString, strategyId2));
        ui->pushButtonEnable2->setDisabled(isStrategyEnabled);
        ui->pushButtonDisable2->setEnabled(isStrategyEnabled);
        updateStrategy2Position();
    } else {
        strategyId2 = QString();
        ui->labelInstrument2->setText(QString());
        ui->pushButtonEnable2->setDisabled(true);
        ui->pushButtonDisable2->setEnabled(true);
    }
}

void MainWindow::on_pushButtonEnable2_clicked()
{
    traderMetaObj->invokeMethod(pTrader, "setStrategyEnabled", Qt::DirectConnection, Q_ARG(QString, strategyId2), Q_ARG(bool, true));
    ui->pushButtonEnable2->setDisabled(true);
    ui->pushButtonDisable2->setEnabled(true);
}

void MainWindow::on_pushButtonDisable2_clicked()
{
    traderMetaObj->invokeMethod(pTrader, "setStrategyEnabled", Qt::DirectConnection, Q_ARG(QString, strategyId2), Q_ARG(bool, false));
    ui->pushButtonEnable2->setDisabled(false);
    ui->pushButtonDisable2->setEnabled(false);
}
