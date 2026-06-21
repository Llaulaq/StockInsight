#include "LoginDialog.h"
#include "RegisterDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent), role("guest")
{
    setWindowTitle("🔐 Авторизация");
    resize(300, 220);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* userLabel = new QLabel("👤 Логин:");
    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Введите логин");

    QLabel* passLabel = new QLabel("🔑 Пароль:");
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Введите пароль");

    statusLabel = new QLabel();
    statusLabel->setStyleSheet("color: red;");

    // --- Кнопки ---
    loginButton = new QPushButton("✅ Войти");
    registerButton = new QPushButton("📝 Регистрация");
    cancelButton = new QPushButton("❌ Отмена");

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addWidget(userLabel);
    mainLayout->addWidget(usernameEdit);
    mainLayout->addWidget(passLabel);
    mainLayout->addWidget(passwordEdit);
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);

    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void LoginDialog::onLoginClicked()
{
    QString user = usernameEdit->text().trimmed();
    QString pass = passwordEdit->text().trimmed();

    // ВРЕМЕННАЯ БАЗА 
    QMap<QString, QString> users;
    QMap<QString, QString> passwords;
    users["admin"] = "admin";
    passwords["admin"] = "1234";
    users["analyst"] = "analyst";
    passwords["analyst"] = "5678";
    users["guest"] = "guest";
    passwords["guest"] = "0000";

    if (users.contains(user) && passwords[user] == pass) {
        role = users[user];
        accept();
    }
    else {
        statusLabel->setText("❌ Неверный логин или пароль!");
        usernameEdit->clear();
        passwordEdit->clear();
        usernameEdit->setFocus();
    }
}

void LoginDialog::onRegisterClicked()
{
    RegisterDialog reg;
    if (reg.exec() == QDialog::Accepted) {
        QString username = reg.getUsername();
        QString password = reg.getPassword();
        QString role = reg.getRole();

        QMessageBox::information(this, "Регистрация успешна",
            "Пользователь " + username + " зарегистрирован!\n"
            "Роль: " + role + "\n\n"
            "Теперь вы можете войти.");
    }
}

QString LoginDialog::getRole() const
{
    return role;
}