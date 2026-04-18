#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QIcon::setThemeName("Papirus-Dark");
    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
