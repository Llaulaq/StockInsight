#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    LoginDialog(QWidget* parent = nullptr);
    QString getRole() const; // Возвращает роль пользователя

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QPushButton* loginButton;
    QPushButton* registerButton;
    QPushButton* cancelButton;
    QLabel* statusLabel;
    QString role; // Хранит роль: admin, analyst, guest
};