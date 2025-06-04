#include "mainwindow.h"
#include "login.h"
#include "mainmenu.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[]) {
    // 在创建 QApplication 实例之前添加此行
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QApplication app(argc, argv);

    // 显示登录界面
    LoginWindow loginWindow;
    if (loginWindow.exec() != QDialog::Accepted) {
        // 用户取消登录或关闭登录窗口
        return 0;
    }

    // 检查登录是否成功
    if (!loginWindow.isLoginSuccessful()) {
        QMessageBox::warning(nullptr, "登录失败", "登录失败，程序将退出。");
        return 0;
    }

    // 登录成功，显示主菜单界面
    MainMenu mainMenu(loginWindow.getUsername());
    mainMenu.show();

    return app.exec();
}

