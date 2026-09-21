#include "Vehicle.h"
#include <QRegularExpression>

Vehicle::Vehicle(const QString& licensePlate) 
    : m_licensePlate(licensePlate.trimmed().toUpper()) {
}

QString Vehicle::getLicensePlate() const {
    return m_licensePlate;
}

void Vehicle::setLicensePlate(const QString& plate) {
    m_licensePlate = plate.trimmed().toUpper();
}

bool Vehicle::validateLicensePlate() const {
    static const QRegularExpression regex("^[0-9]{2}[A-Z0-9\\-]{2,5}[0-9\\.]{4,7}$");
    return !m_licensePlate.isEmpty() && regex.match(m_licensePlate).hasMatch();
}

MotorbikeGas::MotorbikeGas(const QString& licensePlate) : Vehicle(licensePlate) {}
VehicleType MotorbikeGas::getType() const { return VehicleType::MotorbikeGas; }
QString MotorbikeGas::getDisplayName() const { return QStringLiteral("Xe máy xăng"); }
SlotType MotorbikeGas::getRequiredSlotType() const { return SlotType::MotorbikeSlot; }
bool MotorbikeGas::validateLicensePlate() const { return Vehicle::validateLicensePlate(); }

MotorbikeElectric::MotorbikeElectric(const QString& licensePlate) : Vehicle(licensePlate) {}
VehicleType MotorbikeElectric::getType() const { return VehicleType::MotorbikeElectric; }
QString MotorbikeElectric::getDisplayName() const { return QStringLiteral("Xe máy điện"); }
SlotType MotorbikeElectric::getRequiredSlotType() const { return SlotType::MotorbikeSlot; }
bool MotorbikeElectric::validateLicensePlate() const { return Vehicle::validateLicensePlate(); }

CarGas::CarGas(const QString& licensePlate) : Vehicle(licensePlate) {}
VehicleType CarGas::getType() const { return VehicleType::CarGas; }
QString CarGas::getDisplayName() const { return QStringLiteral("Ô tô xăng"); }
SlotType CarGas::getRequiredSlotType() const { return SlotType::CarSlot; }
bool CarGas::validateLicensePlate() const { return Vehicle::validateLicensePlate(); }

CarElectric::CarElectric(const QString& licensePlate) : Vehicle(licensePlate) {}
VehicleType CarElectric::getType() const { return VehicleType::CarElectric; }
QString CarElectric::getDisplayName() const { return QStringLiteral("Ô tô điện"); }
SlotType CarElectric::getRequiredSlotType() const { return SlotType::CarSlot; }
bool CarElectric::validateLicensePlate() const { return Vehicle::validateLicensePlate(); }

std::unique_ptr<Vehicle> VehicleFactory::createVehicle(VehicleType type, const QString& licensePlate) {
    switch (type) {
        case VehicleType::MotorbikeGas:
            return std::make_unique<MotorbikeGas>(licensePlate);
        case VehicleType::MotorbikeElectric:
            return std::make_unique<MotorbikeElectric>(licensePlate);
        case VehicleType::CarGas:
            return std::make_unique<CarGas>(licensePlate);
        case VehicleType::CarElectric:
            return std::make_unique<CarElectric>(licensePlate);
    }
    return nullptr;
}
