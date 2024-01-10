#ifndef SYSTEMPANEL_H
#define SYSTEMPANEL_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QtWidgets>
#include <QDebug>
#include <QString>
#include "qcustomplot.h"
#include <QTimer>
#include <QElapsedTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class SystemPanel;
}
QT_END_NAMESPACE

class SystemPanel : public QMainWindow
{
    Q_OBJECT

public:
    SystemPanel(QWidget *parent = nullptr);
    ~SystemPanel();
    void addPoint(double x, double y);
    void clearData();
    void plot();

private slots:
    void readSerial();
    void handleAction1();
    void handleAction2();
    void makePlot();
    void realTimeDataSlot();
private:
    Ui::SystemPanel *ui;
    QSerialPort *arduino;
    static const quint16 arduino_uno_vendorID = 9025;
    static const quint16 arduino_uno_productID = 67;
    QString arduino_port_name;
    bool arduino_is_available;
    QByteArray serialData;
    QString serialBuffer;
    QCustomPlot *customPlot;
    int res;
    QTimer dataTimer;
    QElapsedTimer timer;
    QVector<double> yValues;

};
#endif // SYSTEMPANEL_H
