#ifndef PRICING_CONFIG_H
#define PRICING_CONFIG_H

#include "VehicleType.h"
#include <QString>

class PricingConfig {
private:
    VehicleType m_vehicleType;
    QString m_typeName;
    double m_hourlyRate;
    double m_monthlyRate;

public:
    PricingConfig()
        : m_vehicleType(VehicleType::MotorbikeGas), m_hourlyRate(5000.0), m_monthlyRate(100000.0) {
        m_typeName = VehicleUtils::getVehicleTypeName(m_vehicleType);
    }

    PricingConfig(VehicleType type, double hourlyRate, double monthlyRate)
        : m_vehicleType(type), m_hourlyRate(hourlyRate), m_monthlyRate(monthlyRate) {
        m_typeName = VehicleUtils::getVehicleTypeName(type);
    }

    PricingConfig(VehicleType type, const QString& typeName, double hourlyRate, double monthlyRate)
        : m_vehicleType(type), m_typeName(typeName), m_hourlyRate(hourlyRate), m_monthlyRate(monthlyRate) {}

    VehicleType getVehicleType() const { return m_vehicleType; }
    void setVehicleType(VehicleType type) { 
        m_vehicleType = type; 
        m_typeName = VehicleUtils::getVehicleTypeName(type);
    }

    QString getTypeName() const { return m_typeName; }
    void setTypeName(const QString& name) { m_typeName = name; }

    double getHourlyRate() const { return m_hourlyRate; }
    void setHourlyRate(double rate) { m_hourlyRate = rate; }

    double getMonthlyRate() const { return m_monthlyRate; }
    void setMonthlyRate(double rate) { m_monthlyRate = rate; }
};

#endif // PRICING_CONFIG_H
