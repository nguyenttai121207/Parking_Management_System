#include "AutoGateManager.h"
#include <QDateTime>

AutoGateManager::AutoGateManager(std::shared_ptr<CameraService> camera,
                                 std::shared_ptr<HardwareController> hardware,
                                 QObject* parent)
    : QObject(parent), m_camera(std::move(camera)),
      m_anpr(std::make_unique<MockANPRService>()),
      m_hardware(std::move(hardware)) {}

void AutoGateManager::triggerVehicleEntry(const QString& plateOverride) {
    QString recognizedPlate = plateOverride.trimmed().toUpper();

    // 1. Nhận diện biển số (ANPR) nếu không nhập biển thủ công
    if (recognizedPlate.isEmpty()) {
        cv::Mat frame = m_camera->getCurrentFrameMat();
        recognizedPlate = m_anpr->recognizeLicensePlate(frame).trimmed().toUpper();
    }

    emit vehicleProcessingStarted(recognizedPlate);

    // Kiểm tra xem xe này hiện có đang ở trong bãi chưa
    auto existingSession = m_repo.findActiveSessionByPlate(recognizedPlate);
    if (existingSession.has_value()) {
        emit checkInFailed(recognizedPlate, QStringLiteral("Xe biển số %1 đang có một phiên đỗ chưa kết thúc!").arg(recognizedPlate));
        return;
    }

    // 2. Logic Prompt 3: Kiểm tra tự động vé tháng (MonthlyPasses)
    auto passOpt = m_repo.findValidMonthlyPass(recognizedPlate, QDate::currentDate());

    if (passOpt.has_value()) {
        // Vé tháng hợp lệ: Tự động ghi log vào ParkingSessions với giá 0đ
        m_repo.createSession(recognizedPlate, passOpt->getVehicleType(), QDateTime::currentDateTime(), 0.0, QStringLiteral("Đang đỗ"));

        // Báo hiệu mở Barrier ngay lập tức, bỏ qua bước lấy thẻ vé ngày
        m_hardware->openBarrier(QStringLiteral("Vé tháng hợp lệ: %1 (%2)").arg(recognizedPlate, passOpt->getCustomerName()));

        // Báo lên màn hình 'Vé tháng hợp lệ - Mời qua'
        emit checkInCompleted(recognizedPlate, true, QStringLiteral("Vé tháng hợp lệ - Mời qua"), 0.0);
    } else {
        // Xe vãng lai / vé ngày thông thường
        VehicleType defaultType = recognizedPlate.startsWith("29A") || recognizedPlate.startsWith("30") || recognizedPlate.startsWith("51F") 
                                  ? VehicleType::Car : VehicleType::MotorbikeManual;

        m_repo.createSession(recognizedPlate, defaultType, QDateTime::currentDateTime(), 0.0, QStringLiteral("Đang đỗ"));
        m_hardware->openBarrier(QStringLiteral("Xe vãng lai vào bãi: %1").arg(recognizedPlate));

        emit checkInCompleted(recognizedPlate, false, QStringLiteral("Đã cấp phiên gửi xe - Mời qua cổng"), 0.0);
    }
}
