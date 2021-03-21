#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("vmdbgui");
    a.setOrganizationName("MoseleyInstruments");
    a.setOrganizationDomain("moseleyinstruments.com");

    MainWindow w;
    w.show();

    return a.exec();
}
