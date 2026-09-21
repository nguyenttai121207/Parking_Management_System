#ifndef VEHICLE_TYPE_H
#define VEHICLE_TYPE_H

#include <QString>

enum class VehicleType {
    MotorbikeGas = 0,
    MotorbikeElectric = 1,
    CarGas = 2,
    CarElectric = 3
};

enum class SlotType {
    MotorbikeSlot = 0,
    CarSlot = 1
};

namespace VehicleUtils {
    inline QString getVehicleTypeName(VehicleType type) {
        switch (type) {
            case VehicleType::MotorbikeGas:      return QStringLiteral("Xe máy xăng");
            case VehicleType::MotorbikeElectric: return QStringLiteral("Xe máy điện");
            case VehicleType::CarGas:            return QStringLiteral("Ô tô xăng");
            case VehicleType::CarElectric:       return QStringLiteral("Ô tô điện");
        }
        return QStringLiteral("Không xác định");
    }

    inline SlotType getSlotTypeForVehicle(VehicleType type) {
        if (type == VehicleType::MotorbikeGas || type == VehicleType::MotorbikeElectric) {
            return SlotType::MotorbikeSlot;
        }
        return SlotType::CarSlot;
    }

    inline QString getSlotTypeName(SlotType type) {
        switch (type) {
            case SlotType::MotorbikeSlot: return QStringLiteral("Chỗ xe máy");
            case SlotType::CarSlot:       return QStringLiteral("Chỗ ô tô");
        }
        return QStringLiteral("Không xác định");
    }
}

#endif // VEHICLE_TYPE_H
