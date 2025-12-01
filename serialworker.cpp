#include "serialworker.h"
#include <QDebug>
#include <QThread>

SerialWorker::SerialWorker(QObject *parent)
    : QObject{parent}
{
    m_serialPort = new QSerialPort;
}

// 串口配置的槽函数
void SerialWorker::openSerialPort(const QString &portName,
                                  qint32 baudRate,
                                  QSerialPort::DataBits dataBits,
                                  QSerialPort::Parity parity,
                                  QSerialPort::StopBits stopBits,
                                  QSerialPort::FlowControl flowControl)
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }

    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(dataBits);
    m_serialPort->setParity(parity);
    m_serialPort->setStopBits(stopBits);
    m_serialPort->setFlowControl(flowControl);

    m_serialPort->open(QIODevice::ReadWrite);

    if(m_serialPort->isOpen())
    {
        // 串口连接成功后,调用函数触发信号,发送给主界面
        emit serialOpened(true);
    }
    else{
        emit serialOpened(false);
    }
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialWorker::startReceiveDataSub);
}

void SerialWorker::closeSerialPort()
{
    // 一定要断开信号槽,不然会一直会卡在数据读取阶段,无法关闭串口
    disconnect(m_serialPort, &QSerialPort::readyRead, this, &SerialWorker::startReceiveDataSub);
    m_serialPort->close();
    emit closeSerialMessage(true);
}

void SerialWorker::RefreshSerialPortSub()
{
    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();
    if(portList.isEmpty())
    {
        emit sendNewPortInfo(portList,"没有发现任何串口");
    }else
    {
        emit sendNewPortInfo(portList,"有串口");
    }
}


void SerialWorker::startReceiveDataSub()
{
    QByteArray data = m_serialPort->readAll();
    // qDebug() << m_serialPort;
    while(m_serialPort->waitForReadyRead(100))
    {
        data += m_serialPort->readAll();
    }
    emit serialReceiveData(data);
}

