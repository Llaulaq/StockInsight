#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    RegisterDialog(QWidget* parent = nullptr);
    QString getUsername() const;
    QString getPassword() const;
    QString getRole() const;

private slots:
    void onRegisterClicked();
    void onRoleChanged(const QString& role);

private:
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QComboBox* roleCombo;
    QLineEdit* secretKeyEdit;
    QPushButton* registerButton;
    QPushButton* cancelButton;
    QLabel* statusLabel;
};