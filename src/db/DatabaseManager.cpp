#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QCryptographicHash>

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

    // Bảng cấu hình biểu phí
    const QString createPricing = R"(
        CREATE TABLE IF NOT EXISTS pricing_config (
            vehicle_type INTEGER PRIMARY KEY,
            type_name TEXT NOT NULL,
            first_block_fee REAL NOT NULL DEFAULT 5000.0,
            next_block_fee REAL NOT NULL DEFAULT 3000.0,
            hourly_rate REAL DEFAULT 5000.0,
            monthly_rate REAL DEFAULT 100000.0,
            updated_at TEXT NOT NULL
        );
    )";
    if (!query.exec(createPricing)) {
        qWarning() << "Lỗi create pricing_config:" << query.lastError().text();
        return false;
    }

    // Đảm bảo có đủ cột first_block_fee và next_block_fee nếu đã tạo trước đó
    query.exec("ALTER TABLE pricing_config ADD COLUMN first_block_fee REAL DEFAULT 5000.0;");
    query.exec("ALTER TABLE pricing_config ADD COLUMN next_block_fee REAL DEFAULT 3000.0;");

    // Bảng Users (Prompt 4)
    const QString createUsers = R"(
        CREATE TABLE IF NOT EXISTS Users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            role TEXT NOT NULL
        );
    )";
    if (!query.exec(createUsers)) {
        qWarning() << "Lỗi create Users:" << query.lastError().text();
        return false;
    }

    // Bảng MonthlyPasses (Prompt 3)
    const QString createMonthlyPasses = R"(
        CREATE TABLE IF NOT EXISTS MonthlyPasses (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            customer_name TEXT NOT NULL,
            license_plate TEXT UNIQUE NOT NULL,
            vehicle_type INTEGER NOT NULL,
            start_date TEXT NOT NULL,
            expiration_date TEXT NOT NULL
        );
    )";
    if (!query.exec(createMonthlyPasses)) {
        qWarning() << "Lỗi create MonthlyPasses:" << query.lastError().text();
        return false;
    }

    // Bảng ParkingSessions (Prompt 1, 2, 3)
    const QString createSessions = R"(
        CREATE TABLE IF NOT EXISTS ParkingSessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            license_plate TEXT NOT NULL,
            vehicle_type INTEGER NOT NULL,
            check_in_time TEXT NOT NULL,
            check_out_time TEXT DEFAULT NULL,
            total_fee REAL DEFAULT 0.0,
            status TEXT NOT NULL DEFAULT 'Đang đỗ'
        );
    )";
    if (!query.exec(createSessions)) {
        qWarning() << "Lỗi create ParkingSessions:" << query.lastError().text();
        return false;
    }

    // Bảng slots và tickets cũ (để tương thích sơ đồ bãi xe hiện tại)
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
    query.exec(createSubscriptions);

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
    query.exec(createTickets);

    return true;
}

bool DatabaseManager::seedInitialData() {
    QSqlQuery query(m_db);
    QString now = QDateTime::currentDateTime().toString(Qt::ISODate);

    // 1. Seed Users (Prompt 4)
    query.exec("SELECT COUNT(*) FROM Users;");
    if (query.next() && query.value(0).toInt() == 0) {
        query.prepare(R"(
            INSERT INTO Users (username, password_hash, role)
            VALUES (?, ?, ?);
        )");

        // admin / admin123
        QString adminHash = QString::fromLatin1(QCryptographicHash::hash("admin123", QCryptographicHash::Sha256).toHex());
        query.bindValue(0, "admin");
        query.bindValue(1, adminHash);
        query.bindValue(2, "Admin");
        query.exec();

        // tech / tech123
        QString techHash = QString::fromLatin1(QCryptographicHash::hash("tech123", QCryptographicHash::Sha256).toHex());
        query.bindValue(0, "tech");
        query.bindValue(1, techHash);
        query.bindValue(2, "Maintenance");
        query.exec();
    }

    // 2. Seed pricing_config 4 loại xe (Prompt 5)
    query.exec("SELECT COUNT(*) FROM pricing_config;");
    if (query.next() && query.value(0).toInt() < 4) {
        query.exec("DELETE FROM pricing_config;"); // Reset để tránh lệch dữ liệu cũ
        query.prepare(R"(
            INSERT INTO pricing_config (vehicle_type, type_name, first_block_fee, next_block_fee, hourly_rate, monthly_rate, updated_at)
            VALUES (?, ?, ?, ?, ?, ?, ?);
        )");

        struct DefaultPricingItem {
            int type;
            const char* name;
            double firstBlock;
            double nextBlock;
            double hourly;
            double monthly;
        };

        DefaultPricingItem items[] = {
            {0, "Xe đạp", 2000.0, 1000.0, 2000.0, 50000.0},
            {1, "Xe máy số", 4000.0, 2000.0, 4000.0, 80000.0},
            {2, "Xe tay ga", 5000.0, 3000.0, 5000.0, 100000.0},
            {3, "Ô tô con", 25000.0, 15000.0, 25000.0, 1200000.0}
        };

        for (const auto& item : items) {
            query.bindValue(0, item.type);
            query.bindValue(1, QString::fromUtf8(item.name));
            query.bindValue(2, item.firstBlock);
            query.bindValue(3, item.nextBlock);
            query.bindValue(4, item.hourly);
            query.bindValue(5, item.monthly);
            query.bindValue(6, now);
            query.exec();
        }
    }

    // 3. Seed MonthlyPasses mẫu (Prompt 3)
    query.exec("SELECT COUNT(*) FROM MonthlyPasses;");
    if (query.next() && query.value(0).toInt() == 0) {
        query.prepare(R"(
            INSERT INTO MonthlyPasses (customer_name, license_plate, vehicle_type, start_date, expiration_date)
            VALUES (?, ?, ?, ?, ?);
        )");
        query.bindValue(0, QString::fromUtf8("Nguyễn Văn Tuấn"));
        query.bindValue(1, "29A-839.21");
        query.bindValue(2, 1); // Xe máy số
        query.bindValue(3, QDate::currentDate().addMonths(-1).toString(Qt::ISODate));
        query.bindValue(4, QDate::currentDate().addMonths(6).toString(Qt::ISODate));
        query.exec();
    }

    // 4. Seed parking_slots
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
