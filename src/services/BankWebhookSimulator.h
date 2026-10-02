#ifndef BANK_WEBHOOK_SIMULATOR_H
#define BANK_WEBHOOK_SIMULATOR_H

#include <QThread>
#include <QString>
#include <QMutex>

class BankWebhookSimulator : public QThread {
    Q_OBJECT
private:
    int m_sessionId;
    double m_amount;
    int m_delaySeconds;
    bool m_stopRequested;
    bool m_forcePayment;
    QMutex m_mutex;

public:
    explicit BankWebhookSimulator(QObject* parent = nullptr);
    ~BankWebhookSimulator() override;

    void startListening(int sessionId, double amount, int delaySeconds = 4);
    void triggerInstantPayment();
    void stopListening();

signals:
    void listeningStarted(int sessionId, double amount);
    void paymentConfirmed(int sessionId, double amount, const QString& transactionId);

protected:
    void run() override;
};

#endif // BANK_WEBHOOK_SIMULATOR_H
