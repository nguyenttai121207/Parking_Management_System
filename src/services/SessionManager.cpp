#include "SessionManager.h"
#include "PasswordHasher.h"

SessionManager::SessionManager() {}

SessionManager& SessionManager::instance() {
    static SessionManager inst;
    return inst;
}

bool SessionManager::login(const QString& username, const QString& password) {
    auto userOpt = m_repo.findUserByUsername(username.trimmed());
    if (!userOpt.has_value()) return false;

    const User& user = userOpt.value();
    auto result = PasswordHasher::verify(password, user.getPasswordHash());

    if (result == PasswordHasher::VerifyResult::Wrong) return false;

    // Tương thích ngược: nếu hash cũ (SHA-256), upgrade lên PBKDF2 ngay sau login
    if (result == PasswordHasher::VerifyResult::OkLegacy) {
        m_repo.updateUserPasswordHash(user.getId(), PasswordHasher::hash(password));
    }

    m_currentUser = user;
    return true;
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
