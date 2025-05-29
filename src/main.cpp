#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
        // 在创建 QApplication 实例之前添加此行
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QApplication app(argc, argv);
    MainWindow w;
    //设置窗口图标
    w.setWindowIcon(QIcon(":/resource/images/icons/gui_title.png"));
    w.show();

    return app.exec();
}

