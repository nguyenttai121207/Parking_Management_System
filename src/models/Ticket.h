#ifndef TICKET_H
#define TICKET_H

#include "VehicleType.h"
#include <QString>
#include <QDateTime>

class Ticket {
public:
    enum class Status {
        Active,
        Completed
    };

private:
    int m_ticketId;
    QString m_licensePlate;
    VehicleType m_vehicleType;
    int m_slotId;
    QString m_slotNumber;
    QDateTime m_checkInTime;
    QDateTime m_checkOutTime;
    QString m_pricingType;
    double m_totalFee;
    Status m_status;

public:
    Ticket()
        : m_ticketId(0), m_vehicleType(VehicleType::MotorbikeGas), m_slotId(0),
          m_pricingType("HOURLY"), m_totalFee(0.0), m_status(Status::Active) {}

    Ticket(int ticketId, const QString& licensePlate, VehicleType vType, int slotId, 
           const QString& slotNumber, const QDateTime& checkInTime)
        : m_ticketId(ticketId), m_licensePlate(licensePlate), m_vehicleType(vType),
          m_slotId(slotId), m_slotNumber(slotNumber), m_checkInTime(checkInTime),
          m_pricingType("HOURLY"), m_totalFee(0.0), m_status(Status::Active) {}

    int getTicketId() const { return m_ticketId; }
    void setTicketId(int id) { m_ticketId = id; }

    QString getLicensePlate() const { return m_licensePlate; }
    void setLicensePlate(const QString& plate) { m_licensePlate = plate; }

    VehicleType getVehicleType() const { return m_vehicleType; }
    void setVehicleType(VehicleType type) { m_vehicleType = type; }

    int getSlotId() const { return m_slotId; }
    void setSlotId(int id) { m_slotId = id; }

    QString getSlotNumber() const { return m_slotNumber; }
    void setSlotNumber(const QString& number) { m_slotNumber = number; }

    QDateTime getCheckInTime() const { return m_checkInTime; }
    void setCheckInTime(const QDateTime& time) { m_checkInTime = time; }

    QDateTime getCheckOutTime() const { return m_checkOutTime; }
    void setCheckOutTime(const QDateTime& time) { m_checkOutTime = time; }

    QString getPricingType() const { return m_pricingType; }
    void setPricingType(const QString& type) { m_pricingType = type; }

    double getTotalFee() const { return m_totalFee; }
    void setTotalFee(double fee) { m_totalFee = fee; }

    Status getStatus() const { return m_status; }
    void setStatus(Status status) { m_status = status; }

    bool isActive() const { return m_status == Status::Active; }
};

#endif // TICKET_H
