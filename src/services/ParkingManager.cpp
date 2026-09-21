#include "ParkingManager.h"
#include <QDateTime>

ParkingManager::ParkingManager() = default;

CheckInResult ParkingManager::checkIn(VehicleType type, const QString& licensePlate) {
    CheckInResult result;
    result.success = false;

    std::unique_ptr<Vehicle> vehicle = VehicleFactory::createVehicle(type, licensePlate);
    if (!vehicle) {
        result.message = QStringLiteral("Loại phương tiện không hợp lệ!");
        return result;
    }

    if (!vehicle->validateLicensePlate()) {
        result.message = QStringLiteral("Định dạng biển số xe không hợp lệ (Ví dụ: 29A-12345, 59B1-99999)!");
        return result;
    }

    auto existingTicket = m_repo.getActiveTicketByPlate(vehicle->getLicensePlate());
    if (existingTicket.has_value()) {
        result.message = QStringLiteral("Xe mang biển số %1 hiện đang ở trong bãi (Chỗ %2)!")
                             .arg(vehicle->getLicensePlate(), existingTicket->getSlotNumber());
        return result;
    }

    SlotType requiredSlotType = vehicle->getRequiredSlotType();
    auto availableSlot = m_repo.findAvailableSlot(requiredSlotType);
    if (!availableSlot.has_value()) {
        result.message = QStringLiteral("Bãi giữ xe đã hết chỗ trống cho %1!")
                             .arg(VehicleUtils::getSlotTypeName(requiredSlotType));
        return result;
    }

    QDateTime now = QDateTime::currentDateTime();
    auto ticket = m_repo.createTicket(vehicle->getLicensePlate(), type, availableSlot->getSlotId(), now);
    if (!ticket.has_value()) {
        result.message = QStringLiteral("Lỗi cơ sở dữ liệu khi tạo vé xe!");
        return result;
    }

    m_repo.updateSlotOccupancy(availableSlot->getSlotId(), true, vehicle->getLicensePlate());

    result.success = true;
    result.message = QStringLiteral("Check-in thành công! Gán vị trí: %1").arg(availableSlot->getSlotNumber());
    result.ticket = ticket;
    return result;
}

CheckOutReceipt ParkingManager::previewCheckOut(const QString& licensePlate) {
    CheckOutReceipt receipt;
    receipt.success = false;
    receipt.fee = 0.0;
    receipt.billableHours = 0;
    receipt.isMonthlySubscriber = false;

    auto ticketOpt = m_repo.getActiveTicketByPlate(licensePlate);
    if (!ticketOpt.has_value()) {
        receipt.message = QStringLiteral("Không tìm thấy xe có biển số %1 đang gửi trong bãi!").arg(licensePlate);
        return receipt;
    }

    receipt.ticket = ticketOpt.value();
    receipt.checkOutTime = QDateTime::currentDateTime();

    auto configOpt = m_repo.getPricingConfig(receipt.ticket.getVehicleType());
    PricingConfig config = configOpt.value_or(PricingConfig(receipt.ticket.getVehicleType(), 10000.0, 100000.0));

    auto subOpt = m_repo.findActiveSubscription(receipt.ticket.getLicensePlate(), receipt.checkOutTime.date());
    if (subOpt.has_value()) {
        receipt.isMonthlySubscriber = true;
        m_pricingService.setStrategy(std::make_unique<MonthlyPassStrategy>(true));
        receipt.pricingType = "MONTHLY_PASS";
    } else {
        receipt.isMonthlySubscriber = false;
        auto hourlyStrategy = std::make_unique<HourlyPricingStrategy>();
        receipt.billableHours = hourlyStrategy->calculateBillableHours(receipt.ticket.getCheckInTime(), receipt.checkOutTime);
        m_pricingService.setStrategy(std::move(hourlyStrategy));
        receipt.pricingType = "HOURLY";
    }

    receipt.fee = m_pricingService.calculate(receipt.ticket, config, receipt.checkOutTime);
    receipt.success = true;
    receipt.message = QStringLiteral("Tính tiền thành công!");
    return receipt;
}

bool ParkingManager::confirmCheckOut(const CheckOutReceipt& receipt) {
    if (!receipt.success) {
        return false;
    }

    bool ticketUpdated = m_repo.completeTicket(
        receipt.ticket.getTicketId(),
        receipt.checkOutTime,
        receipt.fee,
        receipt.pricingType
    );

    if (ticketUpdated) {
        m_repo.updateSlotOccupancy(receipt.ticket.getSlotId(), false, "");
        return true;
    }
    return false;
}

bool ParkingManager::registerSubscription(const QString& licensePlate, VehicleType type,
                                          const QString& customerName, const QString& phone,
                                          int durationMonths, QString& outMessage) {
    std::unique_ptr<Vehicle> vehicle = VehicleFactory::createVehicle(type, licensePlate);
    if (!vehicle || !vehicle->validateLicensePlate()) {
        outMessage = QStringLiteral("Biển số xe không đúng định dạng!");
        return false;
    }

    if (durationMonths <= 0) {
        outMessage = QStringLiteral("Thời hạn đăng ký phải từ 1 tháng trở lên!");
        return false;
    }

    auto configOpt = m_repo.getPricingConfig(type);
    double monthlyRate = configOpt.has_value() ? configOpt->getMonthlyRate() : 100000.0;
    double totalPrice = monthlyRate * durationMonths;

    QDate startDate = QDate::currentDate();
    QDate endDate = startDate.addMonths(durationMonths);

    MonthlySubscription sub(0, vehicle->getLicensePlate(), type, customerName, phone, startDate, endDate, totalPrice);
    if (m_repo.saveSubscription(sub)) {
        outMessage = QStringLiteral("Đăng ký thành công vé tháng cho xe %1 đến ngày %2! Phí: %3 VNĐ")
                         .arg(vehicle->getLicensePlate(), endDate.toString("dd/MM/yyyy"), QString::number(totalPrice, 'f', 0));
        return true;
    }

    outMessage = QStringLiteral("Lỗi cơ sở dữ liệu khi lưu thông tin vé tháng!");
    return false;
}

DashboardStats ParkingManager::getDashboardStats() {
    DashboardStats stats;
    stats.totalMotorbikeSlots = m_repo.countTotalSlots(SlotType::MotorbikeSlot);
    stats.occupiedMotorbikeSlots = m_repo.countOccupiedSlots(SlotType::MotorbikeSlot);
    stats.totalCarSlots = m_repo.countTotalSlots(SlotType::CarSlot);
    stats.occupiedCarSlots = m_repo.countOccupiedSlots(SlotType::CarSlot);
    stats.activeVehiclesCount = stats.occupiedMotorbikeSlots + stats.occupiedCarSlots;
    stats.todayRevenue = m_repo.getDailyRevenue(QDate::currentDate());
    return stats;
}

std::vector<ParkingSlot> ParkingManager::getAllSlots() {
    return m_repo.getAllSlots();
}

std::vector<Ticket> ParkingManager::getAllActiveTickets() {
    return m_repo.getAllActiveTickets();
}

std::vector<Ticket> ParkingManager::searchHistory(const QString& licensePlate) {
    return m_repo.searchTicketHistory(licensePlate);
}

std::vector<MonthlySubscription> ParkingManager::getAllSubscriptions() {
    return m_repo.getAllSubscriptions();
}

std::vector<PricingConfig> ParkingManager::getPricingConfigs() {
    return m_repo.getAllPricingConfigs();
}

bool ParkingManager::updatePricingConfig(const PricingConfig& config) {
    return m_repo.updatePricingConfig(config);
}
