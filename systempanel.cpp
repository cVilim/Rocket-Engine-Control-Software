#include "systempanel.h"
#include "ui_systempanel.h"
#include <QThread>

SystemPanel::SystemPanel(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SystemPanel)
{
    ui->setupUi(this);
    makePlot();
    arduino = new QSerialPort;  //QSerialPort setup
    arduino_is_available = false;
    arduino_port_name = "";
    serialBuffer = "";
    //gledamo kolko portova je dostupno na sistemu

    qDebug() << "Number of Ports: " <<QSerialPortInfo::availablePorts().length();
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        if(serialPortInfo.hasVendorIdentifier() &&serialPortInfo.hasProductIdentifier()){
            qDebug() << "Vendor ID: " << serialPortInfo.vendorIdentifier();
            qDebug() << "Product ID: " << serialPortInfo.productIdentifier();
        }
    }

    //na kojem portu je arduino

    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        if(serialPortInfo.hasVendorIdentifier() &&serialPortInfo.hasProductIdentifier()){
            if(serialPortInfo.vendorIdentifier() == arduino_uno_vendorID){
                if(serialPortInfo.productIdentifier() == arduino_uno_productID){
                    arduino_port_name = serialPortInfo.portName();
                    arduino_is_available = true;
                    qDebug() << "Port Available!";
                }
            }
        }
    }
    //postavke koje se postave ak je arduino dostupni
    if(arduino_is_available){
        arduino->setPortName(arduino_port_name);
        arduino->open(QSerialPort::ReadOnly);
        arduino->setBaudRate(QSerialPort::Baud19200);
        arduino->setDataBits(QSerialPort::Data8);
        arduino->setParity(QSerialPort::NoParity);
        arduino->setStopBits(QSerialPort::OneStop);
        arduino->setFlowControl(QSerialPort::NoFlowControl);
        QObject::connect(arduino, SIGNAL(readyRead()), this, SLOT(readSerial()));
    }
    else{
        QMessageBox::warning(this, "Error", "Port error 1: Couldn't find arduino");
    }

    QMenu *fileMenu = menuBar()->addMenu("Menu");

    QAction *action1 = new QAction("About RECS", this);
    connect(action1, &QAction::triggered, this, &SystemPanel::handleAction1);
    fileMenu->addAction(action1);

    QAction *action2 = new QAction("About QT", this);
    connect(action2, &QAction::triggered, this, &SystemPanel::handleAction2);
    fileMenu->addAction(action2);
}

SystemPanel::~SystemPanel()
{
    if(arduino->isOpen()){
        arduino->close();
    }
    delete ui;
}



void SystemPanel::readSerial()
{
    QByteArray serialData = arduino->readLine();
    qDebug() << "Received data from Arduino: " << serialData;
}

void SystemPanel::handleAction1(){
    QMessageBox::about(this, "About RECS", "RECS version: 1.0.0 \n\nÂ©Vilim Cindori, 2024");
}

void SystemPanel::handleAction2(){
    QMessageBox::aboutQt(this, "About QT");
}

void SystemPanel::makePlot()
{
    ui->customPlot->setNoAntialiasingOnDrag(true);
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setPen(QPen(QColor(48, 174, 50), 2));
    ui->customPlot->graph(0)->setBrush(QBrush(QColor(48, 174, 50, 50)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%m:%s:%ms");
    //bg
    QColor backgroundColor(30, 36, 40);

    //grid
    QColor gridColor(140, 140, 140);

    //alpha
    gridColor.setAlpha(150);
    ui->customPlot->graph(0)->rescaleAxes();
    //ime i rang x-osi
    ui->customPlot->xAxis->setLabel("Relative Timestamp");
    ui->customPlot->xAxis->setLabelColor(Qt::white);
    ui->customPlot->xAxis->setTicker(timeTicker);

    //ime i rang y-osi
    ui->customPlot->yAxis->setLabel("Pressure (bar)");
    ui->customPlot->yAxis->setLabelColor(Qt::white);
    ui->customPlot->yAxis->setRange(-5, 50);

    //boja pozadine
    ui->customPlot->setBackground(backgroundColor);

    //boja x-os
    ui->customPlot->xAxis->setBasePen(QPen(QColor(199, 203, 205)));
    ui->customPlot->xAxis->setTickPen(QPen(QColor(199, 203, 205)));
    ui->customPlot->xAxis->setSubTickPen(QPen(QColor(199, 203, 205)));
    ui->customPlot->xAxis->setTickLabelColor(QColor(199, 203, 205));

    //boja y-os
    ui->customPlot->yAxis->setBasePen(QPen(QColor(199, 203, 205)));
    ui->customPlot->yAxis->setTickPen(QPen(QColor(199, 203, 205)));
    ui->customPlot->yAxis->setSubTickPen(QPen(QColor(199, 203, 205)));
    ui->customPlot->yAxis->setTickLabelColor(QColor(199, 203, 205));

    //blend boja
    ui->customPlot->xAxis->grid()->setPen(QPen(gridColor, 0.6));

    //blend boja
    ui->customPlot->yAxis->grid()->setPen(QPen(gridColor, 0.6));
    //timer
    QTimer *dataTimer = new QTimer(this);
    dataTimer->setInterval(3);
    dataTimer->setSingleShot(false);
    connect(dataTimer, SIGNAL(timeout()), this, SLOT(realTimeDataSlot()));
    dataTimer->start();
}

void SystemPanel::realTimeDataSlot() {
    static QTime time(QTime::currentTime());
    double key = time.msecsTo(QTime::currentTime()) / 1000.0;
    //čitaj serial
    QByteArray serialData = arduino->readLine();
    QString sensor_reading(serialData);
    //pretvorba
    bool conversionOk;
    double yValue = sensor_reading.toDouble(&conversionOk);

    if (!conversionOk) {
        qDebug() << "Error converting QString to double: " << sensor_reading;

        if (!yValues.isEmpty()) {
            yValue = yValues.first();
        } else {
            qDebug() << "No valid values available.";
            return;
        }
    }
    yValues.prepend(yValue);
    while (yValues.size() > 5) {
        yValues.removeLast();
    }

    double smoothedValue = 0.0;
    for (double value : yValues) {
        smoothedValue += value;
    }
    smoothedValue /= yValues.size();

    ui->customPlot->graph(0)->addData(key, smoothedValue);

    ui->customPlot->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->customPlot->replot();
}
