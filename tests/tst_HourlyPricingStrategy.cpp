// tst_HourlyPricingStrategy.cpp
#include <QtTest>
#include "HourlyPricingStrategy.h"
#include "PricingConfig.h"
#include "Ticket.h"
#include "VehicleType.h"

// Quy tắc đọc từ code:
// - Block duy nhất = 1 giờ
// - remainingMinutes >= 15 → cộng thêm 1 giờ (làm tròn lên)
// - tối thiểu 1 giờ dù thời gian = 0
// - out < in hoặc invalid → safeOut = in + 60s → totalMinutes = 1 → 0 giờ + 1 phút → hours=0, 1%60=1 < 15 → hours=0 → max(1,0)=1 giờ
// - calculateFee = billableHours * hourlyRate (simple, không dùng first/next block)

// ⚠️ GHI CHÚ: PricingConfig có field firstBlockFee/nextBlockFee nhưng HourlyPricingStrategy
// chỉ dùng getHourlyRate(). Dữ liệu seed trong DatabaseManager.cpp gán cả firstBlockFee
// và hourlyRate cùng giá trị → nhất quán về kết quả, nhưng firstBlockFee/nextBlockFee
// hoàn toàn không được dùng trong strategy. Không phải bug, nhưng cần lưu ý.

static Ticket makeTicket(VehicleType vt, const QDateTime& checkIn) {
    Ticket t;
    t.setVehicleType(vt);
    t.setCheckInTime(checkIn);
    return t;
}

static PricingConfig makeConfig(VehicleType vt, double hourlyRate) {
    return PricingConfig(vt, hourlyRate, 0.0);
}

class tst_HourlyPricingStrategy : public QObject {
    Q_OBJECT

private:
    HourlyPricingStrategy strategy;
    QDateTime base; // 2024-01-15 08:00:00

    void init() { base = QDateTime(QDate(2024, 1, 15), QTime(8, 0, 0)); }

private slots:
    void initTestCase()  { base = QDateTime(QDate(2024, 1, 15), QTime(8, 0, 0)); }

    // ── calculateBillableHours ─────────────────────────────────

    void billable_zero_seconds() {
        // out == in: safeOut = in+60s → 1 min → hours=0, rem=1 < 15 → max(1,0)=1
        QCOMPARE(strategy.calculateBillableHours(base, base), 1);
    }

    void billable_invalid_checkout() {
        // out invalid → safeOut = in+60s → same path as above → 1
        QCOMPARE(strategy.calculateBillableHours(base, QDateTime()), 1);
    }

    void billable_out_before_in() {
        // out < in → safeOut = in+60s → 1
        QCOMPARE(strategy.calculateBillableHours(base, base.addSecs(-3600)), 1);
    }

    void billable_exactly_1h() {
        // 60 min → hours=1, rem=0 → 1
        QCOMPARE(strategy.calculateBillableHours(base, base.addSecs(3600)), 1);
    }

    void billable_1h_14min() {
        // 74 min → hours=1, rem=14 < 15 → 1
        QCOMPARE(strategy.calculateBillableHours(base, base.addSecs(74 * 60)), 1);
    }

    void billable_1h_15min() {
        // 75 min → hours=1, rem=15 >= 15 → +1 → 2
        QCOMPARE(strategy.calculateBillableHours(base, base.addSecs(75 * 60)), 2);
    }

    void billable_1h_59min() {
        // 119 min → hours=1, rem=59 >= 15 → 2
        QCOMPARE(strategy.calculateBillableHours(base, base.addSecs(119 * 60)), 2);
    }

    void billable_2h_exactly() {
        // 120 min → hours=2, rem=0 → 2
        QCOMPARE(strategy.calculateBillableHours(base, base.addSecs(120 * 60)), 2);
    }

    void billable_under_1h() {
        // 30 min → hours=0, rem=30 >= 15 → +1 → max(1,1)=1
        QCOMPARE(strategy.calculateBillableHours(base, base.addSecs(30 * 60)), 1);
    }

    void billable_14min() {
        // 14 min → hours=0, rem=14 < 15 → 0 → max(1,0)=1
        QCOMPARE(strategy.calculateBillableHours(base, base.addSecs(14 * 60)), 1);
    }

    // ── calculateFee cho 4 loại xe ────────────────────────────

    void fee_bicycle_1h() {
        // Xe đạp: 2000đ/giờ, đỗ 1h → 2000
        auto ticket = makeTicket(VehicleType::Bicycle, base);
        auto config = makeConfig(VehicleType::Bicycle, 2000.0);
        QCOMPARE(strategy.calculateFee(ticket, config, base.addSecs(3600)), 2000.0);
    }

    void fee_motorbike_manual_2h() {
        // Xe máy số: 4000đ/giờ, đỗ 2h → 8000
        auto ticket = makeTicket(VehicleType::MotorbikeManual, base);
        auto config = makeConfig(VehicleType::MotorbikeManual, 4000.0);
        QCOMPARE(strategy.calculateFee(ticket, config, base.addSecs(2 * 3600)), 8000.0);
    }

    void fee_motorbike_scooter_rounded_up() {
        // Xe tay ga: 5000đ/giờ, đỗ 1h15min → 2 giờ → 10000
        auto ticket = makeTicket(VehicleType::MotorbikeScooter, base);
        auto config = makeConfig(VehicleType::MotorbikeScooter, 5000.0);
        QCOMPARE(strategy.calculateFee(ticket, config, base.addSecs(75 * 60)), 10000.0);
    }

    void fee_car_3h() {
        // Ô tô: 25000đ/giờ, đỗ 3h → 75000
        auto ticket = makeTicket(VehicleType::Car, base);
        auto config = makeConfig(VehicleType::Car, 25000.0);
        QCOMPARE(strategy.calculateFee(ticket, config, base.addSecs(3 * 3600)), 75000.0);
    }

    void fee_minimum_1h_even_if_zero() {
        // Dù checkout == checkin → vẫn tính 1 giờ tối thiểu
        auto ticket = makeTicket(VehicleType::Bicycle, base);
        auto config = makeConfig(VehicleType::Bicycle, 2000.0);
        QCOMPARE(strategy.calculateFee(ticket, config, base), 2000.0);
    }
};

QTEST_MAIN(tst_HourlyPricingStrategy)
#include "tst_HourlyPricingStrategy.moc"
