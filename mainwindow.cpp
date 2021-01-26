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

    connect(vmc_serialport_,SIGNAL(readyRead()),this,SLOT(receiveInfo()));
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
    // this function is used to choose a com port and open it

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
{
    QByteArray read_data= vmc_serialport_->readAll();
    qDebug()<<read_data;
    ui->textBrowser->append(QString(read_data));
}

void MainWindow::on_pbtn_send_clicked()
{
    QString data_string =  ui->lineEdit->text() + "\n";
    QByteArray send_data =  data_string.toLatin1();

    // before sending data, clear the buffer
    if(vmc_serialport_->isOpen())
    {
        vmc_serialport_->clear();
        if(vmc_serialport_->clear())
            qDebug()<<"buffer cleared";
    }

    vmc_serialport_->write(send_data);
    qDebug()<<send_data;
}

void MainWindow::on_pbtn_clear_clicked()
{
    if(vmc_serialport_->isOpen())
    {
    vmc_serialport_->clear();
    }
    ui->textBrowser->setText("");
}
