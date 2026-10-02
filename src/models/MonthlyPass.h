#ifndef MONTHLY_PASS_H
#define MONTHLY_PASS_H

#include "VehicleType.h"
#include <QString>
#include <QDate>

class MonthlyPass {
private:
    int m_id;
    QString m_customerName;
    QString m_licensePlate;
    VehicleType m_vehicleType;
    QDate m_startDate;
    QDate m_expirationDate;

public:
    MonthlyPass()
        : m_id(0), m_vehicleType(VehicleType::MotorbikeManual) {}

    MonthlyPass(int id, const QString& customerName, const QString& licensePlate,
                VehicleType vehicleType, const QDate& startDate, const QDate& expirationDate)
        : m_id(id), m_customerName(customerName), m_licensePlate(licensePlate.trimmed().toUpper()),
          m_vehicleType(vehicleType), m_startDate(startDate), m_expirationDate(expirationDate) {}

    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString getCustomerName() const { return m_customerName; }
    void setCustomerName(const QString& name) { m_customerName = name; }

    QString getLicensePlate() const { return m_licensePlate; }
    void setLicensePlate(const QString& plate) { m_licensePlate = plate.trimmed().toUpper(); }

    VehicleType getVehicleType() const { return m_vehicleType; }
    void setVehicleType(VehicleType type) { m_vehicleType = type; }

    QDate getStartDate() const { return m_startDate; }
    void setStartDate(const QDate& date) { m_startDate = date; }

    QDate getExpirationDate() const { return m_expirationDate; }
    void setExpirationDate(const QDate& date) { m_expirationDate = date; }

    bool isValid(const QDate& onDate = QDate::currentDate()) const {
        return m_startDate <= onDate && onDate <= m_expirationDate;
    }
};

#endif // MONTHLY_PASS_H
