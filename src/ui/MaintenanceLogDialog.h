#ifndef MAINTENANCE_LOG_DIALOG_H
#define MAINTENANCE_LOG_DIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <memory>
#include "../services/HardwareController.h"
#include "../services/CameraService.h"

class MaintenanceLogDialog : public QDialog {
    Q_OBJECT

private:
    std::shared_ptr<HardwareController> m_hardware;
    std::shared_ptr<CameraService> m_camera;

    QLabel* m_camStatusLabel;
    QLabel* m_barrierStatusLabel;
    QLabel* m_dbStatusLabel;

    QTextEdit* m_logViewer;
    QPushButton* m_emergencyOpenBtn;
    QPushButton* m_clearLogsBtn;
    QPushButton* m_closeBtn;

public:
    explicit MaintenanceLogDialog(std::shared_ptr<HardwareController> hardware,
                                 std::shared_ptr<CameraService> camera,
                                 QWidget* parent = nullptr);

public slots:
    void appendLog(const QString& log);

private slots:
    void onEmergencyOpenClicked();
    void updateHardwareStatus();

private:
    void setupUi();
};

#endif // MAINTENANCE_LOG_DIALOG_H
