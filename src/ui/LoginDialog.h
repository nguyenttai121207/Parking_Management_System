#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QAction>

class LoginDialog : public QDialog {
    Q_OBJECT

private:
    QLineEdit*  m_usernameEdit;
    QLineEdit*  m_passwordEdit;
    QPushButton* m_loginBtn;
    QPushButton* m_cancelBtn;
    QLabel*     m_errorLabel;
    QAction*    m_togglePassAction; // hiện/ẩn mật khẩu

public:
    explicit LoginDialog(QWidget* parent = nullptr);

private slots:
    void onLoginClicked();
    void onTogglePassword(bool checked);

private:
    void setupUi();
};

#endif // LOGIN_DIALOG_H
