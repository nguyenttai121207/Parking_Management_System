#ifndef PARKING_SESSION_H
#define PARKING_SESSION_H

#include "VehicleType.h"
#include <QString>
#include <QDateTime>

class ParkingSession {
private:
    int m_id;
    QString m_licensePlate;
    VehicleType m_vehicleType;
    QDateTime m_checkInTime;
    QDateTime m_checkOutTime;
    double m_totalFee;
    QString m_status; // "Đang đỗ", "Đã thanh toán", "Đã rời bãi"

public:
    ParkingSession()
        : m_id(0), m_vehicleType(VehicleType::MotorbikeManual), m_totalFee(0.0), m_status(QStringLiteral("Đang đỗ")) {}

    ParkingSession(int id, const QString& licensePlate, VehicleType type,
                   const QDateTime& checkInTime, const QDateTime& checkOutTime = QDateTime(),
                   double totalFee = 0.0, const QString& status = QStringLiteral("Đang đỗ"))
        : m_id(id), m_licensePlate(licensePlate), m_vehicleType(type),
          m_checkInTime(checkInTime), m_checkOutTime(checkOutTime),
          m_totalFee(totalFee), m_status(status) {}

    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString getLicensePlate() const { return m_licensePlate; }
    void setLicensePlate(const QString& plate) { m_licensePlate = plate; }

    VehicleType getVehicleType() const { return m_vehicleType; }
    void setVehicleType(VehicleType type) { m_vehicleType = type; }

    QDateTime getCheckInTime() const { return m_checkInTime; }
    void setCheckInTime(const QDateTime& time) { m_checkInTime = time; }

    QDateTime getCheckOutTime() const { return m_checkOutTime; }
    void setCheckOutTime(const QDateTime& time) { m_checkOutTime = time; }

    double getTotalFee() const { return m_totalFee; }
    void setTotalFee(double fee) { m_totalFee = fee; }

    QString getStatus() const { return m_status; }
    void setStatus(const QString& status) { m_status = status; }

    bool isActive() const { return m_status == QStringLiteral("Đang đỗ"); }
};

#endif // PARKING_SESSION_H
