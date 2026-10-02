#ifndef PARKING_REPOSITORY_H
#define PARKING_REPOSITORY_H

#include "../models/VehicleType.h"
#include "../models/ParkingSlot.h"
#include "../models/Ticket.h"
#include "../models/MonthlySubscription.h"
#include "../models/PricingConfig.h"
#include "../models/ParkingSession.h"
#include "../models/MonthlyPass.h"
#include "../models/User.h"
#include "../models/PricingModel.h"

#include <vector>
#include <optional>
#include <QDate>
#include <QDateTime>

class ParkingRepository {
public:
    ParkingRepository() = default;

    // --- Slot Management ---
    std::optional<ParkingSlot> findAvailableSlot(SlotType type);
    std::vector<ParkingSlot> getAllSlots();
    int countTotalSlots(SlotType type);
    int countOccupiedSlots(SlotType type);
    bool updateSlotOccupancy(int slotId, bool occupied, const QString& licensePlate = "");

    // --- ParkingSessions (Prompt 1, 2, 3) ---
    std::optional<ParkingSession> createSession(const QString& licensePlate, VehicleType type,
                                               const QDateTime& checkInTime, double fee = 0.0,
                                               const QString& status = QStringLiteral("Đang đỗ"));
    std::optional<ParkingSession> findActiveSessionByPlate(const QString& licensePlate);
    std::optional<ParkingSession> getSessionById(int id);
    std::vector<ParkingSession> getAllActiveSessions();
    std::vector<ParkingSession> getAllSessions();
    bool updateSessionPayment(int sessionId, const QDateTime& checkOutTime, double fee,
                              const QString& status = QStringLiteral("Đã thanh toán"));

    // --- MonthlyPasses (Prompt 3) ---
    bool addMonthlyPass(const MonthlyPass& pass);
    bool updateMonthlyPass(const MonthlyPass& pass);
    bool deleteMonthlyPass(int id);
    std::vector<MonthlyPass> getAllMonthlyPasses();
    std::optional<MonthlyPass> findValidMonthlyPass(const QString& licensePlate, const QDate& onDate = QDate::currentDate());

    // --- Users & RBAC (Prompt 4) ---
    std::optional<User> findUserByUsername(const QString& username);
    std::optional<User> verifyUserCredentials(const QString& username, const QString& passwordHash);

    // --- PricingModel (Prompt 5) ---
    PricingModel getPricingModel();
    bool savePricingModel(const PricingModel& model);

    // --- Legacy Ticket & Pricing compatibility ---
    std::optional<Ticket> createTicket(const QString& licensePlate, VehicleType vType, int slotId, const QDateTime& checkInTime);
    std::optional<Ticket> getActiveTicketByPlate(const QString& licensePlate);
    std::vector<Ticket> getAllActiveTickets();
    bool completeTicket(int ticketId, const QDateTime& checkOutTime, double fee, const QString& pricingType);
    std::vector<Ticket> searchTicketHistory(const QString& licensePlate = "");

    bool saveSubscription(const MonthlySubscription& sub);
    std::optional<MonthlySubscription> findActiveSubscription(const QString& licensePlate, const QDate& onDate);
    std::vector<MonthlySubscription> getAllSubscriptions();

    std::optional<PricingConfig> getPricingConfig(VehicleType type);
    std::vector<PricingConfig> getAllPricingConfigs();
    bool updatePricingConfig(const PricingConfig& config);

    double getDailyRevenue(const QDate& date);
    double getMonthlyRevenue(int year, int month);
};

#endif // PARKING_REPOSITORY_H
