#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent), role("guest")
{
    setWindowTitle("🔐 Авторизация");
    resize(300, 180);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- Поля ввода ---
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
    cancelButton = new QPushButton("❌ Отмена");

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(cancelButton);

    // --- Собираем всё ---
    mainLayout->addWidget(userLabel);
    mainLayout->addWidget(usernameEdit);
    mainLayout->addWidget(passLabel);
    mainLayout->addWidget(passwordEdit);
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);

    // --- Подключаем кнопки ---
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void LoginDialog::onLoginClicked()
{
    QString user = usernameEdit->text().trimmed();
    QString pass = passwordEdit->text().trimmed();

    // --- Проверка логинов и паролей ---
    if (user == "admin" && pass == "1234") {
        role = "admin";
        accept(); // Закрываем окно с успехом
    }
    else if (user == "analyst" && pass == "5678") {
        role = "analyst";
        accept();
    }
    else if (user == "guest" && pass == "0000") {
        role = "guest";
        accept();
    }
    else {
        statusLabel->setText("❌ Неверный логин или пароль!");
        usernameEdit->clear();
        passwordEdit->clear();
        usernameEdit->setFocus();
    }
}

QString LoginDialog::getRole() const
{
    return role;
}