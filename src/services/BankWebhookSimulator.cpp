#include "BankWebhookSimulator.h"
#include <QDateTime>
#include <QRandomGenerator>

BankWebhookSimulator::BankWebhookSimulator(QObject* parent)
    : QThread(parent), m_sessionId(0), m_amount(0.0), m_delaySeconds(4),
      m_stopRequested(false), m_forcePayment(false) {}

BankWebhookSimulator::~BankWebhookSimulator() {
    stopListening();
    wait();
}

void BankWebhookSimulator::startListening(int sessionId, double amount, int delaySeconds) {
    stopListening();
    wait();

    m_sessionId = sessionId;
    m_amount = amount;
    m_delaySeconds = delaySeconds;
    m_stopRequested = false;
    m_forcePayment = false;

    emit listeningStarted(m_sessionId, m_amount);
    start();
}

void BankWebhookSimulator::triggerInstantPayment() {
    QMutexLocker locker(&m_mutex);
    m_forcePayment = true;
}

void BankWebhookSimulator::stopListening() {
    QMutexLocker locker(&m_mutex);
    m_stopRequested = true;
}

void BankWebhookSimulator::run() {
    int elapsed = 0;
    const int checkIntervalMs = 200;
    const int totalMs = m_delaySeconds * 1000;

    while (elapsed < totalMs) {
        {
            QMutexLocker locker(&m_mutex);
            if (m_stopRequested) {
                return;
            }
            if (m_forcePayment) {
                break;
            }
        }
        msleep(checkIntervalMs);
        elapsed += checkIntervalMs;
    }

    // Tạo mã giao dịch ngân hàng giả lập (FTxxxxxxxx)
    QString transId = QStringLiteral("FT%1%2")
                      .arg(QDateTime::currentMSecsSinceEpoch() % 1000000)
                      .arg(QRandomGenerator::global()->bounded(100, 999));

    emit paymentConfirmed(m_sessionId, m_amount, transId);
}
