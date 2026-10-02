#include "KioskGateDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPixmap>
#include <QDateTime>

KioskGateDialog::KioskGateDialog(std::shared_ptr<AutoGateManager> gateManager, QWidget* parent)
    : QDialog(parent), m_gateManager(std::move(gateManager)) {
    setupUi();

    // Kết nối luồng Camera
    if (m_gateManager && m_gateManager->getCamera()) {
        connect(m_gateManager->getCamera().get(), &CameraService::frameReady,
                this, &KioskGateDialog::onFrameReceived);
        m_gateManager->getCamera()->startStream();
    }

    // Kết nối phần cứng Barrier
    if (m_gateManager && m_gateManager->getHardware()) {
        connect(m_gateManager->getHardware().get(), &HardwareController::barrierStatusChanged,
                this, &KioskGateDialog::onBarrierStatusChanged);
    }

    // Kết nối sự kiện Check-in tự động
    if (m_gateManager) {
        connect(m_gateManager.get(), &AutoGateManager::checkInCompleted,
                this, &KioskGateDialog::onCheckInSuccess);
        connect(m_gateManager.get(), &AutoGateManager::checkInFailed,
                this, &KioskGateDialog::onCheckInFailed);
    }
}

KioskGateDialog::~KioskGateDialog() {
    if (m_gateManager && m_gateManager->getCamera()) {
        m_gateManager->getCamera()->stopStream();
    }
}

void KioskGateDialog::setupUi() {
    setWindowTitle(QStringLiteral("Cổng Vào Tự Động - Kiosk Mode"));
    resize(1000, 720);
    setAttribute(Qt::WA_DeleteOnClose, false);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 1. Dòng chữ hướng dẫn to, rõ ràng theo đúng yêu cầu Prompt 1
    m_instructionLabel = new QLabel(QStringLiteral("VUI LÒNG ĐƯA XE VÀO VỊ TRÍ VÀ CHỜ MỞ CỔNG"), this);
    m_instructionLabel->setObjectName("titleLabel");
    m_instructionLabel->setStyleSheet(
        "font-size: 24px; font-weight: 800; color: #f9e2af; background-color: #1e1e2e; "
        "border: 2px solid #fab387; border-radius: 8px; padding: 14px; letter-spacing: 0.05em;"
    );
    m_instructionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_instructionLabel);

    // 2. Khu vực trung tâm: Camera QLabel & Bảng thông tin trạng thái Kiosk
    auto centerLayout = new QHBoxLayout();
    centerLayout->setSpacing(16);

    // Frame Camera RTSP
    auto cameraFrame = new QFrame(this);
    cameraFrame->setStyleSheet("background-color: #11111b; border: 2px solid #313244; border-radius: 10px;");
    auto camLayout = new QVBoxLayout(cameraFrame);
    camLayout->setContentsMargins(6, 6, 6, 6);

    m_cameraLabel = new QLabel(cameraFrame);
    m_cameraLabel->setMinimumSize(600, 360);
    m_cameraLabel->setStyleSheet("background-color: #000000; border-radius: 6px;");
    m_cameraLabel->setAlignment(Qt::AlignCenter);
    m_cameraLabel->setText(QStringLiteral("Đang kết nối luồng Camera IP RTSP..."));
    camLayout->addWidget(m_cameraLabel);

    centerLayout->addWidget(cameraFrame, 3);

    // Panel bên phải: Trạng thái Barrier, Biển số, Thông báo
    auto statusPanel = new QFrame(this);
    statusPanel->setStyleSheet("background-color: #252538; border: 1px solid #313244; border-radius: 10px; padding: 16px;");
    auto panelLayout = new QVBoxLayout(statusPanel);
    panelLayout->setSpacing(14);

    auto panelTitle = new QLabel(QStringLiteral("TRẠNG THÁI CỔNG VÀO"), statusPanel);
    panelTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #89b4fa;");
    panelTitle->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(panelTitle);

    // Đèn Barrier
    m_barrierStatusLabel = new QLabel(QStringLiteral("BARRIER: ĐANG ĐÓNG [●]"), statusPanel);
    m_barrierStatusLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #f38ba8; background-color: #1e1e2e; "
        "border: 1px solid #f38ba8; border-radius: 6px; padding: 10px; text-align: center;"
    );
    m_barrierStatusLabel->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(m_barrierStatusLabel);

    // Biển số vừa quét
    auto plateHeader = new QLabel(QStringLiteral("Biển Số Xe Vừa Quét:"), statusPanel);
    plateHeader->setStyleSheet("color: #a6adc8; font-size: 12px;");
    panelLayout->addWidget(plateHeader);

    m_plateResultLabel = new QLabel(QStringLiteral("Chờ phương tiện..."), statusPanel);
    m_plateResultLabel->setStyleSheet(
        "font-size: 22px; font-weight: 800; color: #a6e3a1; background-color: #181825; "
        "border: 1px solid #45475a; border-radius: 6px; padding: 8px; font-family: 'Consolas', monospace;"
    );
    m_plateResultLabel->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(m_plateResultLabel);

    // Thông báo lớn (Prompt 3: 'Vé tháng hợp lệ - Mời qua')
    m_announcementLabel = new QLabel(QStringLiteral("Sẵn sàng đón xe"), statusPanel);
    m_announcementLabel->setStyleSheet(
        "font-size: 16px; font-weight: 700; color: #cdd6f4; background-color: #181825; "
        "border: 1px solid #313244; border-radius: 6px; padding: 14px;"
    );
    m_announcementLabel->setWordWrap(true);
    m_announcementLabel->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(m_announcementLabel);

    panelLayout->addStretch();
    centerLayout->addWidget(statusPanel, 2);
    mainLayout->addLayout(centerLayout);

    // 3. Khung điều khiển & mô phỏng cảm biến (Sensor)
    auto controlBar = new QFrame(this);
    controlBar->setStyleSheet("background-color: #1e1e2e; border: 1px solid #313244; border-radius: 8px; padding: 8px 14px;");
    auto barLayout = new QHBoxLayout(controlBar);
    barLayout->setSpacing(12);

    auto simLabel = new QLabel(QStringLiteral("Mô phỏng Sensor xe vào / Nhập biển test:"), controlBar);
    simLabel->setStyleSheet("color: #bac2de; font-size: 12px;");
    barLayout->addWidget(simLabel);

    m_plateOverrideEdit = new QLineEdit(controlBar);
    m_plateOverrideEdit->setPlaceholderText(QStringLiteral("Để trống để nhận diện tự động qua Camera ANPR"));
    m_plateOverrideEdit->setStyleSheet("max-width: 320px;");
    barLayout->addWidget(m_plateOverrideEdit);

    m_sensorBtn = new QPushButton(QStringLiteral("🔔 PHÁT HIỆN XE (KÍCH HOẠT SENSOR)"), controlBar);
    m_sensorBtn->setObjectName("successBtn");
    m_sensorBtn->setStyleSheet("font-size: 13px; font-weight: bold; padding: 10px 18px;");
    m_sensorBtn->setCursor(Qt::PointingHandCursor);
    barLayout->addWidget(m_sensorBtn);

    barLayout->addStretch();

    m_closeBtn = new QPushButton(QStringLiteral("Đóng Kiosk"), controlBar);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    barLayout->addWidget(m_closeBtn);

    mainLayout->addWidget(controlBar);

    connect(m_sensorBtn, &QPushButton::clicked, this, &KioskGateDialog::onSensorTriggered);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void KioskGateDialog::onFrameReceived(const QImage& frame) {
    if (!frame.isNull()) {
        m_cameraLabel->setPixmap(QPixmap::fromImage(frame).scaled(
            m_cameraLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void KioskGateDialog::onBarrierStatusChanged(bool isOpen, const QString& reason) {
    if (isOpen) {
        m_barrierStatusLabel->setText(QStringLiteral("BARRIER: ĐANG MỞ [✔]"));
        m_barrierStatusLabel->setStyleSheet(
            "font-size: 16px; font-weight: bold; color: #a6e3a1; background-color: #1e2e22; "
            "border: 2px solid #a6e3a1; border-radius: 6px; padding: 10px;"
        );
    } else {
        m_barrierStatusLabel->setText(QStringLiteral("BARRIER: ĐANG ĐÓNG [●]"));
        m_barrierStatusLabel->setStyleSheet(
            "font-size: 16px; font-weight: bold; color: #f38ba8; background-color: #2e1e22; "
            "border: 1px solid #f38ba8; border-radius: 6px; padding: 10px;"
        );
    }
}

void KioskGateDialog::onCheckInSuccess(const QString& plate, bool isMonthly, const QString& message, double /*fee*/) {
    m_plateResultLabel->setText(plate);

    if (isMonthly) {
        // Prompt 3: Báo lên màn hình 'Vé tháng hợp lệ - Mời qua'
        m_announcementLabel->setText(QStringLiteral("VÉ THÁNG HỢP LỆ - MỜI QUA\n[Miễn phí 0 VNĐ]"));
        m_announcementLabel->setStyleSheet(
            "font-size: 16px; font-weight: 800; color: #a6e3a1; background-color: #18281e; "
            "border: 2px solid #a6e3a1; border-radius: 8px; padding: 14px;"
        );
    } else {
        m_announcementLabel->setText(QStringLiteral("%1\n[Phiên đỗ: 'Đang đỗ']").arg(message));
        m_announcementLabel->setStyleSheet(
            "font-size: 16px; font-weight: 800; color: #89b4fa; background-color: #1e1e2e; "
            "border: 2px solid #89b4fa; border-radius: 8px; padding: 14px;"
        );
    }
}

void KioskGateDialog::onCheckInFailed(const QString& plate, const QString& error) {
    m_plateResultLabel->setText(plate);
    m_announcementLabel->setText(QStringLiteral("LỖI CHECK-IN:\n%1").arg(error));
    m_announcementLabel->setStyleSheet(
        "font-size: 14px; font-weight: bold; color: #f38ba8; background-color: #2e1e22; "
        "border: 2px solid #f38ba8; border-radius: 8px; padding: 14px;"
    );
}

void KioskGateDialog::onSensorTriggered() {
    QString customPlate = m_plateOverrideEdit->text().trimmed();
    m_gateManager->triggerVehicleEntry(customPlate);
}
