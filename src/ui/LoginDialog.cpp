#include "LoginDialog.h"
#include "../services/SessionManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
}

void LoginDialog::setupUi() {
    setWindowTitle(QStringLiteral("Đăng Nhập Hệ Thống Quản Lý Bãi Xe"));
    setFixedSize(420, 360);
    setAttribute(Qt::WA_DeleteOnClose, false);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(28, 28, 28, 28);

    auto title = new QLabel(QStringLiteral("HỆ THỐNG BÃI XE TỰ ĐỘNG"), this);
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #89b4fa;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    auto subtitle = new QLabel(QStringLiteral("Vui lòng đăng nhập với tài khoản Admin hoặc Maintenance"), this);
    subtitle->setStyleSheet("color: #a6adc8; font-size: 11px;");
    subtitle->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitle);

    auto card = new QFrame(this);
    card->setStyleSheet("background-color: #252538; border: 1px solid #313244; border-radius: 8px; padding: 14px;");
    auto formLayout = new QVBoxLayout(card);
    formLayout->setSpacing(10);

    auto userLabel = new QLabel(QStringLiteral("Tên Đăng Nhập:"), card);
    userLabel->setStyleSheet("color: #bac2de; font-weight: 600; font-size: 12px;");
    m_usernameEdit = new QLineEdit(card);
    m_usernameEdit->setPlaceholderText(QStringLiteral("Nhập username (ví dụ: admin hoặc tech)"));
    m_usernameEdit->setText("admin");

    auto passLabel = new QLabel(QStringLiteral("Mật Khẩu:"), card);
    passLabel->setStyleSheet("color: #bac2de; font-weight: 600; font-size: 12px;");
    m_passwordEdit = new QLineEdit(card);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(QStringLiteral("Nhập mật khẩu"));
    m_passwordEdit->setText("admin123");

    formLayout->addWidget(userLabel);
    formLayout->addWidget(m_usernameEdit);
    formLayout->addWidget(passLabel);
    formLayout->addWidget(m_passwordEdit);

    mainLayout->addWidget(card);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: #f38ba8; font-size: 12px; font-weight: bold;");
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    auto btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    m_cancelBtn = new QPushButton(QStringLiteral("Thoát"), this);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);

    m_loginBtn = new QPushButton(QStringLiteral("Đăng Nhập"), this);
    m_loginBtn->setObjectName("primaryBtn");
    m_loginBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addWidget(m_loginBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
}

void LoginDialog::onLoginClicked() {
    QString user = m_usernameEdit->text().trimmed();
    QString pass = m_passwordEdit->text();

    if (user.isEmpty() || pass.isEmpty()) {
        m_errorLabel->setText(QStringLiteral("Vui lòng điền đầy đủ tài khoản và mật khẩu!"));
        m_errorLabel->show();
        return;
    }

    if (SessionManager::instance().login(user, pass)) {
        accept();
    } else {
        m_errorLabel->setText(QStringLiteral("Sai tên đăng nhập hoặc mật khẩu!"));
        m_errorLabel->show();
    }
}
