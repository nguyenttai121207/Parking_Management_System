#ifndef HOURLY_PRICING_STRATEGY_H
#define HOURLY_PRICING_STRATEGY_H

#include "IPricingStrategy.h"

class HourlyPricingStrategy : public IPricingStrategy {
public:
    HourlyPricingStrategy() = default;
    ~HourlyPricingStrategy() override = default;

    double calculateFee(const Ticket& ticket, 
                        const PricingConfig& config, 
                        const QDateTime& checkoutTime) const override;

    QString getStrategyName() const override;

    int calculateBillableHours(const QDateTime& checkIn, const QDateTime& checkOut) const;
};

#endif // HOURLY_PRICING_STRATEGY_H
