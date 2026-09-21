#ifndef VEHICLE_H
#define VEHICLE_H

#include "VehicleType.h"
#include <QString>
#include <memory>

class Vehicle {
protected:
    QString m_licensePlate;

public:
    explicit Vehicle(const QString& licensePlate);
    virtual ~Vehicle() = default;

    QString getLicensePlate() const;
    void setLicensePlate(const QString& plate);

    virtual VehicleType getType() const = 0;
    virtual QString getDisplayName() const = 0;
    virtual SlotType getRequiredSlotType() const = 0;
    virtual bool validateLicensePlate() const;
};

class MotorbikeGas : public Vehicle {
public:
    explicit MotorbikeGas(const QString& licensePlate);
    ~MotorbikeGas() override = default;

    VehicleType getType() const override;
    QString getDisplayName() const override;
    SlotType getRequiredSlotType() const override;
    bool validateLicensePlate() const override;
};

class MotorbikeElectric : public Vehicle {
public:
    explicit MotorbikeElectric(const QString& licensePlate);
    ~MotorbikeElectric() override = default;

    VehicleType getType() const override;
    QString getDisplayName() const override;
    SlotType getRequiredSlotType() const override;
    bool validateLicensePlate() const override;
};

class CarGas : public Vehicle {
public:
    explicit CarGas(const QString& licensePlate);
    ~CarGas() override = default;

    VehicleType getType() const override;
    QString getDisplayName() const override;
    SlotType getRequiredSlotType() const override;
    bool validateLicensePlate() const override;
};

class CarElectric : public Vehicle {
public:
    explicit CarElectric(const QString& licensePlate);
    ~CarElectric() override = default;

    VehicleType getType() const override;
    QString getDisplayName() const override;
    SlotType getRequiredSlotType() const override;
    bool validateLicensePlate() const override;
};

class VehicleFactory {
public:
    static std::unique_ptr<Vehicle> createVehicle(VehicleType type, const QString& licensePlate);
};

#endif // VEHICLE_H
