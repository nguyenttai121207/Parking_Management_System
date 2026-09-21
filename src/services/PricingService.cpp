#include "PricingService.h"

PricingService::PricingService() 
    : m_strategy(std::make_unique<HourlyPricingStrategy>()) {
}

void PricingService::setStrategy(std::unique_ptr<IPricingStrategy> strategy) {
    if (strategy) {
        m_strategy = std::move(strategy);
    }
}

double PricingService::calculate(const Ticket& ticket, 
                                 const PricingConfig& config, 
                                 const QDateTime& checkoutTime) const {
    if (!m_strategy) {
        return 0.0;
    }
    return m_strategy->calculateFee(ticket, config, checkoutTime);
}

QString PricingService::getCurrentStrategyName() const {
    return m_strategy ? m_strategy->getStrategyName() : QStringLiteral("Chưa thiết lập");
}
