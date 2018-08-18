#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lazzyquantproxy.h"

#include <QDebug>

MainWindow::MainWindow(LazzyQuantProxy *proxy, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    proxy(proxy)
{
    ui->setupUi(this);

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

void MainWindow::onModuleState(bool watcherStatus, bool replayerStatus, bool executerStatus, bool traderStatus)
{
    ui->labelWatcher1->setText(getStatusString(watcherStatus));
    ui->labelReplayer1->setText(getStatusString(replayerStatus));
    ui->labelExecuter1->setText(getStatusString(executerStatus));
    ui->labelTrader1->setText(getStatusString(traderStatus));

    ui->labelWatcher2->setStyleSheet(getStatusStyleSheet(watcherStatus));
    ui->labelReplayer2->setStyleSheet(getStatusStyleSheet(replayerStatus));
    ui->labelExecuter2->setStyleSheet(getStatusStyleSheet(executerStatus));
    ui->labelTrader2->setStyleSheet(getStatusStyleSheet(traderStatus));

    if (prevTraderStatus && !traderStatus) {
        prevTraderStatus = false;
        ui->groupBox1->setEnabled(false);
        ui->groupBox2->setEnabled(false);
    }
    if (!prevTraderStatus && traderStatus) {
        prevTraderStatus = true;
        auto strategyIds = proxy->getStrategyId();
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
    if (instrument1 == instrumentID) {
        updateStrategy1Position();
    }
    if (instrument2 == instrumentID) {
        updateStrategy2Position();
    }
}

void MainWindow::updateStrategy1Position()
{
    int p1 = proxy->getPositionByStrategyId(strategyId1);
    if (p1 == -INT_MAX) {
        p1 = 0;
    }
    ui->lcdNumberPosition1->display(p1);
}

void MainWindow::updateStrategy2Position()
{
    int p2 = proxy->getPositionByStrategyId(strategyId2);
    if (p2 == -INT_MAX) {
        p2 = 0;
    }
    ui->lcdNumberPosition2->display(p2);
}

void MainWindow::on_comboBoxId1_currentTextChanged(const QString &arg1)
{
    instrument1 = proxy->getInstrumentByStrategyId(arg1);
    if (!instrument1.isEmpty()) {
        strategyId1 = arg1;
        ui->labelInstrument1->setText(instrument1);
        bool isStrategyEnabled = proxy->getStrategyEnabled(strategyId1);
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
    proxy->setStrategyEnabled(strategyId1, true);
    ui->pushButtonEnable1->setDisabled(true);
    ui->pushButtonDisable1->setEnabled(true);
}

void MainWindow::on_pushButtonDisable1_clicked()
{
    proxy->setStrategyEnabled(strategyId1, false);
    ui->pushButtonEnable1->setDisabled(false);
    ui->pushButtonDisable1->setEnabled(false);
}

void MainWindow::on_comboBoxId2_currentTextChanged(const QString &arg1)
{
    instrument2 = proxy->getInstrumentByStrategyId(arg1);
    if (!instrument2.isEmpty()) {
        strategyId2 = arg1;
        ui->labelInstrument2->setText(instrument2);
        bool isStrategyEnabled = proxy->getStrategyEnabled(strategyId2);
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
    proxy->setStrategyEnabled(strategyId2, true);
    ui->pushButtonEnable2->setDisabled(true);
    ui->pushButtonDisable2->setEnabled(true);
}

void MainWindow::on_pushButtonDisable2_clicked()
{
    proxy->setStrategyEnabled(strategyId2, false);
    ui->pushButtonEnable2->setDisabled(false);
    ui->pushButtonDisable2->setEnabled(false);
}
