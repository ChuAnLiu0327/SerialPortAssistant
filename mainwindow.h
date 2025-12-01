#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QMessageBox>
#include "serialworker.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_ClearReceiveArea_pushButton_clicked();

    void on_ClearSendAreapushButton_clicked();

    void on_ConnectSerial_pushButton_clicked();

    void on_DisconnectSerial_pushButton_clicked();

    void on_RefreshSerialPort_pushButton_clicked();

    void onSerialOpened(bool success);

    void SerialComboBoxInit();

    void getCloseSerialMessage(bool message);

    void getNewPortsMessage(QList<QSerialPortInfo> list,QString message);

    void serialReceiveData(QByteArray data);

private:
    Ui::MainWindow *ui;

    // 添加线程和worker对象
    QThread *m_serialThread;
    SerialWorker *m_serialWorker;

signals:
    // 主线程发送给子线程的信号
    void openSerialRequest(const QString &portName,
                           qint32 baudRate,
                           QSerialPort::DataBits dataBits,
                           QSerialPort::Parity parity,
                           QSerialPort::StopBits stopBits,
                           QSerialPort::FlowControl flowControl);
    // 断开串口连接
    void closeSerialPort();

    void RefreshSerialPort();

    void startReceivData();

};
#endif // MAINWINDOW_H
