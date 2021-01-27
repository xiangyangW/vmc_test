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

    cmd_string_ = "1";

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
}

void MainWindow::receiveInfo()
{
    read_data_ = vmc_serialport_->readAll();

    // check if RX is correct
    // ps. RX is simply check
    if (receiveDataCheck(QString(read_data_)))
    {
        ui->textBrowser->append(QString(read_data_));
    }
    else
    {
        qDebug() << "system error : wrong RX";
        cmd_string_ = "1";
    }

}

void MainWindow::on_pbtn_send_clicked()
{
    if (cmd_string_ == "1")
    {
        cmd_string_ =  ui->lineEdit->text() + "\n";
        QByteArray send_data =  cmd_string_.toLatin1();

        // before sending data, clear the buffer
        if (vmc_serialport_->isOpen())
        {
            vmc_serialport_->clear();
        }

        // check the command in the line edit. see if it's correct.
        if (cmdBool(cmd_string_))
        {
            vmc_serialport_->write(send_data);

            // set a timer for 80s when data write to serialport
            timer_->start();
        }
        else
        {
            cmd_string_ = "1";
            ui->lineEdit->text() = "cmd is wrong";
        }

    }
    else
        ui->lineEdit->text() = "last cmd is still in process,pls wait";
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

    if (cmd_string_ == "1")
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
        cmd_string_ = "1";
        timer_->stop();
    }
}

bool MainWindow::cmdBool(QString cmd_str)
{
    if (cmd_str.size() == 5)
    {
        if (cmd_str == "VMIF\n"
                || cmd_str == "CPON\n"
                || cmd_str == "CPOF\n"
                || cmd_str == "TPAL\n")
            return true;
        if (cmd_str.left(2) == "CH" && cmd_str.right(1) == "\n")
        {
            // strings that start with "CH" come in
            QString check_num = cmd_str.mid(2, 2);
            bool is_num;
            check_num.toInt(&is_num, 10);
            if (is_num && check_num.at(0) != "0" && check_num.at(1) != "0")
                return true;
            else
            {
                if (check_num == "RT")
                    return true;
                else
                {
                    cmd_string_ = "1";
                    return false;
                }
            }
        }
        else
        {
            cmd_string_ = "1";
            return false;
        }
    }
    else
    {
        cmd_string_ = "1";
        return false;
    }
}

bool MainWindow::receiveDataCheck(QString read_str)
{
    if (cmd_string_ == "CPON\n")
        return true;
    {
        if (read_str.size() == 6)
        {
            if (read_str == "CPONOK")
            {
                cmd_string_ = "1";
                return true;
            }
            else
                return false;
        }
        else
        {
            // if its size is wrong wait for a while
            while (vmc_serialport_->waitForReadyRead())
            {
                read_data_ += vmc_serialport_->readAll();
            }
            if (QString(read_data_) == "CPONOK")
            {
                return true;
                cmd_string_ = "1";
            }
            else
                return false;

        }
    }
    if (cmd_string_ == "CPOF\n")
    {
        if (read_str.size() == 6)
        {
            if (read_str == "CPOFOK")
            {
                return true;
                cmd_string_ = "1";
            }
            else
                return false;
        }
        else
        {
            // if its size is wrong wait for a while
            while (vmc_serialport_->waitForReadyRead())
            {
                read_data_ += vmc_serialport_->readAll();
            }
            if (QString(read_data_) == "CPOFOK")
            {
                return true;
                cmd_string_ = "1";
            }
            else
                return false;

        }
    }
    if (cmd_string_ == "TPAL\n")
    {
        if (read_str.size() == 81)
        {
            if (read_str.left(3) == "TP0")
            {
                return true;
                cmd_string_ = "1";
            }
            else
                return false;
        }
        else
        {
            // if its size is wrong wait for a while
            while (vmc_serialport_->waitForReadyRead())
            {
                read_data_ += vmc_serialport_->readAll();
            }
            if (read_str.size() == 81)
            {
                if (read_str.left(3) == "TP0")
                {
                    return true;
                    cmd_string_ = "1";
                }
                else
                    return false;
            }
            else
                return false;
        }
    }
    if (cmd_string_ == "CHRT\n")
    {
        if (read_str.size() == 6)
        {
            if (read_str == "CHRTDO"
                    || read_str == "CHRTNO"
                    || read_str == "CHRTEE")
            {
                cmd_string_ = "1";
                return true;
            }
            else if (read_str == "CHRT\r\n")
                return true;
            else
                return false;
        }
        else
        {
            // if its size is wrong wait for a while
            while (vmc_serialport_->waitForReadyRead())
            {
                read_data_ += vmc_serialport_->readAll();
            }
            if (read_str == "CHRTDO"
                    || read_str == "CHRTNO"
                    || read_str == "CHRTEE")
            {
                cmd_string_ = "1";
                return true;
            }
            else if (read_str == "CHRT\r\n")
                return true;
            else
                return false;

        }
    }

    QString check_num = cmd_string_.mid(2, 2);
    bool is_num;
    check_num.toInt(&is_num, 10);
    if (cmd_string_.left(2) == "CH" && is_num)
    {
        if (read_str.size() == 6 || read_str.size() == 8)
        {
            if (read_str.left(4) == cmd_string_.left(4))
            {
                return true;
                cmd_string_ = "1";
            }
            else
                return false;
        }
        else
        {
            while (vmc_serialport_->waitForReadyRead())
            {
                read_data_ += vmc_serialport_->readAll();
            }
            if (read_str.size() == 6 || read_str.size() == 8)
            {
                if (read_str.left(4) == cmd_string_.left(4))
                {
                    return true;
                    cmd_string_ = "1";
                }
                else
                    return false;
            }
            else
                return false;
        }
    }
    if (cmd_string_ == "VMIF\n")
    {
        if (read_str.size() == 9)
        {
            // check 1023 part
            QString check_four_digit = read_str.right(4);
            bool is_four;
            check_four_digit.toInt(&is_four, 10);
            // check 1.0 part
            QString check_float = read_str.mid(1, 3);
            bool is_float;
            check_float.toFloat(&is_float);
            if (read_str.at(0) == "r" && is_four && is_float)
            {
                return true;
                cmd_string_ = "1";
            }
            else
                return false;

        }
        else
        {
            // if its size is wrong wait for a while
            while (vmc_serialport_->waitForReadyRead())
            {
                read_data_ += vmc_serialport_->readAll();
            }
            // check 1023 part
            QString check_four_digit = read_str.right(4);
            bool is_four;
            check_four_digit.toInt(&is_four, 10);
            // check 1.0 part
            QString check_float = read_str.mid(1, 3);
            bool is_float;
            check_float.toFloat(&is_float);
            if (read_str.at(0) == "r" && is_four && is_float)
            {
                cmd_string_ = "1";
                return true;
            }
            else
                return false;
        }
    }
    else
        return false;
}

