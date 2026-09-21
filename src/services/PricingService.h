#ifndef PRICING_SERVICE_H
#define PRICING_SERVICE_H

#include "../strategies/IPricingStrategy.h"
#include "../strategies/HourlyPricingStrategy.h"
#include "../strategies/MonthlyPassStrategy.h"
#include "../models/Ticket.h"
#include "../models/PricingConfig.h"
#include <memory>

class PricingService {
private:
    std::unique_ptr<IPricingStrategy> m_strategy;

public:
    PricingService();

    void setStrategy(std::unique_ptr<IPricingStrategy> strategy);

    double calculate(const Ticket& ticket, 
                     const PricingConfig& config, 
                     const QDateTime& checkoutTime) const;

    QString getCurrentStrategyName() const;
};

#endif // PRICING_SERVICE_H
