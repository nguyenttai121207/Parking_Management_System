#include "HourlyPricingStrategy.h"
#include <algorithm>

static inline QDateTime ensureValidCheckOut(const QDateTime& in, const QDateTime& out) {
    return (out.isValid() && out >= in) ? out : in.addSecs(60);
}

int HourlyPricingStrategy::calculateBillableHours(const QDateTime& checkIn, const QDateTime& checkOut) const {
    QDateTime safeOut = ensureValidCheckOut(checkIn, checkOut);
    qint64 seconds = checkIn.secsTo(safeOut);
    qint64 totalMinutes = seconds / 60;
    if (totalMinutes <= 0) {
        return 1;
    }

    int hours = static_cast<int>(totalMinutes / 60);
    int remainingMinutes = static_cast<int>(totalMinutes % 60);

    if (remainingMinutes >= 15) {
        hours += 1;
    }

    return std::max(1, hours);
}

double HourlyPricingStrategy::calculateFee(const Ticket& ticket, 
                                          const PricingConfig& config, 
                                          const QDateTime& checkoutTime) const {
    int billableHours = calculateBillableHours(ticket.getCheckInTime(), checkoutTime);
    return billableHours * config.getHourlyRate();
}

QString HourlyPricingStrategy::getStrategyName() const {
    return QStringLiteral("Vé theo giờ");
}
