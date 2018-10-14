#include "mainwindow.h"
#include <QApplication>
#include <QLabel>
#include <QPixmap>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();

    //QApplication a(argc, argv);
    //
    //QPixmap pixmap(R"(C:\images\img1.png)");
    //
    //QLabel label;
    //label.setPixmap(pixmap);
    //label.show();
    //return a.exec();
}


//R"(C:\images\img1.png)"
