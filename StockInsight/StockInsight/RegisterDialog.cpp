#include "RegisterDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFile>

RegisterDialog::RegisterDialog(QWidget* parent)
    : QDialog(parent)
{
    // Загружаем стили (тёмная тема по умолчанию)
    QFile styleFile("styles/dark.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = styleFile.readAll();
        this->setStyleSheet(style);
        styleFile.close();
    }

    setWindowTitle("📝 Регистрация");
    resize(350, 250);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- Поля ввода ---
    QLabel* userLabel = new QLabel("👤 Логин:");
    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Придумайте логин");

    QLabel* passLabel = new QLabel("🔑 Пароль:");
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Придумайте пароль");

    QLabel* roleLabel = new QLabel("🎭 Роль:");
    roleCombo = new QComboBox();
    roleCombo->addItems({ "Аналитик", "Гость", "Админ" });

    // --- Секретный ключ (скрыт по умолчанию) ---
    QLabel* keyLabel = new QLabel("🔑 Секретный ключ:");
    secretKeyEdit = new QLineEdit();
    secretKeyEdit->setEchoMode(QLineEdit::Password);
    secretKeyEdit->setPlaceholderText("Введите секретный ключ для админа");
    secretKeyEdit->setVisible(false);
    keyLabel->setVisible(false);

    statusLabel = new QLabel();
    statusLabel->setStyleSheet("color: red;");

    // --- Кнопки ---
    registerButton = new QPushButton("✅ Зарегистрироваться");
    cancelButton = new QPushButton("❌ Отмена");

    registerButton->setObjectName("registerButton");
    cancelButton->setObjectName("cancelButton");

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(registerButton);
    buttonLayout->addWidget(cancelButton);

    // --- Собираем всё ---
    mainLayout->addWidget(userLabel);
    mainLayout->addWidget(usernameEdit);
    mainLayout->addWidget(passLabel);
    mainLayout->addWidget(passwordEdit);
    mainLayout->addWidget(roleLabel);
    mainLayout->addWidget(roleCombo);
    mainLayout->addWidget(keyLabel);
    mainLayout->addWidget(secretKeyEdit);
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);

    // --- Подключения ---
    connect(roleCombo, &QComboBox::currentTextChanged, this, &RegisterDialog::onRoleChanged);
    connect(registerButton, &QPushButton::clicked, this, &RegisterDialog::onRegisterClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void RegisterDialog::onRoleChanged(const QString& role)
{
    if (role == "Админ") {
        secretKeyEdit->setVisible(true);
        secretKeyEdit->setPlaceholderText("Введите секретный ключ для админа");
    }
    else {
        secretKeyEdit->setVisible(false);
        secretKeyEdit->clear();
    }
}

void RegisterDialog::onRegisterClicked()
{
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();
    QString role = roleCombo->currentText();
    QString secretKey = secretKeyEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        statusLabel->setText("❌ Заполните все поля!");
        return;
    }

    if (role == "Админ") {
        if (secretKey != "SUPER2026") {
            statusLabel->setText("❌ Неверный секретный ключ!");
            return;
        }
    }

    accept();
}

QString RegisterDialog::getUsername() const
{
    return usernameEdit->text().trimmed();
}

QString RegisterDialog::getPassword() const
{
    return passwordEdit->text().trimmed();
}

QString RegisterDialog::getRole() const
{
    return roleCombo->currentText();
}