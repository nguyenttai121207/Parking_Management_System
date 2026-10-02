#ifndef SELF_CHECKOUT_DIALOG_H
#define SELF_CHECKOUT_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <memory>
#include "../db/ParkingRepository.h"
#include "../services/HardwareController.h"
#include "../services/BankWebhookSimulator.h"
#include "../models/ParkingSession.h"
#include "../models/PricingModel.h"

class SelfCheckoutDialog : public QDialog {
    Q_OBJECT

private:
    ParkingRepository m_repo;
    std::shared_ptr<HardwareController> m_hardware;
    std::unique_ptr<BankWebhookSimulator> m_webhookSimulator;

    QLineEdit* m_plateSearchEdit;
    QPushButton* m_searchBtn;
    QPushButton* m_rfidMockBtn;

    // Session Info UI
    QFrame* m_infoFrame;
    QLabel* m_sessionIdLabel;
    QLabel* m_plateValLabel;
    QLabel* m_typeValLabel;
    QLabel* m_timeInLabel;
    QLabel* m_durationLabel;
    QLabel* m_amountLabel;

    // QR Payment UI
    QFrame* m_qrFrame;
    QLabel* m_qrCodeImgLabel;
    QLabel* m_transferMemoLabel;
    QLabel* m_webhookStatusLabel;
    QPushButton* m_simulatePaidBtn;

    // Thank you message
    QLabel* m_thankYouLabel;

    std::optional<ParkingSession> m_currentSession;
    double m_currentFee;

public:
    explicit SelfCheckoutDialog(std::shared_ptr<HardwareController> hardware, QWidget* parent = nullptr);
    ~SelfCheckoutDialog() override;

private slots:
    void onSearchClicked();
    void onRfidMockClicked();
    void onSimulatePaymentClicked();
    void onPaymentConfirmed(int sessionId, double amount, const QString& transId);

private:
    void setupUi();
    void displaySessionDetails(const ParkingSession& session);
    QImage generateVietQRImage(int sessionId, double amount, const QString& memo);
};

#endif // SELF_CHECKOUT_DIALOG_H
