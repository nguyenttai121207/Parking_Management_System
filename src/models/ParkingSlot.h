#ifndef PARKING_SLOT_H
#define PARKING_SLOT_H

#include "VehicleType.h"
#include <QString>

class ParkingSlot {
private:
    int m_slotId;
    QString m_slotNumber;
    SlotType m_slotType;
    bool m_isOccupied;
    QString m_currentLicensePlate;

public:
    ParkingSlot() 
        : m_slotId(0), m_slotType(SlotType::MotorbikeSlot), m_isOccupied(false) {}

    ParkingSlot(int slotId, const QString& slotNumber, SlotType slotType, bool isOccupied = false, const QString& currentPlate = "")
        : m_slotId(slotId), m_slotNumber(slotNumber), m_slotType(slotType), 
          m_isOccupied(isOccupied), m_currentLicensePlate(currentPlate) {}

    int getSlotId() const { return m_slotId; }
    void setSlotId(int id) { m_slotId = id; }

    QString getSlotNumber() const { return m_slotNumber; }
    void setSlotNumber(const QString& number) { m_slotNumber = number; }

    SlotType getSlotType() const { return m_slotType; }
    void setSlotType(SlotType type) { m_slotType = type; }

    bool isOccupied() const { return m_isOccupied; }
    QString getCurrentLicensePlate() const { return m_currentLicensePlate; }

    void occupySlot(const QString& licensePlate) {
        m_isOccupied = true;
        m_currentLicensePlate = licensePlate;
    }

    void releaseSlot() {
        m_isOccupied = false;
        m_currentLicensePlate.clear();
    }
};

#endif // PARKING_SLOT_H
