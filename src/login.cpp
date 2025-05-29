// //
// // Created by 郭泰麟 on 25-5-27.
// //
// #include "login.h"
// #include <QApplication>
// #include <QScreen>
// #include <QMessageBox>
// #include <QCryptographicHash>
// #include <QPixmap>
// #include <QIcon>
// #include <QTimer>
// #include <QDateTime>
//
// LoginWindow::LoginWindow(QWidget *parent)
//     : QDialog(parent)
//     , m_loginSuccessful(false)
//     , m_settings(nullptr)
// {
//     setWindowTitle("滑雪大冒险 - 登录");
//     setWindowIcon(QIcon(":/resource/images/icons/gui_title.png"));
//     setFixedSize(700, 600);
//     setModal(true);
//
//     // 初始化设置
//     QString settingsPath = QApplication::applicationDirPath() + "/user_data.ini";
//     m_settings = new QSettings(settingsPath, QSettings::IniFormat);
//
//     setupUI();
//     setupStyles();
//     loadSavedCredentials();
//
//     // 居中显示
//     QScreen* screen = QGuiApplication::primaryScreen();
//     if (screen) {
//         QRect screenGeometry = screen->geometry();
//         QPoint center = screenGeometry.center() - QPoint(width() / 2, height() / 2);
//         move(center);
//     }
// }
//
// LoginWindow::~LoginWindow()
// {
//     delete m_settings;
// }
//
// void LoginWindow::setupUI()
// {
//     // 主布局
//     QVBoxLayout* mainLayout = new QVBoxLayout(this);
//     mainLayout->setSpacing(20);
//     mainLayout->setContentsMargins(40, 40, 40, 40);
//
//     // 标题区域
//     m_titleLabel = new QLabel("滑雪大冒险");
//     m_titleLabel->setAlignment(Qt::AlignCenter);
//     mainLayout->addWidget(m_titleLabel);
//
//     // 游戏logo或图片（如果有的话）
//     QLabel* logoLabel = new QLabel();
//     logoLabel->setAlignment(Qt::AlignCenter);
//     logoLabel->setMinimumHeight(60);
//     logoLabel->setStyleSheet("QLabel { background-color: rgba(135, 206, 235, 100); border-radius: 10px; }");
//     logoLabel->setText("🎿 ⛷️ 🏔️");
//     logoLabel->setFont(QFont("Arial", 14));
//     mainLayout->addWidget(logoLabel);
//
//     // 间距
//     mainLayout->addSpacing(20);
//
//     // 用户名输入
//     QLabel* usernameLabel = new QLabel("用户名:");
//     mainLayout->addWidget(usernameLabel);
//
//     m_usernameEdit = new QLineEdit();
//     m_usernameEdit->setPlaceholderText("请输入用户名");
//     m_usernameEdit->setMaxLength(20);
//     m_usernameEdit->setMinimumHeight(45);  // 增加输入框高度
//     mainLayout->addWidget(m_usernameEdit);
//
//     // 密码输入
//     QLabel* passwordLabel = new QLabel("密码:");
//     mainLayout->addWidget(passwordLabel);
//
//     m_passwordEdit = new QLineEdit();
//     m_passwordEdit->setPlaceholderText("请输入密码");
//     m_passwordEdit->setMaxLength(20);
//     m_usernameEdit->setMinimumHeight(45);  // 增加输入框高度
//     mainLayout->addWidget(m_passwordEdit);
//
//     // 记住密码选项
//     m_rememberPasswordCheckBox = new QCheckBox("记住密码");
//     mainLayout->addWidget(m_rememberPasswordCheckBox);
//
//     // 状态标签
//     m_statusLabel = new QLabel();
//     m_statusLabel->setAlignment(Qt::AlignCenter);
//     m_statusLabel->setWordWrap(true);
//     mainLayout->addWidget(m_statusLabel);
//
//     // 按钮区域
//     QHBoxLayout* buttonLayout = new QHBoxLayout();
//
//     m_loginButton = new QPushButton("登录");
//     m_registerButton = new QPushButton("注册");
//
//     buttonLayout->addWidget(m_registerButton);
//     buttonLayout->addWidget(m_loginButton);
//
//     mainLayout->addLayout(buttonLayout);
//
//     // 连接信号槽
//     connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
//     connect(m_registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
//     connect(m_rememberPasswordCheckBox, &QCheckBox::toggled, this, &LoginWindow::onRememberPasswordToggled);
//
//     // 回车键登录
//     connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
//     connect(m_usernameEdit, &QLineEdit::returnPressed, [this]() {
//         m_passwordEdit->setFocus();
//     });
//
//     // 设置默认焦点
//     m_usernameEdit->setFocus();
// }
//
// void LoginWindow::setupStyles()
// {
//     setStyleSheet(
//         "QDialog {"
//         "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
//         "    stop:0 #87CEEB, stop:1 #E0F6FF);"
//         "  border-radius: 15px;"
//         "}"
//
//         "QLabel {"
//         "  color: #2C3E50;"
//         "  font-weight: bold;"
//         "}"
//
//         "QLineEdit {"
//         "  padding: 12px 15px;"
//         "  border: 2px solid #BDC3C7;"
//         "  border-radius: 8px;"
//         "  background-color: white;"
//         "  font-size: 14px;"
//         "  selection-background-color: #3498DB;"
//         "}"
//
//         "QLineEdit:focus {"
//         "  border: 2px solid #3498DB;"
//         "  background-color: #F8F9FA;"
//         "}"
//
//         "QPushButton {"
//         "  padding: 12px 25px;"
//         "  border: none;"
//         "  border-radius: 8px;"
//         "  font-size: 14px;"
//         "  font-weight: bold;"
//         "  min-width: 100px;"
//         "}"
//
//         "QPushButton#loginButton {"
//         "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
//         "    stop:0 #3498DB, stop:1 #2980B9);"
//         "  color: white;"
//         "}"
//
//         "QPushButton#loginButton:hover {"
//         "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
//         "    stop:0 #5DADE2, stop:1 #3498DB);"
//         "}"
//
//         "QPushButton#loginButton:pressed {"
//         "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
//         "    stop:0 #2980B9, stop:1 #21618C);"
//         "}"
//
//         "QPushButton#registerButton {"
//         "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
//         "    stop:0 #27AE60, stop:1 #229954);"
//         "  color: white;"
//         "}"
//
//         "QPushButton#registerButton:hover {"
//         "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
//         "    stop:0 #58D68D, stop:1 #27AE60);"
//         "}"
//
//         "QPushButton#registerButton:pressed {"
//         "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
//         "    stop:0 #229954, stop:1 #1E8449);"
//         "}"
//
//         "QCheckBox {"
//         "  color: #2C3E50;"
//         "  font-size: 13px;"
//         "}"
//
//         "QCheckBox::indicator {"
//         "  width: 18px;"
//         "  height: 18px;"
//         "}"
//
//         "QCheckBox::indicator:unchecked {"
//         "  border: 2px solid #BDC3C7;"
//         "  border-radius: 3px;"
//         "  background-color: white;"
//         "}"
//
//         "QCheckBox::indicator:checked {"
//         "  border: 2px solid #3498DB;"
//         "  border-radius: 3px;"
//         "  background-color: #3498DB;"
//         "  image: url(:/icons/checkmark.png);"
//         "}"
//     );
//
//     // 设置按钮ID用于样式
//     m_loginButton->setObjectName("loginButton");
//     m_registerButton->setObjectName("registerButton");
//
//     // 标题样式
//     m_titleLabel->setStyleSheet(
//         "QLabel {"
//         "  font-size: 32px;"
//         "  font-weight: bold;"
//         "  color: #2C3E50;"
//         "  margin-bottom: 10px;"
//         "}"
//     );
//
//     // 状态标签样式
//     m_statusLabel->setStyleSheet(
//         "QLabel {"
//         "  font-size: 12px;"
//         "  min-height: 20px;"
//         "  color: #E74C3C;"
//         "}"
//     );
// }
//
// void LoginWindow::loadSavedCredentials()
// {
//     if (m_settings->value("RememberPassword", false).toBool()) {
//         m_rememberPasswordCheckBox->setChecked(true);
//         m_usernameEdit->setText(m_settings->value("Username").toString());
//         m_passwordEdit->setText(m_settings->value("Password").toString());
//     }
// }
//
// void LoginWindow::saveCredentials()
// {
//     if (m_rememberPasswordCheckBox->isChecked()) {
//         m_settings->setValue("RememberPassword", true);
//         m_settings->setValue("Username", m_usernameEdit->text());
//         m_settings->setValue("Password", m_passwordEdit->text());
//     } else {
//         m_settings->setValue("RememberPassword", false);
//         m_settings->remove("Username");
//         m_settings->remove("Password");
//     }
// }
//
// void LoginWindow::onLoginClicked()
// {
//     QString username = m_usernameEdit->text().trimmed();
//     QString password = m_passwordEdit->text();
//
//     // 验证输入
//     if (username.isEmpty()) {
//         m_statusLabel->setText("请输入用户名");
//         m_statusLabel->setStyleSheet("QLabel { color: #E74C3C; }");
//         m_usernameEdit->setFocus();
//         return;
//     }
//
//     if (password.isEmpty()) {
//         m_statusLabel->setText("请输入密码");
//         m_statusLabel->setStyleSheet("QLabel { color: #E74C3C; }");
//         m_passwordEdit->setFocus();
//         return;
//     }
//
//     // 验证用户凭据
//     if (validateCredentials(username, password)) {
//         m_statusLabel->setText("登录成功！");
//         m_statusLabel->setStyleSheet("QLabel { color: #27AE60; }");
//         m_loginSuccessful = true;
//         m_currentUsername = username;
//
//         // 保存凭据（如果选择记住密码）
//         saveCredentials();
//
//         // 延迟关闭对话框
//         QTimer::singleShot(500, this, &QDialog::accept);
//     } else {
//         m_statusLabel->setText("用户名或密码错误");
//         m_statusLabel->setStyleSheet("QLabel { color: #E74C3C; }");
//         m_passwordEdit->clear();
//         m_passwordEdit->setFocus();
//     }
// }
//
// void LoginWindow::onRegisterClicked()
// {
//     QString username = m_usernameEdit->text().trimmed();
//     QString password = m_passwordEdit->text();
//
//     // 验证输入
//     if (username.isEmpty()) {
//         m_statusLabel->setText("请输入用户名");
//         m_statusLabel->setStyleSheet("QLabel { color: #E74C3C; }");
//         m_usernameEdit->setFocus();
//         return;
//     }
//
//     if (username.length() < 3) {
//         m_statusLabel->setText("用户名至少需要3个字符");
//         m_statusLabel->setStyleSheet("QLabel { color: #E74C3C; }");
//         m_usernameEdit->setFocus();
//         return;
//     }
//
//     if (password.isEmpty()) {
//         m_statusLabel->setText("请输入密码");
//         m_statusLabel->setStyleSheet("QLabel { color: #E74C3C; }");
//         m_passwordEdit->setFocus();
//         return;
//     }
//
//     if (password.length() < 6) {
//         m_statusLabel->setText("密码至少需要6个字符");
//         m_statusLabel->setStyleSheet("QLabel { color: #E74C3C; }");
//         m_passwordEdit->setFocus();
//         return;
//     }
//
//     // 检查用户是否已存在
//     if (userExists(username)) {
//         m_statusLabel->setText("用户名已存在，请选择其他用户名");
//         m_statusLabel->setStyleSheet("QLabel { color: #E74C3C; }");
//         m_usernameEdit->setFocus();
//         return;
//     }
//
//     // 注册用户
//     registerUser(username, password);
//     m_statusLabel->setText("注册成功！请使用新账户登录");
//     m_statusLabel->setStyleSheet("QLabel { color: #27AE60; }");
//
//     // 清空密码字段，保留用户名
//     m_passwordEdit->clear();
//     m_passwordEdit->setFocus();
// }
//
// void LoginWindow::onRememberPasswordToggled(bool checked)
// {
//     if (!checked) {
//         // 如果取消勾选，立即清除保存的凭据
//         m_settings->setValue("RememberPassword", false);
//         m_settings->remove("Username");
//         m_settings->remove("Password");
//     }
// }
//
// bool LoginWindow::validateCredentials(const QString& username, const QString& password)
// {
//     // 对密码进行哈希处理
//     QByteArray passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
//     QString hashedPassword = passwordHash.toHex();
//
//     // 从设置中获取保存的密码哈希
//     QString savedPasswordHash = m_settings->value(QString("Users/%1/password").arg(username)).toString();
//
//     return !savedPasswordHash.isEmpty() && savedPasswordHash == hashedPassword;
// }
//
// void LoginWindow::registerUser(const QString& username, const QString& password)
// {
//     // 对密码进行哈希处理
//     QByteArray passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
//     QString hashedPassword = passwordHash.toHex();
//
//     // 保存用户信息
//     m_settings->setValue(QString("Users/%1/password").arg(username), hashedPassword);
//     m_settings->setValue(QString("Users/%1/registerTime").arg(username),
//                         QDateTime::currentDateTime().toString(Qt::ISODate));
// }
//
// bool LoginWindow::userExists(const QString& username)
// {
//     return m_settings->contains(QString("Users/%1/password").arg(username));
// }