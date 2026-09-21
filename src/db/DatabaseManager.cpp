#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

DatabaseManager::DatabaseManager() : m_initialized(false) {}

DatabaseManager::~DatabaseManager() {
    closeDatabase();
}

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::openDatabase(const QString& path) {
    if (m_initialized && m_db.isOpen()) {
        return true;
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        qCritical() << "Lỗi mở SQLite database:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query(m_db);
    query.exec("PRAGMA foreign_keys = ON;");

    if (!createTables()) {
        qCritical() << "Lỗi tạo bảng cơ sở dữ liệu!";
        return false;
    }

    seedInitialData();

    m_initialized = true;
    return true;
}

void DatabaseManager::closeDatabase() {
    if (m_db.isOpen()) {
        m_db.close();
    }
    m_initialized = false;
}

bool DatabaseManager::isConnected() const {
    return m_db.isOpen();
}

QSqlDatabase DatabaseManager::getDatabase() const {
    return m_db;
}

bool DatabaseManager::createTables() {
    QSqlQuery query(m_db);

    const QString createPricing = R"(
        CREATE TABLE IF NOT EXISTS pricing_config (
            vehicle_type INTEGER PRIMARY KEY,
            type_name TEXT NOT NULL,
            hourly_rate REAL NOT NULL,
            monthly_rate REAL NOT NULL,
            updated_at TEXT NOT NULL
        );
    )";
    if (!query.exec(createPricing)) return false;

    const QString createSlots = R"(
        CREATE TABLE IF NOT EXISTS parking_slots (
            slot_id INTEGER PRIMARY KEY AUTOINCREMENT,
            slot_number TEXT UNIQUE NOT NULL,
            slot_type INTEGER NOT NULL,
            is_occupied INTEGER NOT NULL DEFAULT 0,
            current_license_plate TEXT DEFAULT NULL
        );
    )";
    if (!query.exec(createSlots)) return false;

    const QString createSubscriptions = R"(
        CREATE TABLE IF NOT EXISTS monthly_subscriptions (
            subscription_id INTEGER PRIMARY KEY AUTOINCREMENT,
            license_plate TEXT NOT NULL,
            vehicle_type INTEGER NOT NULL,
            customer_name TEXT,
            phone_number TEXT,
            start_date TEXT NOT NULL,
            end_date TEXT NOT NULL,
            price_paid REAL NOT NULL,
            created_at TEXT NOT NULL
        );
    )";
    if (!query.exec(createSubscriptions)) return false;

    const QString createTickets = R"(
        CREATE TABLE IF NOT EXISTS tickets (
            ticket_id INTEGER PRIMARY KEY AUTOINCREMENT,
            license_plate TEXT NOT NULL,
            vehicle_type INTEGER NOT NULL,
            slot_id INTEGER NOT NULL,
            check_in_time TEXT NOT NULL,
            check_out_time TEXT DEFAULT NULL,
            pricing_type TEXT DEFAULT 'HOURLY',
            total_fee REAL DEFAULT 0.0,
            status TEXT NOT NULL DEFAULT 'ACTIVE',
            FOREIGN KEY(slot_id) REFERENCES parking_slots(slot_id)
        );
    )";
    if (!query.exec(createTickets)) return false;

    query.exec(R"(
        CREATE UNIQUE INDEX IF NOT EXISTS idx_active_ticket_license 
        ON tickets(license_plate) WHERE status = 'ACTIVE';
    )");

    return true;
}

bool DatabaseManager::seedInitialData() {
    QSqlQuery query(m_db);

    query.exec("SELECT COUNT(*) FROM pricing_config;");
    if (query.next() && query.value(0).toInt() == 0) {
        QString now = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        query.prepare(R"(
            INSERT INTO pricing_config (vehicle_type, type_name, hourly_rate, monthly_rate, updated_at)
            VALUES (?, ?, ?, ?, ?);
        )");

        struct DefaultPrice { int type; const char* name; double hourly; double monthly; };
        DefaultPrice defaults[] = {
            {0, "Xe máy xăng", 5000.0, 100000.0},
            {1, "Xe máy điện", 4000.0, 80000.0},
            {2, "Ô tô xăng", 25000.0, 1200000.0},
            {3, "Ô tô điện", 20000.0, 1000000.0}
        };

        for (const auto& item : defaults) {
            query.bindValue(0, item.type);
            query.bindValue(1, QString::fromUtf8(item.name));
            query.bindValue(2, item.hourly);
            query.bindValue(3, item.monthly);
            query.bindValue(4, now);
            query.exec();
        }
    }

    query.exec("SELECT COUNT(*) FROM parking_slots;");
    if (query.next() && query.value(0).toInt() == 0) {
        query.prepare("INSERT INTO parking_slots (slot_number, slot_type, is_occupied) VALUES (?, ?, 0);");

        for (int i = 1; i <= 20; ++i) {
            QString slotName = QString("M-%1").arg(i, 2, 10, QChar('0'));
            query.bindValue(0, slotName);
            query.bindValue(1, 0);
            query.exec();
        }

        for (int i = 1; i <= 10; ++i) {
            QString slotName = QString("C-%1").arg(i, 2, 10, QChar('0'));
            query.bindValue(0, slotName);
            query.bindValue(1, 1);
            query.exec();
        }
    }

    return true;
}
