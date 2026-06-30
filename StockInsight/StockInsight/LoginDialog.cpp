#include "LoginDialog.h"
#include "RegisterDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFile>

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent), role("guest")
{
    // Загружаем пользователей из файла
    loadUsersFromFile();

    // Загружаем стили (тёмная тема по умолчанию)
    QFile styleFile("styles/dark.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = styleFile.readAll();
        this->setStyleSheet(style);
        styleFile.close();
    }

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
    statusLabel->clear();  // Очищаем статус при создании

    // --- Кнопки ---
    loginButton = new QPushButton("✅ Войти");
    registerButton = new QPushButton("📝 Регистрация");
    cancelButton = new QPushButton("❌ Отмена");

    loginButton->setObjectName("loginButton");
    registerButton->setObjectName("registerButton");
    cancelButton->setObjectName("cancelButton");

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
        currentUser = user;   // ← запоминаем пользователя
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
    RegisterDialog reg(this);
    int result = reg.exec();

    if (result == QDialog::Accepted) {
        QString username = reg.getUsername();
        QString password = reg.getPassword();
        QString role = reg.getRole();

        // Добавляем нового пользователя
        users[username] = role;
        passwords[username] = password;
        userThemes[username] = "dark";
        saveUsersToFile();

        QMessageBox::information(this, "Регистрация успешна",
            "Пользователь " + username + " зарегистрирован!\n"
            "Роль: " + role + "\n\n"
            "Теперь вы можете войти.");
    }
    // При нажатии "Отмена" или закрытии окна регистрации - ничего не делаем,
    // просто остаёмся на окне входа
}

// Возвращает роль текущего пользователя
QString LoginDialog::getRole() const
{
    return role;
}

// Возвращает тему текущего пользователя
QString LoginDialog::getTheme() const
{
    if (userThemes.contains(currentUser)) {
        return userThemes[currentUser];
    }
    return "dark";
}

// Возвращает имя текущего пользователя
QString LoginDialog::getUsername() const
{
    return currentUser;
}

// РАБОТА С ФАЙЛОМ users.json

// Загружает пользователей из JSON-файла
void LoginDialog::loadUsersFromFile()
{
    QFile file("users.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        createDefaultUsersFile();
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    users.clear();
    passwords.clear();
    userThemes.clear();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    QJsonArray usersArray = root["users"].toArray();

    for (const QJsonValue& value : usersArray) {
        QJsonObject obj = value.toObject();
        QString username = obj["username"].toString();
        QString password = obj["password"].toString();
        QString role = obj["role"].toString();
        QString theme = obj["theme"].toString();

        if (theme.isEmpty()) {
            theme = "dark";
        }

        users[username] = role;
        passwords[username] = password;
        userThemes[username] = theme;
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
        obj["theme"] = userThemes.value(it.key(), "dark");
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

    userThemes["admin"] = "dark";
    userThemes["analyst"] = "dark";
    userThemes["guest"] = "dark";

    saveUsersToFile();
}

// Очищаем статус и поля при показе окна
void LoginDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    statusLabel->clear();
    usernameEdit->clear();
    passwordEdit->clear();
    usernameEdit->setFocus();
}