#ifndef VEHICLE_TYPE_H
#define VEHICLE_TYPE_H

#include <QString>

enum class VehicleType {
    Bicycle = 0,          // Xe đạp
    MotorbikeManual = 1,  // Xe máy số
    MotorbikeScooter = 2, // Xe tay ga
    Car = 3,              // Ô tô con

    // Tương thích ngược
    MotorbikeGas = 1,
    MotorbikeElectric = 2,
    CarGas = 3,
    CarElectric = 4
};

enum class SlotType {
    MotorbikeSlot = 0,
    CarSlot = 1
};

namespace VehicleUtils {
    inline QString getVehicleTypeName(VehicleType type) {
        switch (static_cast<int>(type)) {
            case 0: return QStringLiteral("Xe đạp");
            case 1: return QStringLiteral("Xe máy số");
            case 2: return QStringLiteral("Xe tay ga");
            case 3:
            case 4: return QStringLiteral("Ô tô con");
            default: break;
        }
        return QStringLiteral("Không xác định");
    }

    inline SlotType getSlotTypeForVehicle(VehicleType type) {
        if (static_cast<int>(type) <= 2) {
            return SlotType::MotorbikeSlot;
        }
        return SlotType::CarSlot;
    }

    inline QString getSlotTypeName(SlotType type) {
        switch (type) {
            case SlotType::MotorbikeSlot: return QStringLiteral("Chỗ xe máy/xe đạp");
            case SlotType::CarSlot:       return QStringLiteral("Chỗ ô tô");
        }
        return QStringLiteral("Không xác định");
    }
}

#endif // VEHICLE_TYPE_H
