#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    vmc_serialport_ = new QSerialPort(this);
    all_port_namelist_ = getPortNameList();  // build a list that contains all the ports connecting
    ui->comboBox->addItems(all_port_namelist_);

}

MainWindow::~MainWindow()
{
    delete ui;
}

QStringList MainWindow::getPortNameList()
{
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts()){
        all_port_namelist_<<info.portName();
    }
    return all_port_namelist_;
}

void MainWindow::on_pbtn_open_clicked()
{
// choose a port and open it

    if(vmc_serialport_->isOpen())
    {
        vmc_serialport_->clear();
        vmc_serialport_->close();
    }

    vmc_serialport_->setPortName(ui->comboBox->currentText());

    // initial setup for serialport
    vmc_serialport_->setBaudRate(QSerialPort::Baud115200,QSerialPort::AllDirections);
    vmc_serialport_->setDataBits(QSerialPort::Data8);
    vmc_serialport_->setStopBits(QSerialPort::OneStop);
    vmc_serialport_->setParity(QSerialPort::NoParity);
    vmc_serialport_->setFlowControl(QSerialPort::NoFlowControl);

    if(!vmc_serialport_->open(QIODevice::ReadWrite)){
        QMessageBox::warning(this,"warning","Port didn't open" );
        return;
    }
    qDebug()<<"ok"; // ************delete
}

void MainWindow::receiveInfo()
{}
