#ifndef PARKING_MANAGER_H
#define PARKING_MANAGER_H

#include "../db/ParkingRepository.h"
#include "../services/PricingService.h"
#include "../models/Vehicle.h"
#include <QString>
#include <memory>
#include <vector>

struct CheckInResult {
    bool success;
    QString message;
    std::optional<Ticket> ticket;
};

struct CheckOutReceipt {
    bool success;
    QString message;
    Ticket ticket;
    QDateTime checkOutTime;
    int billableHours;
    double fee;
    QString pricingType;
    bool isMonthlySubscriber;
};

struct DashboardStats {
    int totalMotorbikeSlots;
    int occupiedMotorbikeSlots;
    int totalCarSlots;
    int occupiedCarSlots;
    int activeVehiclesCount;
    double todayRevenue;
};

class ParkingManager {
private:
    ParkingRepository m_repo;
    PricingService m_pricingService;

public:
    ParkingManager();

    CheckInResult checkIn(VehicleType type, const QString& licensePlate);

    CheckOutReceipt previewCheckOut(const QString& licensePlate);
    bool confirmCheckOut(const CheckOutReceipt& receipt);

    bool registerSubscription(const QString& licensePlate, VehicleType type,
                              const QString& customerName, const QString& phone,
                              int durationMonths, QString& outMessage);

    DashboardStats getDashboardStats();
    std::vector<ParkingSlot> getAllSlots();
    std::vector<Ticket> getAllActiveTickets();
    std::vector<Ticket> searchHistory(const QString& licensePlate = "");
    std::vector<MonthlySubscription> getAllSubscriptions();

    std::vector<PricingConfig> getPricingConfigs();
    bool updatePricingConfig(const PricingConfig& config);
};

#endif // PARKING_MANAGER_H
