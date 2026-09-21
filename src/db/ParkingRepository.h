#ifndef PARKING_REPOSITORY_H
#define PARKING_REPOSITORY_H

#include "../models/VehicleType.h"
#include "../models/ParkingSlot.h"
#include "../models/Ticket.h"
#include "../models/MonthlySubscription.h"
#include "../models/PricingConfig.h"
#include <vector>
#include <optional>
#include <QDate>
#include <QDateTime>

class ParkingRepository {
public:
    ParkingRepository() = default;

    std::optional<ParkingSlot> findAvailableSlot(SlotType type);
    std::vector<ParkingSlot> getAllSlots();
    int countTotalSlots(SlotType type);
    int countOccupiedSlots(SlotType type);
    bool updateSlotOccupancy(int slotId, bool occupied, const QString& licensePlate = "");

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
