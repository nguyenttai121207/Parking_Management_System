#ifndef I_PRICING_STRATEGY_H
#define I_PRICING_STRATEGY_H

#include "../models/Ticket.h"
#include "../models/PricingConfig.h"
#include <QDateTime>

class IPricingStrategy {
public:
    virtual ~IPricingStrategy() = default;

    virtual double calculateFee(const Ticket& ticket, 
                                const PricingConfig& config, 
                                const QDateTime& checkoutTime) const = 0;

    virtual QString getStrategyName() const = 0;
};

#endif // I_PRICING_STRATEGY_H
