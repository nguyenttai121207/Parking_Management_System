#include "SessionManager.h"
#include <QCryptographicHash>

SessionManager::SessionManager() {}

SessionManager& SessionManager::instance() {
    static SessionManager inst;
    return inst;
}

bool SessionManager::login(const QString& username, const QString& password) {
    QString hash = QString::fromLatin1(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
    auto userOpt = m_repo.verifyUserCredentials(username, hash);
    if (userOpt.has_value()) {
        m_currentUser = userOpt.value();
        return true;
    }
    return false;
}

void SessionManager::logout() {
    m_currentUser = std::nullopt;
}

bool SessionManager::isLoggedIn() const {
    return m_currentUser.has_value();
}

bool SessionManager::isAdmin() const {
    return m_currentUser.has_value() && m_currentUser->isAdmin();
}

bool SessionManager::isMaintenance() const {
    return m_currentUser.has_value() && m_currentUser->isMaintenance();
}

User SessionManager::getCurrentUser() const {
    if (m_currentUser.has_value()) {
        return m_currentUser.value();
    }
    return User(0, QStringLiteral("Guest"), "", QStringLiteral("Guest"));
}
