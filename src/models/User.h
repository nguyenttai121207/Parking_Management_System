#ifndef USER_H
#define USER_H

#include <QString>

class User {
private:
    int m_id;
    QString m_username;
    QString m_passwordHash;
    QString m_role; // "Admin", "Maintenance"

public:
    User() : m_id(0) {}

    User(int id, const QString& username, const QString& passwordHash, const QString& role)
        : m_id(id), m_username(username), m_passwordHash(passwordHash), m_role(role) {}

    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString getUsername() const { return m_username; }
    void setUsername(const QString& username) { m_username = username; }

    QString getPasswordHash() const { return m_passwordHash; }
    void setPasswordHash(const QString& hash) { m_passwordHash = hash; }

    QString getRole() const { return m_role; }
    void setRole(const QString& role) { m_role = role; }

    bool isAdmin() const { return m_role.compare(QStringLiteral("Admin"), Qt::CaseInsensitive) == 0; }
    bool isMaintenance() const { return m_role.compare(QStringLiteral("Maintenance"), Qt::CaseInsensitive) == 0; }
};

#endif // USER_H
