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
    timer_ = new QTimer(this);
    timer_->setInterval(1000);
    all_port_namelist_ = getPortNameList();  // build a list that contains all the ports connecting
    ui->comboBox->addItems(all_port_namelist_);

    connect(vmc_serialport_, SIGNAL(readyRead()), this, SLOT(receiveInfo()));
    connect(timer_, SIGNAL(timeout()), this, SLOT(onTimeOut()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

QStringList MainWindow::getPortNameList()
{
    foreach (const QSerialPortInfo& info, QSerialPortInfo::availablePorts())
    {
        all_port_namelist_ << info.portName();
    }
    return all_port_namelist_;
}

void MainWindow::on_pbtn_open_clicked()
{
    // this function is used to choose a com port and open it

    if (vmc_serialport_->isOpen())
    {
        vmc_serialport_->clear();
        vmc_serialport_->close();
    }

    vmc_serialport_->setPortName(ui->comboBox->currentText());

    // initial setup for serialport
    vmc_serialport_->setBaudRate(QSerialPort::Baud115200, QSerialPort::AllDirections);
    vmc_serialport_->setDataBits(QSerialPort::Data8);
    vmc_serialport_->setStopBits(QSerialPort::OneStop);
    vmc_serialport_->setParity(QSerialPort::NoParity);
    vmc_serialport_->setFlowControl(QSerialPort::NoFlowControl);

    if (!vmc_serialport_->open(QIODevice::ReadWrite))
    {
        QMessageBox::warning(this, "warning", "Port didn't open" );
        return;
    }
    qDebug() << "ok"; // ************delete


}

void MainWindow::receiveInfo()
{
    bool_readdata_ = 1;
    QByteArray read_data = vmc_serialport_->readAll();
    qDebug() << read_data;
    ui->textBrowser->append(QString(read_data));

    receiveDataCheck(read_data);

}

void MainWindow::on_pbtn_send_clicked()
{
    bool_readdata_ = 0;  // this is for timer to check if i get data
    cmd_string_ =  ui->lineEdit->text() + "\n";
    QByteArray send_data =  cmd_string_.toLatin1();

    // TO DO: check the command in the line edit. see if it's correct.

    // before sending data, clear the buffer
    if (vmc_serialport_->isOpen())
    {
        vmc_serialport_->clear();
        if (vmc_serialport_->clear())
            qDebug() << "buffer cleared";
    }

    vmc_serialport_->write(send_data);
    qDebug() << send_data;

    // set a timer for 80s when data write to serialport
    timer_->start();
}

void MainWindow::on_pbtn_clear_clicked()
{
    if (vmc_serialport_->isOpen())
    {
        vmc_serialport_->clear();
    }
    ui->textBrowser->setText("");
}

void MainWindow::onTimeOut()
{
    static int real_time = 0;

    if (bool_readdata_)
    {
        real_time = 0;
        timer_->stop();
    }

    real_time ++;
    if (real_time > 80)
    {
        // warning for overtime
        // clear buffer
        QMessageBox::warning(this, "warning", "wrong cmd or respone overtime, pls send again");
        vmc_serialport_->clear();
        real_time = 0;
        timer_->stop();
    }
}

void MainWindow::receiveDataCheck(QByteArray& read_data)
{
    // 依據read_data和傳送指令比對，檢查運作是否正確
    // 條件式設置會因為非同步通訊有問題，如果response還沒處理好，cmd_string_可能已經被洗掉
    if ( QString(read_data) == "r1.0.1023")
    {
        if (cmd_string_ != "VMIF\n")
            qDebug() << "response don't match to command";
    }
    if ( QString(read_data) == "CPONOK")
    {
        if (cmd_string_ != "CPON\n")
            qDebug() << "response don't match to command";
    }
    if ( QString(read_data) == "CPOFOK")
    {
        if (cmd_string_ != "CPOF\n")
            qDebug() << "response don't match to command";
    }
    if ( QString(read_data) == "CHRT\r\n"
            || QString(read_data) == "CHRT\r\n"
            || QString(read_data) == "CHRTDO"
            || QString(read_data) == "CHRTEE"
            || QString(read_data) == "CHRTNO")
    {
        if (cmd_string_ != "CHRT\n")
        {
            qDebug() << "response don't match to command";
        }
    }
    QString response_str = QString(read_data);
    if (response_str.left(2) == "CH" )
    {
        if (response_str.endsWith("OK")
                || response_str.endsWith("OP")
                || response_str.endsWith("DO")
                || response_str.endsWith("EE01")
                || response_str.endsWith("EE02")
                || response_str.endsWith("EE03")
                || response_str.endsWith("EE04"))
        {
            QString str = response_str.mid(2, 2);
            if (str.toInt() > 10 && str.toInt() < 100)
            {
                if (cmd_string_.left(2) != "CH" || cmd_string_.right(2) != "\n")
                    qDebug() << "response don't match to command";
                QString str2 = cmd_string_.mid(2, 2);
                if (str2.toInt() > 99 || str2.toInt() < 11)
                    qDebug() << "response don't match to command";
            }
        }
    }
    // ******REAL TPAL IS NOT DONE
    if ( QString(read_data) == "TP01000.0TP02000.0TP03000.0TP04000.0TP05000.0TP06000.0TP07000.0TP08000.0CP0FN0CD0")
    {
        if (cmd_string_ != "TPAL\n")
            qDebug() << "response don't match to command";
    }
}
