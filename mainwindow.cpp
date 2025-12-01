#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QThread>
#include <QTime>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("串口助手");

    // 清除默认的数据
    ui->ComPortNum_comboBox->clear();

    ui->DisconnectSerial_pushButton->setEnabled(false);

    // 添加点作者信息
    ui->ReceivetextBrowser->append("AUTHOR : ChuAn Liu");
    ui->ReceivetextBrowser->append("VERSION : V1.6");
    ui->ReceivetextBrowser->append("https://github.com/ChuAnLiu0327");

    // 获取串口号
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    if(ports.isEmpty()){
        ui->ReceivetextBrowser->append("没有发现任何串口");
        qDebug() << "没有发现任何串口";
        ui->ConnectSerial_pushButton->setEnabled(false);
    }

    // debug 测试串口代码(后续会删除)
    // 遍历并添加串口信息
    foreach(const QSerialPortInfo &port, ports) {
        QString portInfo = QString("%1 - %2").arg(port.portName()).arg(port.description());
        ui->ComPortNum_comboBox->addItem(portInfo, port.portName());
        // qDebug() << "发现串口: " << port.portName()
        //          << " | 描述: " << port.description()
        //          << " | 厂商: " << port.manufacturer();
    }
    SerialComboBoxInit();


    // 创建线程对象
    QThread *m_serialThread = new QThread;

    // 创建类对象
    SerialWorker *m_serialWorker = new SerialWorker;

    // 将类对象移动到线程中
    m_serialWorker->moveToThread(m_serialThread);

    // 调用开启串口函数并且将数据传输到子线程中
    connect(this,&MainWindow::openSerialRequest,m_serialWorker,&SerialWorker::openSerialPort);

    // 接收来自子线程中串口是否配置完全
    connect(m_serialWorker,&SerialWorker::serialOpened,this,&MainWindow::onSerialOpened);

    // 断开串口连接
    connect(this,&MainWindow::closeSerialPort,m_serialWorker,&SerialWorker::closeSerialPort);
    connect(m_serialWorker,&SerialWorker::closeSerialMessage,this,&MainWindow::getCloseSerialMessage);

    // 刷新串口数据
    connect(this,&MainWindow::RefreshSerialPort,m_serialWorker,&SerialWorker::RefreshSerialPortSub);
    connect(m_serialWorker,&SerialWorker::sendNewPortInfo,this,&MainWindow::getNewPortsMessage);

    // 获取串口的数据
    connect(this,&MainWindow::startReceivData,m_serialWorker,&SerialWorker::startReceiveDataSub);
    connect(m_serialWorker,&SerialWorker::serialReceiveData,this,&MainWindow::serialReceiveData);

    // 开启配置串口任务
    m_serialThread->start();

}

MainWindow::~MainWindow()
{
    delete ui;
}


// 初始化ComboBox
void MainWindow::SerialComboBoxInit()
{
    // 波特率设置
    ui->BaudRate_comboBox->addItem("1200", QSerialPort::Baud1200);
    ui->BaudRate_comboBox->addItem("2400", QSerialPort::Baud2400);
    ui->BaudRate_comboBox->addItem("4800", QSerialPort::Baud4800);
    ui->BaudRate_comboBox->addItem("9600", QSerialPort::Baud9600);
    ui->BaudRate_comboBox->addItem("19200", QSerialPort::Baud19200);
    ui->BaudRate_comboBox->addItem("38400", QSerialPort::Baud38400);
    ui->BaudRate_comboBox->addItem("57600", QSerialPort::Baud57600);
    ui->BaudRate_comboBox->addItem("115200", QSerialPort::Baud115200);
    ui->BaudRate_comboBox->setCurrentText("115200");

    // 数据位设置
    ui->DataBit_comboBox->addItem("5", QSerialPort::Data5);
    ui->DataBit_comboBox->addItem("6", QSerialPort::Data6);
    ui->DataBit_comboBox->addItem("7", QSerialPort::Data7);
    ui->DataBit_comboBox->addItem("8", QSerialPort::Data8);
    ui->DataBit_comboBox->setCurrentText("8");

    // 校验位设置
    ui->Parity_comboBox->addItem("无校验", QSerialPort::NoParity);
    ui->Parity_comboBox->addItem("偶校验", QSerialPort::EvenParity);
    ui->Parity_comboBox->addItem("奇校验", QSerialPort::OddParity);
    ui->Parity_comboBox->addItem("标记校验", QSerialPort::MarkParity);
    ui->Parity_comboBox->addItem("空格校验", QSerialPort::SpaceParity);
    ui->Parity_comboBox->setCurrentText("无校验");

    // 停止位设置
    ui->StopBit_comboBox->addItem("1", QSerialPort::OneStop);
    ui->StopBit_comboBox->addItem("1.5", QSerialPort::OneAndHalfStop);
    ui->StopBit_comboBox->addItem("2", QSerialPort::TwoStop);
    ui->StopBit_comboBox->setCurrentText("1");

    // 填充流控制
    ui->FlowControl_comboBox->addItem("无流控制",QSerialPort::NoFlowControl);
    ui->FlowControl_comboBox->addItem("硬件流控制",QSerialPort::HardwareControl);
    ui->FlowControl_comboBox->addItem("软件流控制",QSerialPort::SoftwareControl);
    ui->FlowControl_comboBox->setCurrentText("无流控制");

    // 发送格式设置
    ui->SendType_comboBox->addItem("HEX");
    ui->SendType_comboBox->addItem("文本");

    // 接收格式设置
    ui->ReceiveType_comboBox->addItem("HEX");
    ui->ReceiveType_comboBox->addItem("文本");

}

// 清空接收区
void MainWindow::on_ClearReceiveArea_pushButton_clicked()
{
    ui->ReceivetextBrowser->clear();
}

// 清空发送区
void MainWindow::on_ClearSendAreapushButton_clicked()
{
    ui->SendtextEdit->clear();
}

// 连接串口(需要移植到多线程里面)
void MainWindow::on_ConnectSerial_pushButton_clicked()
{
    // 从ui获取串口的配置
    QString portName = ui->ComPortNum_comboBox->currentData().toString();
    qint32 baudRate = static_cast<QSerialPort::BaudRate>(ui->BaudRate_comboBox->currentData().toInt());
    QSerialPort::DataBits dataBits = static_cast<QSerialPort::DataBits>(ui->DataBit_comboBox->currentData().toInt());
    QSerialPort::Parity parity = static_cast<QSerialPort::Parity>(ui->Parity_comboBox->currentData().toInt());
    QSerialPort::StopBits stopBits = static_cast<QSerialPort::StopBits>(ui->StopBit_comboBox->currentData().toInt());
    QSerialPort::FlowControl flowControl = static_cast<QSerialPort::FlowControl>(ui->FlowControl_comboBox->currentData().toInt());

    ui->ReceivetextBrowser->append("正在打开串口");
    // 发射信号给子线程进行初始化
    emit openSerialRequest(portName,baudRate,dataBits,parity,stopBits,flowControl);
}

void MainWindow::onSerialOpened(bool success)
{
    if(success){
        ui->ReceivetextBrowser->append("串口配置成功");
        // 将连接按钮给失能
        ui->ConnectSerial_pushButton->setEnabled(false);
        ui->DisconnectSerial_pushButton->setEnabled(true);
        ui->RefreshSerialPort_pushButton->setEnabled(false);

        // 失能所有选择按钮
        ui->ComPortNum_comboBox->setEnabled(false);
        ui->BaudRate_comboBox->setEnabled(false);
        ui->DataBit_comboBox->setEnabled(false);
        ui->Parity_comboBox->setEnabled(false);
        ui->StopBit_comboBox->setEnabled(false);
        ui->FlowControl_comboBox->setEnabled(false);
        ui->SendType_comboBox->setEnabled(false);
        ui->ReceiveType_comboBox->setEnabled(false);

        emit startReceivData();

    }
    else{
        ui->ReceivetextBrowser->append("串口配置失败");
    }
}

// 断开按钮的槽函数
void MainWindow::on_DisconnectSerial_pushButton_clicked()
{
    emit closeSerialPort();
}

// 刷新串口
void MainWindow::on_RefreshSerialPort_pushButton_clicked()
{
    emit RefreshSerialPort();
    ui->ComPortNum_comboBox->clear();
}

void MainWindow::getCloseSerialMessage(bool message)
{
    if(message){
        ui->ReceivetextBrowser->append("串口关闭成功");
        ui->ConnectSerial_pushButton->setEnabled(true);
        ui->DisconnectSerial_pushButton->setEnabled(false);
        ui->RefreshSerialPort_pushButton->setEnabled(true);

        // 失能所有选择按钮
        ui->ComPortNum_comboBox->setEnabled(true);
        ui->BaudRate_comboBox->setEnabled(true);
        ui->DataBit_comboBox->setEnabled(true);
        ui->Parity_comboBox->setEnabled(true);
        ui->StopBit_comboBox->setEnabled(true);
        ui->FlowControl_comboBox->setEnabled(true);
        ui->SendType_comboBox->setEnabled(true);
        ui->ReceiveType_comboBox->setEnabled(true);

    }else{
        ui->ReceivetextBrowser->append("串口关闭失败");
    }
}

void MainWindow::getNewPortsMessage(QList<QSerialPortInfo> list,QString message)
{
    ui->ReceivetextBrowser->append(message);
    foreach(const QSerialPortInfo &port, list) {
        QString portInfo = QString("%1 - %2").arg(port.portName()).arg(port.description());
        ui->ComPortNum_comboBox->addItem(portInfo, port.portName());
    }
}

void MainWindow::serialReceiveData(QByteArray data)
{
    if(data.isEmpty())
    {
        // ui->ReceivetextBrowser->append("没有任何数据");
    }else
    {
        // 获取显示格式
        QString displayFormat = "文本"; // 默认值
        if (ui->ReceiveType_comboBox->count() > 0) {
            displayFormat = ui->ReceiveType_comboBox->currentText();
        }

        QString displayText;

        if (displayFormat == "HEX") {
            // HEX格式显示
            QStringList hexBytes;
            for (char byte : data) {
                hexBytes.append(QString("%1").arg(static_cast<unsigned char>(byte), 2, 16, QChar('0')).toUpper());
            }
            displayText = hexBytes.join(' ');
        } else {
            // 文本格式显示
            displayText = QString::fromLocal8Bit(data);

            displayText.remove('\r');
            displayText.remove('\n');
            displayText.replace('\t', "[TAB]");
        }

        // 添加时间戳
        QString currentTime = QTime::currentTime().toString("hh:mm:ss.zzz");
        QString finalText = QString("[%1] %2").arg(currentTime).arg(displayText);

        // 显示在界面中
        ui->ReceivetextBrowser->append(finalText);

        // 自动滚动
        QTextCursor cursor = ui->ReceivetextBrowser->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->ReceivetextBrowser->setTextCursor(cursor);
    }
}

