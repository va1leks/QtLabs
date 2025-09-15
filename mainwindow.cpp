#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QSerialPortInfo"
#include "QWindow"
#include <windows.h>
#include <iostream>
#include <QProcess>
#include <QStringDecoder>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    port1 = new QSerialPort(this);
    port2 = new QSerialPort(this);


    QWidget *info = new QWidget();
    connect(this, &MainWindow::destroyed, info, &QWidget::close);
    logs = new QTextBrowser(info);
    logs->setPlainText("-logs");


    QVBoxLayout *layout = new QVBoxLayout(info);
    layout->addWidget(logs);
    info->setLayout(layout);
    info->show();

    ui->comboBox->addItem("5bit");
    ui->comboBox->addItem("6bit");
    ui->comboBox->addItem("7bit");
    ui->comboBox->addItem("8bit");
    ui->comboBox->setCurrentIndex(3);
    setupPort(port1,"COM1");
    setupPort(port2,"COM2");

    foreach (const QSerialPortInfo &info1,QSerialPortInfo::availablePorts()) {
        QSerialPort port;
        port.setPort(info1);

            qDebug() << info1.portName() + info1.description() +  info1.manufacturer();

    }

}

void MainWindow::setupPort(QSerialPort *port, const QString &portName)
{
    switch (ui->comboBox->currentIndex()) {
    case 0: port->setDataBits(QSerialPort::Data5); break;
    case 1: port->setDataBits(QSerialPort::Data6); break;
    case 2: port->setDataBits(QSerialPort::Data7); break;
    case 3: port->setDataBits(QSerialPort::Data8); break;
    default: port->setDataBits(QSerialPort::Data8);
    }

    port->setPortName(portName);
    port->setBaudRate(QSerialPort::Baud9600);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);
}

MainWindow::~MainWindow()
{
    delete ui;

}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    if (port1->isOpen()) port1->close();
    if (port2->isOpen()) port2->close();
    logs->append("index -" + QString::number(index));
    setupPort(port1,"COM1");
    setupPort(port2,"COM2");

    if (!port1->open(QIODevice::ReadWrite)) {
        logs->append("Ошибка открытия COM1:" + port1->errorString());
        return;
    }

    // Шаг 5: Открытие порта 2 для чтения и записи
    if (!port2->open(QIODevice::ReadWrite)) {
        logs->append("Ошибка открытия COM2:" + port2->errorString());
        port1->close();
        return;
    }

    logs->append("Оба порта успешно открыты");
}


void MainWindow::on_pushButton_clicked()
{





  logs->append(ui->plainTextEdit->toPlainText());
    logs->append("Data bits: " + QString::number(port1->dataBits()));
  QByteArray data = ui->plainTextEdit->toPlainText().toUtf8();

  qint64 bytesWritten = port1->write(data);


  if (bytesWritten == -1) {
      logs->append( "Ошибка записи:"  +port1->errorString());
  } else {
      logs->append("Отправлено байт:" + bytesWritten);
  }


  if (!port1->waitForBytesWritten(1000)) {
     logs->append( "Таймаут записи");
  }

  if (port2->waitForReadyRead(1000)) {
      // Шаг 12: Чтение всех доступных данных
      QByteArray receivedData = port2->readAll();

      // Шаг 13: Преобразование обратно в строку
      QString receivedMessage = QString::fromUtf8(receivedData).trimmed();

      logs->append( "Получено:" + receivedMessage);
      logs->append( "Размер:" + QString::number(receivedData.size()) + "байт");
      ui->plainTextEdit_2->setPlainText(receivedMessage);
  } else {
      logs->append( "Данные не получены (таймаут)");
  }
}


void MainWindow::on_pushButton_2_clicked()
{
    logs->append(ui->plainTextEdit->toPlainText());
    QByteArray data = (ui->plainTextEdit_2->toPlainText()+= "\r\n").toUtf8();

    qint64 bytesWritten = port2->write(data);


    if (bytesWritten == -1) {
        logs->append( "Ошибка записи:"  +port1->errorString());
    } else {
        logs->append("Отправлено байт:" + bytesWritten);
    }


    if (!port2->waitForBytesWritten(1000)) {
        logs->append( "Таймаут записи");
    }

    if (port1->waitForReadyRead(1000)) {
        // Шаг 12: Чтение всех доступных данных
        QByteArray receivedData = port1->readAll();

        // Шаг 13: Преобразование обратно в строку
        QString receivedMessage = QString::fromUtf8(receivedData).trimmed();

        logs->append( "Получено:" + receivedMessage);
        logs->append( "Размер:" + QString::number(receivedData.size()) + "байт");
        ui->plainTextEdit->setPlainText(receivedMessage);
    } else {
        logs->append( "Данные не получены (таймаут)");
    }
}

