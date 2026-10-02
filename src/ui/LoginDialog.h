#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class LoginDialog : public QDialog {
    Q_OBJECT

private:
    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QPushButton* m_loginBtn;
    QPushButton* m_cancelBtn;
    QLabel* m_errorLabel;

public:
    explicit LoginDialog(QWidget* parent = nullptr);

private slots:
    void onLoginClicked();

private:
    void setupUi();
};

#endif // LOGIN_DIALOG_H
