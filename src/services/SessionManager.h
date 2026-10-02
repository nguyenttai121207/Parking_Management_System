#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include "../models/User.h"
#include "../db/ParkingRepository.h"
#include <QString>
#include <optional>

class SessionManager {
private:
    SessionManager();
    ~SessionManager() = default;

    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    std::optional<User> m_currentUser;
    ParkingRepository m_repo;

public:
    static SessionManager& instance();

    bool login(const QString& username, const QString& password);
    void logout();

    bool isLoggedIn() const;
    bool isAdmin() const;
    bool isMaintenance() const;

    User getCurrentUser() const;
};

#endif // SESSION_MANAGER_H
