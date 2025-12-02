#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

class SerialWorker : public QObject
{
    Q_OBJECT
public:
    explicit SerialWorker(QObject *parent = nullptr);


public slots:
    void openSerialPort(const QString &portName,
                        qint32 baudRate,
                        QSerialPort::DataBits dataBits,
                        QSerialPort::Parity parity,
                        QSerialPort::StopBits stopBits,
                        QSerialPort::FlowControl flowControl);


    void closeSerialPort();

    void RefreshSerialPortSub();

    void startReceiveDataSub();

    void writeData(const QByteArray &data);

signals:
    void dataReceived(const QByteArray &data);

    void errorOccurred(const QString &errorString);

    void serialOpened(bool success);

    void closeSerialMessage(bool message);

    void sendNewPortInfo(QList<QSerialPortInfo> list,QString refreshInfo);

    void serialReceiveData(QByteArray data);

private:
    QSerialPort *m_serialPort;
};



#endif // SERIALWORKER_H
