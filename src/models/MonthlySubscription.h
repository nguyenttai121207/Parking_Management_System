#ifndef MONTHLY_SUBSCRIPTION_H
#define MONTHLY_SUBSCRIPTION_H

#include "VehicleType.h"
#include <QString>
#include <QDate>

class MonthlySubscription {
private:
    int m_subscriptionId;
    QString m_licensePlate;
    VehicleType m_vehicleType;
    QString m_customerName;
    QString m_phoneNumber;
    QDate m_startDate;
    QDate m_endDate;
    double m_pricePaid;

public:
    MonthlySubscription()
        : m_subscriptionId(0), m_vehicleType(VehicleType::MotorbikeGas), m_pricePaid(0.0) {}

    MonthlySubscription(int id, const QString& licensePlate, VehicleType vType, 
                        const QString& customerName, const QString& phone,
                        const QDate& startDate, const QDate& endDate, double pricePaid)
        : m_subscriptionId(id), m_licensePlate(licensePlate), m_vehicleType(vType),
          m_customerName(customerName), m_phoneNumber(phone),
          m_startDate(startDate), m_endDate(endDate), m_pricePaid(pricePaid) {}

    int getSubscriptionId() const { return m_subscriptionId; }
    void setSubscriptionId(int id) { m_subscriptionId = id; }

    QString getLicensePlate() const { return m_licensePlate; }
    void setLicensePlate(const QString& plate) { m_licensePlate = plate; }

    VehicleType getVehicleType() const { return m_vehicleType; }
    void setVehicleType(VehicleType type) { m_vehicleType = type; }

    QString getCustomerName() const { return m_customerName; }
    void setCustomerName(const QString& name) { m_customerName = name; }

    QString getPhoneNumber() const { return m_phoneNumber; }
    void setPhoneNumber(const QString& phone) { m_phoneNumber = phone; }

    QDate getStartDate() const { return m_startDate; }
    void setStartDate(const QDate& date) { m_startDate = date; }

    QDate getEndDate() const { return m_endDate; }
    void setEndDate(const QDate& date) { m_endDate = date; }

    double getPricePaid() const { return m_pricePaid; }
    void setPricePaid(double price) { m_pricePaid = price; }

    bool isValidOn(const QDate& date) const {
        return date >= m_startDate && date <= m_endDate;
    }
};

#endif // MONTHLY_SUBSCRIPTION_H
