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
    void receiveInfo();

private:
    //void OpenPort();
    QStringList    getPortNameList();

private:
    Ui::MainWindow  *ui;
    QSerialPort     *vmc_serialport_;
    QStringList     all_port_namelist_;
};
#endif // MAINWINDOW_H
