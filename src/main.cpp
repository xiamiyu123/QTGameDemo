#include "mainwindow.h"
#include "login.h"
#include "mainmenu.h"
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QDateTime>

int main(int argc, char *argv[]) {
    // 在创建 QApplication 实例之前添加此行
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QApplication app(argc, argv);

    // 检查是否有自动登录标志
    QString userDataPath = QApplication::applicationDirPath() + "/user_data.ini";
    QSettings userSettings(userDataPath, QSettings::IniFormat);
    
    bool autoLoginEnabled = userSettings.value("AutoLogin/enabled", false).toBool();
    QString autoLoginUsername = userSettings.value("AutoLogin/username", "").toString();
    QString autoLoginTimestamp = userSettings.value("AutoLogin/timestamp", "").toString();
    
    // 检查自动登录是否有效（在24小时内且用户名不为空）
    bool autoLoginValid = false;
    if (autoLoginEnabled && !autoLoginUsername.isEmpty() && !autoLoginTimestamp.isEmpty()) {
        QDateTime loginTime = QDateTime::fromString(autoLoginTimestamp, Qt::ISODate);
        QDateTime currentTime = QDateTime::currentDateTime();
        // 如果在24小时内，认为自动登录有效
        if (loginTime.isValid() && loginTime.secsTo(currentTime) < 24 * 3600) {
            autoLoginValid = true;
        }
    }
    
    QString username;
    
    if (autoLoginValid) {
        // 使用自动登录
        username = autoLoginUsername;
        
        // 清除自动登录标志
        userSettings.setValue("AutoLogin/enabled", false);
        userSettings.remove("AutoLogin/username");
        userSettings.remove("AutoLogin/timestamp");
        userSettings.sync();
    } else {
        // 清除过期的自动登录数据
        userSettings.setValue("AutoLogin/enabled", false);
        userSettings.remove("AutoLogin/username");
        userSettings.remove("AutoLogin/timestamp");
        userSettings.sync();
        
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
        
        username = loginWindow.getUsername();
    }

    // 登录成功，显示主菜单界面
    MainMenu mainMenu(username);
    mainMenu.show();

    return app.exec();
}

