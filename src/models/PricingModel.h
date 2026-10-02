#ifndef PRICING_MODEL_H
#define PRICING_MODEL_H

#include "VehicleType.h"
#include <QDateTime>
#include <map>
#include <QString>

struct PricingRate {
    VehicleType vehicleType;
    QString typeName;
    double firstBlockFee; // Phí block đầu (VNĐ)
    double nextBlockFee;  // Phí block tiếp theo (VNĐ)
};

class PricingModel {
private:
    std::map<VehicleType, PricingRate> m_rates;

public:
    PricingModel();

    void setRate(VehicleType type, double firstBlock, double nextBlock);
    PricingRate getRate(VehicleType type) const;
    std::vector<PricingRate> getAllRates() const;

    // Hàm theo đúng yêu cầu Prompt 5:
    double calculateFee(VehicleType type, const QDateTime& checkInTime, const QDateTime& checkOutTime) const;
};

#endif // PRICING_MODEL_H
