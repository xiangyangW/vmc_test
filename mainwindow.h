#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort>
#include <QSerialPortInfo>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pbtn_open_clicked();

    void on_pbtn_send_clicked();

    void on_pbtn_clear_clicked();

    void onTimeOut();
    void receiveInfo();

private:
    QStringList    getPortNameList();
    void           receiveDataCheck(QByteArray&);

private:
    Ui::MainWindow  *ui;
    QSerialPort     *vmc_serialport_;
    QStringList     all_port_namelist_;
    QTimer          *timer_;
    bool            bool_readdata_;
    QString         cmd_string_;  // command given in line edit
};
#endif // MAINWINDOW_H
