#include "ANPRService.h"
#include <QRandomGenerator>

MockANPRService::MockANPRService() {
    // Danh sách biển số xe mẫu chuẩn định dạng Việt Nam
    m_mockPlates = {
        QStringLiteral("29A-839.21"),
        QStringLiteral("51F-123.45"),
        QStringLiteral("30H-999.88"),
        QStringLiteral("43C-678.90"),
        QStringLiteral("14B-567.89"),
        QStringLiteral("60A-333.22"),
        QStringLiteral("98A-111.44"),
        QStringLiteral("72B-888.66")
    };
}

QString MockANPRService::recognizeLicensePlate(const cv::Mat& /*frame*/) {
    // Tạm thời trả về chuỗi random định dạng biển số Việt Nam để test (Prompt 1)
    if (m_mockPlates.empty()) {
        return QStringLiteral("29A-123.45");
    }
    int idx = QRandomGenerator::global()->bounded(static_cast<int>(m_mockPlates.size()));
    return m_mockPlates[idx];
}
