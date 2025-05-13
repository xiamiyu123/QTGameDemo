#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    //设置窗口图标
    w.setWindowIcon(QIcon(":/resource/images/icons/gui_title.png"));
    w.show();

    return a.exec();
}
