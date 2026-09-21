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
