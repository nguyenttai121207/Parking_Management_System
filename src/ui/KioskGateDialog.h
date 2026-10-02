#ifndef KIOSK_GATE_DIALOG_H
#define KIOSK_GATE_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <memory>
#include "../services/AutoGateManager.h"

class KioskGateDialog : public QDialog {
    Q_OBJECT

private:
    std::shared_ptr<AutoGateManager> m_gateManager;
    QLabel* m_cameraLabel;
    QLabel* m_instructionLabel;
    QLabel* m_barrierStatusLabel;
    QLabel* m_plateResultLabel;
    QLabel* m_announcementLabel;
    QLineEdit* m_plateOverrideEdit;
    QPushButton* m_sensorBtn;
    QPushButton* m_closeBtn;

public:
    explicit KioskGateDialog(std::shared_ptr<AutoGateManager> gateManager, QWidget* parent = nullptr);
    ~KioskGateDialog() override;

private slots:
    void onFrameReceived(const QImage& frame);
    void onBarrierStatusChanged(bool isOpen, const QString& reason);
    void onCheckInSuccess(const QString& plate, bool isMonthly, const QString& message, double fee);
    void onCheckInFailed(const QString& plate, const QString& error);
    void onSensorTriggered();

private:
    void setupUi();
};

#endif // KIOSK_GATE_DIALOG_H
