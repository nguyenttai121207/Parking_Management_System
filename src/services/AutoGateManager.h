#ifndef AUTO_GATE_MANAGER_H
#define AUTO_GATE_MANAGER_H

#include <QObject>
#include <memory>
#include "CameraService.h"
#include "ANPRService.h"
#include "HardwareController.h"
#include "../db/ParkingRepository.h"

class AutoGateManager : public QObject {
    Q_OBJECT
private:
    std::shared_ptr<CameraService> m_camera;
    std::unique_ptr<IANPRService> m_anpr;
    std::shared_ptr<HardwareController> m_hardware;
    ParkingRepository m_repo;

public:
    explicit AutoGateManager(std::shared_ptr<CameraService> camera,
                             std::shared_ptr<HardwareController> hardware,
                             QObject* parent = nullptr);

    std::shared_ptr<CameraService> getCamera() const { return m_camera; }
    std::shared_ptr<HardwareController> getHardware() const { return m_hardware; }

public slots:
    void triggerVehicleEntry(const QString& plateOverride = "");

signals:
    void checkInCompleted(const QString& plate, bool isMonthly, const QString& message, double fee);
    void checkInFailed(const QString& plate, const QString& error);
    void vehicleProcessingStarted(const QString& plate);
};

#endif // AUTO_GATE_MANAGER_H
