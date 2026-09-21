#include "MonthlyPassStrategy.h"

MonthlyPassStrategy::MonthlyPassStrategy(bool hasActiveSubscription)
    : m_hasActiveSubscription(hasActiveSubscription) {
}

void MonthlyPassStrategy::setHasActiveSubscription(bool active) {
    m_hasActiveSubscription = active;
}

bool MonthlyPassStrategy::hasActiveSubscription() const {
    return m_hasActiveSubscription;
}

double MonthlyPassStrategy::calculateFee(const Ticket& ticket, 
                                        const PricingConfig& config, 
                                        const QDateTime& checkoutTime) const {
    if (m_hasActiveSubscription) {
        return 0.0;
    }
    return m_fallbackHourly.calculateFee(ticket, config, checkoutTime);
}

QString MonthlyPassStrategy::getStrategyName() const {
    return m_hasActiveSubscription ? QStringLiteral("Vé tháng (Đã thanh toán trước)") 
                                   : QStringLiteral("Vé tháng hết hạn (Tính theo giờ)");
}
