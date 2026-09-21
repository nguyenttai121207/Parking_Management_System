#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager {
private:
    DatabaseManager();
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_db;
    bool m_initialized;

public:
    static DatabaseManager& instance();

    bool openDatabase(const QString& path = "parking_system.db");
    void closeDatabase();

    bool isConnected() const;
    QSqlDatabase getDatabase() const;

private:
    bool createTables();
    bool seedInitialData();
};

#endif // DATABASE_MANAGER_H
