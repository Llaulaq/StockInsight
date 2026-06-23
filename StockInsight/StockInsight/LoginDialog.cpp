#include "LoginDialog.h"
#include "RegisterDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent), role("guest")
{
    // Загружаем пользователей из файла
    loadUsersFromFile();

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

// Обработчик нажатия кнопки "Войти"
void LoginDialog::onLoginClicked()
{
    QString user = usernameEdit->text().trimmed();
    QString pass = passwordEdit->text().trimmed();

    // Проверяем по загруженным из файла данным
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

// Обработчик нажатия кнопки "Регистрация"
void LoginDialog::onRegisterClicked()
{
    RegisterDialog reg;
    if (reg.exec() == QDialog::Accepted) {
        QString username = reg.getUsername();
        QString password = reg.getPassword();
        QString role = reg.getRole();

        // Добавляем нового пользователя
        users[username] = role;
        passwords[username] = password;
        saveUsersToFile();

        QMessageBox::information(this, "Регистрация успешна",
            "Пользователь " + username + " зарегистрирован!\n"
            "Роль: " + role + "\n\n"
            "Теперь вы можете войти.");
    }
}

// Возвращает роль текущего пользователя
QString LoginDialog::getRole() const
{
    return role;
}

// Загружает пользователей из JSON-файла
void LoginDialog::loadUsersFromFile()
{
    QFile file("users.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        createDefaultUsersFile(); // Если файла нет — создаём
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    users.clear();
    passwords.clear();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    QJsonArray usersArray = root["users"].toArray();

    for (const QJsonValue& value : usersArray) {
        QJsonObject obj = value.toObject();
        QString username = obj["username"].toString();
        QString password = obj["password"].toString();
        QString role = obj["role"].toString();

        users[username] = role;
        passwords[username] = password;
    }
}

// Сохраняет пользователей в JSON-файл
void LoginDialog::saveUsersToFile()
{
    QJsonArray usersArray;

    for (auto it = users.begin(); it != users.end(); ++it) {
        QJsonObject obj;
        obj["username"] = it.key();
        obj["password"] = passwords[it.key()];
        obj["role"] = it.value();
        usersArray.append(obj);
    }

    QJsonObject root;
    root["users"] = usersArray;

    QFile file("users.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        file.close();
    }
}

// Создаёт файл с пользователями по умолчанию (admin, analyst, guest)
void LoginDialog::createDefaultUsersFile()
{
    users["admin"] = "admin";
    passwords["admin"] = "1234";
    users["analyst"] = "analyst";
    passwords["analyst"] = "5678";
    users["guest"] = "guest";
    passwords["guest"] = "0000";
    saveUsersToFile();
}