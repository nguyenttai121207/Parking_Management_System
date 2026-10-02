#include "PricingModel.h"
#include <algorithm>
#include <cmath>

PricingModel::PricingModel() {
    // Khởi tạo giá trị mặc định cho 4 loại xe theo yêu cầu
    m_rates[VehicleType::Bicycle] = {VehicleType::Bicycle, QStringLiteral("Xe đạp"), 2000.0, 1000.0};
    m_rates[VehicleType::MotorbikeManual] = {VehicleType::MotorbikeManual, QStringLiteral("Xe máy số"), 4000.0, 2000.0};
    m_rates[VehicleType::MotorbikeScooter] = {VehicleType::MotorbikeScooter, QStringLiteral("Xe tay ga"), 5000.0, 3000.0};
    m_rates[VehicleType::Car] = {VehicleType::Car, QStringLiteral("Ô tô con"), 25000.0, 15000.0};
}

void PricingModel::setRate(VehicleType type, double firstBlock, double nextBlock) {
    m_rates[type] = {type, VehicleUtils::getVehicleTypeName(type), firstBlock, nextBlock};
}

PricingRate PricingModel::getRate(VehicleType type) const {
    auto it = m_rates.find(type);
    if (it != m_rates.end()) {
        return it->second;
    }
    return {type, VehicleUtils::getVehicleTypeName(type), 5000.0, 3000.0};
}

std::vector<PricingRate> PricingModel::getAllRates() const {
    std::vector<PricingRate> list;
    VehicleType types[] = {
        VehicleType::Bicycle,
        VehicleType::MotorbikeManual,
        VehicleType::MotorbikeScooter,
        VehicleType::Car
    };
    for (auto t : types) {
        list.push_back(getRate(t));
    }
    return list;
}

double PricingModel::calculateFee(VehicleType type, const QDateTime& checkInTime, const QDateTime& checkOutTime) const {
    PricingRate rate = getRate(type);

    QDateTime end = (checkOutTime.isValid() && checkOutTime >= checkInTime) ? checkOutTime : checkInTime.addSecs(60);
    qint64 totalSeconds = checkInTime.secsTo(end);
    qint64 totalMinutes = std::max<qint64>(1, totalSeconds / 60);

    // Block đầu tiên: tối đa 60 phút
    if (totalMinutes <= 60) {
        return rate.firstBlockFee;
    }

    // Các block tiếp theo: mỗi 60 phút (hoặc phần lẻ) tính thêm 1 block
    qint64 remainingMinutes = totalMinutes - 60;
    qint64 additionalBlocks = (remainingMinutes + 59) / 60;

    return rate.firstBlockFee + (additionalBlocks * rate.nextBlockFee);
}
