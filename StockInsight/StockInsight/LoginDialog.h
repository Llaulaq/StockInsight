#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    LoginDialog(QWidget* parent = nullptr);
    QString getRole() const; // Возвращает роль пользователя

private slots:
    void onLoginClicked();   // Обработчик нажатия кнопки "Войти"
    void onRegisterClicked(); // Обработчик нажатия кнопки "Регистрация"

private:
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QPushButton* loginButton;
    QPushButton* registerButton;
    QPushButton* cancelButton;
    QLabel* statusLabel;
    QString role; // Хранит роль: admin, analyst, guest

    // Данные пользователей
    QMap<QString, QString> users;     // логин -> роль
    QMap<QString, QString> passwords; // логин -> пароль

    // Работа с файлом users.json
    void loadUsersFromFile();   // Загружает пользователей из JSON
    void saveUsersToFile();     // Сохраняет пользователей в JSON
    void createDefaultUsersFile(); // Создаёт файл с пользователями по умолчанию
};