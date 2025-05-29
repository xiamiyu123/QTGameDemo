// //
// // Created by 郭泰麟 on 25-5-27.
// //
//
// #ifndef LOGIN_H
// #define LOGIN_H
//
// #endif //LOGIN_H
// #ifndef LOGINWINDOW_H
// #define LOGINWINDOW_H
//
// #include <QDialog>
// #include <QLineEdit>
// #include <QPushButton>
// #include <QLabel>
// #include <QVBoxLayout>
// #include <QHBoxLayout>
// #include <QCheckBox>
// #include <QSettings>
//
// class LoginWindow : public QDialog
// {
//     Q_OBJECT
//
// public:
//     explicit LoginWindow(QWidget *parent = nullptr);
//     ~LoginWindow();
//
//     // 获取登录结果
//     bool isLoginSuccessful() const { return m_loginSuccessful; }
//     QString getUsername() const { return m_currentUsername; }
//
//     private slots:
//         void onLoginClicked();
//     void onRegisterClicked();
//     void onRememberPasswordToggled(bool checked);
//
// private:
//     void setupUI();
//     void setupStyles();
//     void loadSavedCredentials();
//     void saveCredentials();
//     bool validateCredentials(const QString& username, const QString& password);
//     void registerUser(const QString& username, const QString& password);
//     bool userExists(const QString& username);
//
//     // UI组件
//     QLineEdit* m_usernameEdit;
//     QLineEdit* m_passwordEdit;
//     QPushButton* m_loginButton;
//     QPushButton* m_registerButton;
//     QCheckBox* m_rememberPasswordCheckBox;
//     QLabel* m_titleLabel;
//     QLabel* m_statusLabel;
//
//     // 数据
//     bool m_loginSuccessful;
//     QString m_currentUsername;
//     QSettings* m_settings;
// };
//
// #endif // LOGINWINDOW_H
