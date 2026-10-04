#include "ParkingRepository.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

std::optional<ParkingSlot> ParkingRepository::findAvailableSlot(SlotType type) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        SELECT slot_id, slot_number, slot_type, is_occupied, current_license_plate
        FROM parking_slots
        WHERE slot_type = ? AND is_occupied = 0
        ORDER BY slot_id ASC
        LIMIT 1;
    )");
    query.bindValue(0, static_cast<int>(type));

    if (query.exec() && query.next()) {
        return ParkingSlot(
            query.value(0).toInt(),
            query.value(1).toString(),
            static_cast<SlotType>(query.value(2).toInt()),
            query.value(3).toBool(),
            query.value(4).toString()
        );
    }
    return std::nullopt;
}

std::vector<ParkingSlot> ParkingRepository::getAllSlots() {
    std::vector<ParkingSlot> slotList;
    QSqlQuery query("SELECT slot_id, slot_number, slot_type, is_occupied, current_license_plate FROM parking_slots ORDER BY slot_type, slot_id;",
                    DatabaseManager::instance().getDatabase());
    while (query.next()) {
        slotList.emplace_back(
            query.value(0).toInt(),
            query.value(1).toString(),
            static_cast<SlotType>(query.value(2).toInt()),
            query.value(3).toBool(),
            query.value(4).toString()
        );
    }
    return slotList;
}

int ParkingRepository::countTotalSlots(SlotType type) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("SELECT COUNT(*) FROM parking_slots WHERE slot_type = ?;");
    query.bindValue(0, static_cast<int>(type));
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

int ParkingRepository::countOccupiedSlots(SlotType type) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("SELECT COUNT(*) FROM parking_slots WHERE slot_type = ? AND is_occupied = 1;");
    query.bindValue(0, static_cast<int>(type));
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

bool ParkingRepository::updateSlotOccupancy(int slotId, bool occupied, const QString& licensePlate) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        UPDATE parking_slots
        SET is_occupied = ?, current_license_plate = ?
        WHERE slot_id = ?;
    )");
    query.bindValue(0, occupied ? 1 : 0);
    query.bindValue(1, occupied ? licensePlate : QVariant(QVariant::String));
    query.bindValue(2, slotId);
    return query.exec();
}

std::optional<Ticket> ParkingRepository::createTicket(const QString& licensePlate, VehicleType vType, int slotId, const QDateTime& checkInTime) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        INSERT INTO tickets (license_plate, vehicle_type, slot_id, check_in_time, status)
        VALUES (?, ?, ?, ?, 'ACTIVE');
    )");
    query.bindValue(0, licensePlate);
    query.bindValue(1, static_cast<int>(vType));
    query.bindValue(2, slotId);
    query.bindValue(3, checkInTime.toString(Qt::ISODate));

    if (query.exec()) {
        int ticketId = query.lastInsertId().toInt();
        
        QSqlQuery slotQ(DatabaseManager::instance().getDatabase());
        slotQ.prepare("SELECT slot_number FROM parking_slots WHERE slot_id = ?;");
        slotQ.bindValue(0, slotId);
        QString slotNum = "";
        if (slotQ.exec() && slotQ.next()) {
            slotNum = slotQ.value(0).toString();
        }

        return Ticket(ticketId, licensePlate, vType, slotId, slotNum, checkInTime);
    }
    return std::nullopt;
}

std::optional<Ticket> ParkingRepository::getActiveTicketByPlate(const QString& licensePlate) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        SELECT t.ticket_id, t.license_plate, t.vehicle_type, t.slot_id, s.slot_number, t.check_in_time
        FROM tickets t
        LEFT JOIN parking_slots s ON t.slot_id = s.slot_id
        WHERE UPPER(t.license_plate) = UPPER(?) AND t.status = 'ACTIVE';
    )");
    query.bindValue(0, licensePlate.trimmed());

    if (query.exec() && query.next()) {
        int tId = query.value(0).toInt();
        QString plate = query.value(1).toString();
        VehicleType vType = static_cast<VehicleType>(query.value(2).toInt());
        int sId = query.value(3).toInt();
        QString sNum = query.value(4).toString();
        QDateTime inTime = QDateTime::fromString(query.value(5).toString(), Qt::ISODate);

        return Ticket(tId, plate, vType, sId, sNum, inTime);
    }
    return std::nullopt;
}

std::vector<Ticket> ParkingRepository::getAllActiveTickets() {
    std::vector<Ticket> list;
    QSqlQuery query(R"(
        SELECT t.ticket_id, t.license_plate, t.vehicle_type, t.slot_id, s.slot_number, t.check_in_time
        FROM tickets t
        LEFT JOIN parking_slots s ON t.slot_id = s.slot_id
        WHERE t.status = 'ACTIVE'
        ORDER BY t.check_in_time DESC;
    )", DatabaseManager::instance().getDatabase());

    while (query.next()) {
        list.emplace_back(
            query.value(0).toInt(),
            query.value(1).toString(),
            static_cast<VehicleType>(query.value(2).toInt()),
            query.value(3).toInt(),
            query.value(4).toString(),
            QDateTime::fromString(query.value(5).toString(), Qt::ISODate)
        );
    }
    return list;
}

bool ParkingRepository::completeTicket(int ticketId, const QDateTime& checkOutTime, double fee, const QString& pricingType) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        UPDATE tickets
        SET check_out_time = ?, total_fee = ?, pricing_type = ?, status = 'COMPLETED'
        WHERE ticket_id = ?;
    )");
    query.bindValue(0, checkOutTime.toString(Qt::ISODate));
    query.bindValue(1, fee);
    query.bindValue(2, pricingType);
    query.bindValue(3, ticketId);
    return query.exec();
}

std::vector<Ticket> ParkingRepository::searchTicketHistory(const QString& licensePlate) {
    std::vector<Ticket> results;
    QSqlQuery query(DatabaseManager::instance().getDatabase());

    if (licensePlate.trimmed().isEmpty()) {
        query.prepare(R"(
            SELECT t.ticket_id, t.license_plate, t.vehicle_type, t.slot_id, s.slot_number,
                   t.check_in_time, t.check_out_time, t.pricing_type, t.total_fee, t.status
            FROM tickets t
            LEFT JOIN parking_slots s ON t.slot_id = s.slot_id
            ORDER BY t.ticket_id DESC
            LIMIT 200;
        )");
    } else {
        query.prepare(R"(
            SELECT t.ticket_id, t.license_plate, t.vehicle_type, t.slot_id, s.slot_number,
                   t.check_in_time, t.check_out_time, t.pricing_type, t.total_fee, t.status
            FROM tickets t
            LEFT JOIN parking_slots s ON t.slot_id = s.slot_id
            WHERE UPPER(t.license_plate) LIKE UPPER(?)
            ORDER BY t.ticket_id DESC;
        )");
        query.bindValue(0, QString("%%1%").arg(licensePlate.trimmed()));
    }

    if (query.exec()) {
        while (query.next()) {
            Ticket t(
                query.value(0).toInt(),
                query.value(1).toString(),
                static_cast<VehicleType>(query.value(2).toInt()),
                query.value(3).toInt(),
                query.value(4).toString(),
                QDateTime::fromString(query.value(5).toString(), Qt::ISODate)
            );
            if (!query.value(6).isNull()) {
                t.setCheckOutTime(QDateTime::fromString(query.value(6).toString(), Qt::ISODate));
            }
            t.setPricingType(query.value(7).toString());
            t.setTotalFee(query.value(8).toDouble());
            t.setStatus(query.value(9).toString() == "ACTIVE" ? Ticket::Status::Active : Ticket::Status::Completed);
            results.push_back(t);
        }
    }
    return results;
}

bool ParkingRepository::saveSubscription(const MonthlySubscription& sub) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        INSERT INTO monthly_subscriptions 
        (license_plate, vehicle_type, customer_name, phone_number, start_date, end_date, price_paid, created_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?);
    )");
    query.bindValue(0, sub.getLicensePlate());
    query.bindValue(1, static_cast<int>(sub.getVehicleType()));
    query.bindValue(2, sub.getCustomerName());
    query.bindValue(3, sub.getPhoneNumber());
    query.bindValue(4, sub.getStartDate().toString(Qt::ISODate));
    query.bindValue(5, sub.getEndDate().toString(Qt::ISODate));
    query.bindValue(6, sub.getPricePaid());
    query.bindValue(7, QDateTime::currentDateTime().toString(Qt::ISODate));
    return query.exec();
}

std::optional<MonthlySubscription> ParkingRepository::findActiveSubscription(const QString& licensePlate, const QDate& onDate) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        SELECT subscription_id, license_plate, vehicle_type, customer_name, phone_number,
               start_date, end_date, price_paid
        FROM monthly_subscriptions
        WHERE UPPER(license_plate) = UPPER(?) AND date(?) BETWEEN date(start_date) AND date(end_date)
        ORDER BY subscription_id DESC
        LIMIT 1;
    )");
    query.bindValue(0, licensePlate.trimmed());
    query.bindValue(1, onDate.toString(Qt::ISODate));

    if (query.exec() && query.next()) {
        return MonthlySubscription(
            query.value(0).toInt(),
            query.value(1).toString(),
            static_cast<VehicleType>(query.value(2).toInt()),
            query.value(3).toString(),
            query.value(4).toString(),
            QDate::fromString(query.value(5).toString(), Qt::ISODate),
            QDate::fromString(query.value(6).toString(), Qt::ISODate),
            query.value(7).toDouble()
        );
    }
    return std::nullopt;
}

std::vector<MonthlySubscription> ParkingRepository::getAllSubscriptions() {
    std::vector<MonthlySubscription> list;
    QSqlQuery query("SELECT subscription_id, license_plate, vehicle_type, customer_name, phone_number, start_date, end_date, price_paid FROM monthly_subscriptions ORDER BY subscription_id DESC;",
                    DatabaseManager::instance().getDatabase());
    while (query.next()) {
        list.emplace_back(
            query.value(0).toInt(),
            query.value(1).toString(),
            static_cast<VehicleType>(query.value(2).toInt()),
            query.value(3).toString(),
            query.value(4).toString(),
            QDate::fromString(query.value(5).toString(), Qt::ISODate),
            QDate::fromString(query.value(6).toString(), Qt::ISODate),
            query.value(7).toDouble()
        );
    }
    return list;
}

std::optional<PricingConfig> ParkingRepository::getPricingConfig(VehicleType type) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("SELECT vehicle_type, type_name, hourly_rate, monthly_rate FROM pricing_config WHERE vehicle_type = ?;");
    query.bindValue(0, static_cast<int>(type));

    if (query.exec() && query.next()) {
        return PricingConfig(
            static_cast<VehicleType>(query.value(0).toInt()),
            query.value(1).toString(),
            query.value(2).toDouble(),
            query.value(3).toDouble()
        );
    }
    return std::nullopt;
}

std::vector<PricingConfig> ParkingRepository::getAllPricingConfigs() {
    std::vector<PricingConfig> list;
    QSqlQuery query("SELECT vehicle_type, type_name, hourly_rate, monthly_rate FROM pricing_config ORDER BY vehicle_type ASC;",
                    DatabaseManager::instance().getDatabase());
    while (query.next()) {
        list.emplace_back(
            static_cast<VehicleType>(query.value(0).toInt()),
            query.value(1).toString(),
            query.value(2).toDouble(),
            query.value(3).toDouble()
        );
    }
    return list;
}

bool ParkingRepository::updatePricingConfig(const PricingConfig& config) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        UPDATE pricing_config
        SET hourly_rate = ?, monthly_rate = ?, updated_at = ?
        WHERE vehicle_type = ?;
    )");
    query.bindValue(0, config.getHourlyRate());
    query.bindValue(1, config.getMonthlyRate());
    query.bindValue(2, QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(3, static_cast<int>(config.getVehicleType()));
    return query.exec();
}

double ParkingRepository::getDailyRevenue(const QDate& date) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        SELECT COALESCE(SUM(total_fee), 0.0)
        FROM tickets
        WHERE status = 'COMPLETED' AND date(check_out_time) = date(?);
    )");
    query.bindValue(0, date.toString(Qt::ISODate));
    double ticketRev = 0.0;
    if (query.exec() && query.next()) {
        ticketRev = query.value(0).toDouble();
    }

    QSqlQuery subQuery(DatabaseManager::instance().getDatabase());
    subQuery.prepare(R"(
        SELECT COALESCE(SUM(price_paid), 0.0)
        FROM monthly_subscriptions
        WHERE date(created_at) = date(?);
    )");
    subQuery.bindValue(0, date.toString(Qt::ISODate));
    double subRev = 0.0;
    if (subQuery.exec() && subQuery.next()) {
        subRev = subQuery.value(0).toDouble();
    }

    return ticketRev + subRev;
}

double ParkingRepository::getMonthlyRevenue(int year, int month) {
    QString monthStr = QString("%1-%2").arg(year).arg(month, 2, 10, QChar('0'));
    
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        SELECT COALESCE(SUM(total_fee), 0.0)
        FROM tickets
        WHERE status = 'COMPLETED' AND strftime('%Y-%m', check_out_time) = ?;
    )");
    query.bindValue(0, monthStr);
    double ticketRev = 0.0;
    if (query.exec() && query.next()) {
        ticketRev = query.value(0).toDouble();
    }

    QSqlQuery subQuery(DatabaseManager::instance().getDatabase());
    subQuery.prepare(R"(
        SELECT COALESCE(SUM(price_paid), 0.0)
        FROM monthly_subscriptions
        WHERE strftime('%Y-%m', created_at) = ?;
    )");
    subQuery.bindValue(0, monthStr);
    double subRev = 0.0;
    if (subQuery.exec() && subQuery.next()) {
        subRev = subQuery.value(0).toDouble();
    }

    return ticketRev + subRev;
}

// =============================================================================
// PARKING SESSIONS (PROMPT 1, 2, 3)
// =============================================================================

std::optional<ParkingSession> ParkingRepository::createSession(const QString& licensePlate, VehicleType type,
                                                           const QDateTime& checkInTime, double fee,
                                                           const QString& status) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        INSERT INTO ParkingSessions (license_plate, vehicle_type, check_in_time, total_fee, status)
        VALUES (?, ?, ?, ?, ?);
    )");
    query.bindValue(0, licensePlate.trimmed().toUpper());
    query.bindValue(1, static_cast<int>(type));
    query.bindValue(2, checkInTime.toString(Qt::ISODate));
    query.bindValue(3, fee);
    query.bindValue(4, status);

    if (query.exec()) {
        int id = query.lastInsertId().toInt();
        return ParkingSession(id, licensePlate.trimmed().toUpper(), type, checkInTime, QDateTime(), fee, status);
    }
    qWarning() << "Lỗi createSession:" << query.lastError().text();
    return std::nullopt;
}

std::optional<ParkingSession> ParkingRepository::findActiveSessionByPlate(const QString& licensePlate) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        SELECT id, license_plate, vehicle_type, check_in_time, check_out_time, total_fee, status
        FROM ParkingSessions
        WHERE license_plate = ? AND status = 'Đang đỗ'
        ORDER BY id DESC
        LIMIT 1;
    )");
    query.bindValue(0, licensePlate.trimmed().toUpper());
    if (query.exec() && query.next()) {
        return ParkingSession(
            query.value(0).toInt(),
            query.value(1).toString(),
            static_cast<VehicleType>(query.value(2).toInt()),
            QDateTime::fromString(query.value(3).toString(), Qt::ISODate),
            query.value(4).isNull() ? QDateTime() : QDateTime::fromString(query.value(4).toString(), Qt::ISODate),
            query.value(5).toDouble(),
            query.value(6).toString()
        );
    }
    return std::nullopt;
}

std::optional<ParkingSession> ParkingRepository::getSessionById(int id) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        SELECT id, license_plate, vehicle_type, check_in_time, check_out_time, total_fee, status
        FROM ParkingSessions
        WHERE id = ?;
    )");
    query.bindValue(0, id);
    if (query.exec() && query.next()) {
        return ParkingSession(
            query.value(0).toInt(),
            query.value(1).toString(),
            static_cast<VehicleType>(query.value(2).toInt()),
            QDateTime::fromString(query.value(3).toString(), Qt::ISODate),
            query.value(4).isNull() ? QDateTime() : QDateTime::fromString(query.value(4).toString(), Qt::ISODate),
            query.value(5).toDouble(),
            query.value(6).toString()
        );
    }
    return std::nullopt;
}

std::vector<ParkingSession> ParkingRepository::getAllActiveSessions() {
    std::vector<ParkingSession> list;
    QSqlQuery query("SELECT id, license_plate, vehicle_type, check_in_time, check_out_time, total_fee, status FROM ParkingSessions WHERE status = 'Đang đỗ' ORDER BY id DESC;",
                    DatabaseManager::instance().getDatabase());
    while (query.next()) {
        list.emplace_back(
            query.value(0).toInt(),
            query.value(1).toString(),
            static_cast<VehicleType>(query.value(2).toInt()),
            QDateTime::fromString(query.value(3).toString(), Qt::ISODate),
            query.value(4).isNull() ? QDateTime() : QDateTime::fromString(query.value(4).toString(), Qt::ISODate),
            query.value(5).toDouble(),
            query.value(6).toString()
        );
    }
    return list;
}

std::vector<ParkingSession> ParkingRepository::getAllSessions() {
    std::vector<ParkingSession> list;
    QSqlQuery query("SELECT id, license_plate, vehicle_type, check_in_time, check_out_time, total_fee, status FROM ParkingSessions ORDER BY id DESC;",
                    DatabaseManager::instance().getDatabase());
    while (query.next()) {
        list.emplace_back(
            query.value(0).toInt(),
            query.value(1).toString(),
            static_cast<VehicleType>(query.value(2).toInt()),
            QDateTime::fromString(query.value(3).toString(), Qt::ISODate),
            query.value(4).isNull() ? QDateTime() : QDateTime::fromString(query.value(4).toString(), Qt::ISODate),
            query.value(5).toDouble(),
            query.value(6).toString()
        );
    }
    return list;
}

bool ParkingRepository::updateSessionPayment(int sessionId, const QDateTime& checkOutTime, double fee, const QString& status) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        UPDATE ParkingSessions
        SET check_out_time = ?, total_fee = ?, status = ?
        WHERE id = ?;
    )");
    query.bindValue(0, checkOutTime.toString(Qt::ISODate));
    query.bindValue(1, fee);
    query.bindValue(2, status);
    query.bindValue(3, sessionId);
    return query.exec();
}

// =============================================================================
// MONTHLY PASSES (PROMPT 3)
// =============================================================================

bool ParkingRepository::addMonthlyPass(const MonthlyPass& pass) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        INSERT INTO MonthlyPasses (customer_name, license_plate, vehicle_type, start_date, expiration_date)
        VALUES (?, ?, ?, ?, ?);
    )");
    query.bindValue(0, pass.getCustomerName());
    query.bindValue(1, pass.getLicensePlate());
    query.bindValue(2, static_cast<int>(pass.getVehicleType()));
    query.bindValue(3, pass.getStartDate().toString(Qt::ISODate));
    query.bindValue(4, pass.getExpirationDate().toString(Qt::ISODate));
    return query.exec();
}

bool ParkingRepository::updateMonthlyPass(const MonthlyPass& pass) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        UPDATE MonthlyPasses
        SET customer_name = ?, vehicle_type = ?, start_date = ?, expiration_date = ?
        WHERE id = ?;
    )");
    query.bindValue(0, pass.getCustomerName());
    query.bindValue(1, static_cast<int>(pass.getVehicleType()));
    query.bindValue(2, pass.getStartDate().toString(Qt::ISODate));
    query.bindValue(3, pass.getExpirationDate().toString(Qt::ISODate));
    query.bindValue(4, pass.getId());
    return query.exec();
}

bool ParkingRepository::deleteMonthlyPass(int id) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("DELETE FROM MonthlyPasses WHERE id = ?;");
    query.bindValue(0, id);
    return query.exec();
}

std::vector<MonthlyPass> ParkingRepository::getAllMonthlyPasses() {
    std::vector<MonthlyPass> list;
    QSqlQuery query("SELECT id, customer_name, license_plate, vehicle_type, start_date, expiration_date FROM MonthlyPasses ORDER BY id DESC;",
                    DatabaseManager::instance().getDatabase());
    while (query.next()) {
        list.emplace_back(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString(),
            static_cast<VehicleType>(query.value(3).toInt()),
            QDate::fromString(query.value(4).toString(), Qt::ISODate),
            QDate::fromString(query.value(5).toString(), Qt::ISODate)
        );
    }
    return list;
}

std::optional<MonthlyPass> ParkingRepository::findValidMonthlyPass(const QString& licensePlate, const QDate& onDate) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare(R"(
        SELECT id, customer_name, license_plate, vehicle_type, start_date, expiration_date
        FROM MonthlyPasses
        WHERE license_plate = ? AND start_date <= ? AND expiration_date >= ?
        LIMIT 1;
    )");
    query.bindValue(0, licensePlate.trimmed().toUpper());
    query.bindValue(1, onDate.toString(Qt::ISODate));
    query.bindValue(2, onDate.toString(Qt::ISODate));
    if (query.exec() && query.next()) {
        return MonthlyPass(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString(),
            static_cast<VehicleType>(query.value(3).toInt()),
            QDate::fromString(query.value(4).toString(), Qt::ISODate),
            QDate::fromString(query.value(5).toString(), Qt::ISODate)
        );
    }
    return std::nullopt;
}

// =============================================================================
// USERS & RBAC (PROMPT 4)
// =============================================================================

std::optional<User> ParkingRepository::findUserByUsername(const QString& username) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("SELECT id, username, password_hash, role FROM Users WHERE username = ?;");
    query.bindValue(0, username.trimmed());
    if (query.exec() && query.next()) {
        return User(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toString()
        );
    }
    return std::nullopt;
}

std::optional<User> ParkingRepository::verifyUserCredentials(const QString& username, const QString& passwordHash) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("SELECT id, username, password_hash, role FROM Users WHERE username = ? AND password_hash = ?;");
    query.bindValue(0, username.trimmed());
    query.bindValue(1, passwordHash);
    if (query.exec() && query.next()) {
        return User(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toString()
        );
    }
    return std::nullopt;
}

bool ParkingRepository::updateUserPasswordHash(int userId, const QString& newHash) {
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("UPDATE Users SET password_hash = ? WHERE id = ?;");
    query.bindValue(0, newHash);
    query.bindValue(1, userId);
    return query.exec();
}

// =============================================================================
// PRICING MODEL (PROMPT 5)
// =============================================================================

PricingModel ParkingRepository::getPricingModel() {
    PricingModel model;
    QSqlQuery query("SELECT vehicle_type, first_block_fee, next_block_fee FROM pricing_config;",
                    DatabaseManager::instance().getDatabase());
    while (query.next()) {
        VehicleType t = static_cast<VehicleType>(query.value(0).toInt());
        double first = query.value(1).toDouble();
        double next = query.value(2).toDouble();
        model.setRate(t, first, next);
    }
    return model;
}

bool ParkingRepository::savePricingModel(const PricingModel& model) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    if (!db.transaction()) {
        return false;
    }
    QString now = QDateTime::currentDateTime().toString(Qt::ISODate);

    for (const auto& r : model.getAllRates()) {
        QSqlQuery query(db);
        query.prepare(R"(
            INSERT INTO pricing_config (vehicle_type, type_name, first_block_fee, next_block_fee, updated_at)
            VALUES (?, ?, ?, ?, ?)
            ON CONFLICT(vehicle_type) DO UPDATE SET
                type_name = excluded.type_name,
                first_block_fee = excluded.first_block_fee,
                next_block_fee = excluded.next_block_fee,
                updated_at = excluded.updated_at;
        )");
        query.bindValue(0, static_cast<int>(r.vehicleType));
        query.bindValue(1, r.typeName);
        query.bindValue(2, r.firstBlockFee);
        query.bindValue(3, r.nextBlockFee);
        query.bindValue(4, now);
        if (!query.exec()) {
            db.rollback();
            return false;
        }
    }
    return db.commit();
}

