#include "systempanel.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SystemPanel w;
    w.show();
    return a.exec();
}
