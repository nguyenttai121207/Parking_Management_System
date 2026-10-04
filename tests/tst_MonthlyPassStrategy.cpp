// tst_MonthlyPassStrategy.cpp
#include <QtTest>
#include "MonthlyPassStrategy.h"
#include "HourlyPricingStrategy.h"
#include "PricingConfig.h"
#include "Ticket.h"
#include "VehicleType.h"

// Quy tắc đọc từ code MonthlyPassStrategy.cpp:
// - m_hasActiveSubscription=true  → calculateFee() luôn trả 0.0
// - m_hasActiveSubscription=false → fallback sang HourlyPricingStrategy
//
// ⚠️ GHI CHÚ QUAN TRỌNG: MonthlyPassStrategy không nhận QDate để tự kiểm tra
// hạn vé. Caller (PricingService / ParkingManager) phải tự kiểm tra ngày hết hạn
// và truyền vào đúng flag hasActiveSubscription. Nếu caller truyền sai flag,
// xe hết hạn vẫn được tính giá 0. Strategy không bảo vệ được trường hợp này.
// → Không phải bug trong strategy, nhưng là điểm rủi ro ở tầng service.
//
// ⚠️ Không có test "đúng ngày hết hạn" hay "hết hạn 1 giây" ở tầng strategy
// vì strategy không biết ngày — những test này thuộc về tầng service (PricingService).
// Đã ghi chú để bạn viết thêm integration test nếu cần.

static Ticket makeTicket(const QDateTime& checkIn) {
    Ticket t;
    t.setVehicleType(VehicleType::MotorbikeManual);
    t.setCheckInTime(checkIn);
    return t;
}

static PricingConfig makeConfig(double hourlyRate = 4000.0) {
    return PricingConfig(VehicleType::MotorbikeManual, hourlyRate, 80000.0);
}

class tst_MonthlyPassStrategy : public QObject {
    Q_OBJECT

private:
    QDateTime base;

private slots:
    void initTestCase() {
        base = QDateTime(QDate(2024, 1, 15), QTime(8, 0, 0));
    }

    // ── Vé còn hạn (flag = true) ────────────────────────────────

    void active_subscription_fee_is_zero() {
        MonthlyPassStrategy strategy(true);
        auto ticket = makeTicket(base);
        auto config = makeConfig(4000.0);
        QCOMPARE(strategy.calculateFee(ticket, config, base.addSecs(3600)), 0.0);
    }

    void active_subscription_fee_zero_regardless_of_duration() {
        // Dù đỗ 10 giờ, vé còn hạn → 0đ
        MonthlyPassStrategy strategy(true);
        auto ticket = makeTicket(base);
        auto config = makeConfig(4000.0);
        QCOMPARE(strategy.calculateFee(ticket, config, base.addSecs(10 * 3600)), 0.0);
    }

    void active_strategy_name() {
        MonthlyPassStrategy strategy(true);
        QVERIFY(strategy.getStrategyName().contains("Đã thanh toán"));
    }

    // ── Vé hết hạn (flag = false) → fallback hourly ─────────────

    void expired_subscription_falls_back_to_hourly() {
        // 1 giờ đỗ, rate 4000 → 4000đ
        MonthlyPassStrategy strategy(false);
        auto ticket = makeTicket(base);
        auto config = makeConfig(4000.0);
        QCOMPARE(strategy.calculateFee(ticket, config, base.addSecs(3600)), 4000.0);
    }

    void expired_subscription_2h_rounded() {
        // 1h15min đỗ → 2 giờ tính phí → 2 * 4000 = 8000đ
        MonthlyPassStrategy strategy(false);
        auto ticket = makeTicket(base);
        auto config = makeConfig(4000.0);
        QCOMPARE(strategy.calculateFee(ticket, config, base.addSecs(75 * 60)), 8000.0);
    }

    void expired_subscription_minimum_1h() {
        // checkout == checkin → vẫn 1 giờ tối thiểu
        MonthlyPassStrategy strategy(false);
        auto ticket = makeTicket(base);
        auto config = makeConfig(5000.0);
        QCOMPARE(strategy.calculateFee(ticket, config, base), 5000.0);
    }

    void expired_strategy_name() {
        MonthlyPassStrategy strategy(false);
        QVERIFY(strategy.getStrategyName().contains("hết hạn"));
    }

    // ── setHasActiveSubscription runtime change ──────────────────

    void toggle_active_to_expired() {
        MonthlyPassStrategy strategy(true);
        auto ticket = makeTicket(base);
        auto config = makeConfig(4000.0);
        QCOMPARE(strategy.calculateFee(ticket, config, base.addSecs(3600)), 0.0);

        strategy.setHasActiveSubscription(false);
        QCOMPARE(strategy.calculateFee(ticket, config, base.addSecs(3600)), 4000.0);
    }

    void toggle_expired_to_active() {
        MonthlyPassStrategy strategy(false);
        auto ticket = makeTicket(base);
        auto config = makeConfig(4000.0);
        QCOMPARE(strategy.calculateFee(ticket, config, base.addSecs(3600)), 4000.0);

        strategy.setHasActiveSubscription(true);
        QCOMPARE(strategy.calculateFee(ticket, config, base.addSecs(3600)), 0.0);
    }

    void has_active_subscription_getter() {
        MonthlyPassStrategy s1(true);
        QVERIFY(s1.hasActiveSubscription());
        MonthlyPassStrategy s2(false);
        QVERIFY(!s2.hasActiveSubscription());
    }
};

QTEST_MAIN(tst_MonthlyPassStrategy)
#include "tst_MonthlyPassStrategy.moc"
