#ifndef MONTHLY_PASS_STRATEGY_H
#define MONTHLY_PASS_STRATEGY_H

#include "IPricingStrategy.h"
#include "HourlyPricingStrategy.h"

class MonthlyPassStrategy : public IPricingStrategy {
private:
    bool m_hasActiveSubscription;
    HourlyPricingStrategy m_fallbackHourly;

public:
    explicit MonthlyPassStrategy(bool hasActiveSubscription = true);
    ~MonthlyPassStrategy() override = default;

    void setHasActiveSubscription(bool active);
    bool hasActiveSubscription() const;

    double calculateFee(const Ticket& ticket, 
                        const PricingConfig& config, 
                        const QDateTime& checkoutTime) const override;

    QString getStrategyName() const override;
};

#endif // MONTHLY_PASS_STRATEGY_H
