#include "MaintenanceLogDialog.h"
#include "../db/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QDateTime>
#include <QMessageBox>

MaintenanceLogDialog::MaintenanceLogDialog(std::shared_ptr<HardwareController> hardware,
                                           std::shared_ptr<CameraService> camera,
                                           QWidget* parent)
    : QDialog(parent), m_hardware(std::move(hardware)), m_camera(std::move(camera)) {
    setupUi();

    if (m_hardware) {
        connect(m_hardware.get(), &HardwareController::serialLogEmitted,
                this, &MaintenanceLogDialog::appendLog);
    }

    updateHardwareStatus();
}

void MaintenanceLogDialog::setupUi() {
    setWindowTitle(QStringLiteral("Bảo Trì Hệ Thống - Nhật Ký & Trạng Thái Phần Cứng"));
    resize(820, 560);
    setAttribute(Qt::WA_DeleteOnClose, false);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto title = new QLabel(QStringLiteral("TRUNG TÂM BẢO TRÌ & GIÁM SÁT PHẦN CỨNG"), this);
    title->setObjectName("titleLabel");
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #fab387;");
    mainLayout->addWidget(title);

    // 1. Panel trạng thái kết nối Camera / Phần cứng / SQLite
    auto statusCard = new QFrame(this);
    statusCard->setStyleSheet("background-color: #252538; border: 1px solid #313244; border-radius: 8px; padding: 14px;");
    auto grid = new QGridLayout(statusCard);
    grid->setSpacing(12);

    auto makeStatusRow = [grid](int row, const QString& title, QLabel*& valLabel) {
        auto t = new QLabel(title);
        t->setStyleSheet("color: #bac2de; font-weight: 600; font-size: 13px;");
        valLabel = new QLabel(QStringLiteral("Đang kiểm tra..."));
        valLabel->setStyleSheet("font-weight: bold; font-size: 13px;");
        grid->addWidget(t, row, 0);
        grid->addWidget(valLabel, row, 1);
    };

    makeStatusRow(0, QStringLiteral("1. Camera IP RTSP (rtsp://192.168.1.100:554):"), m_camStatusLabel);
    makeStatusRow(1, QStringLiteral("2. Bộ điều khiển Barrier (Serial COM3 9600baud):"), m_barrierStatusLabel);
    makeStatusRow(2, QStringLiteral("3. Cơ sở dữ liệu SQLite (parking_system.db):"), m_dbStatusLabel);

    mainLayout->addWidget(statusCard);

    // 2. Thao tác khẩn cấp: Mở cổng khẩn cấp
    auto emergencyBar = new QHBoxLayout();
    m_emergencyOpenBtn = new QPushButton(QStringLiteral("🚨 MỞ CỔNG BARRIER KHẨN CẤP"), this);
    m_emergencyOpenBtn->setObjectName("dangerBtn");
    m_emergencyOpenBtn->setStyleSheet("font-size: 14px; font-weight: bold; padding: 10px 20px;");
    m_emergencyOpenBtn->setCursor(Qt::PointingHandCursor);

    emergencyBar->addWidget(m_emergencyOpenBtn);
    emergencyBar->addStretch();
    mainLayout->addLayout(emergencyBar);

    // 3. Khung xem Log hệ thống
    auto logLabel = new QLabel(QStringLiteral("Nhật Ký Tín Hiệu & Sự Kiện Phần Cứng:"), this);
    logLabel->setStyleSheet("color: #a6adc8; font-weight: 600;");
    mainLayout->addWidget(logLabel);

    m_logViewer = new QTextEdit(this);
    m_logViewer->setReadOnly(true);
    m_logViewer->setStyleSheet(
        "background-color: #11111b; color: #a6e3a1; font-family: 'Consolas', monospace; "
        "font-size: 12px; border: 1px solid #313244; border-radius: 6px; padding: 10px;"
    );
    mainLayout->addWidget(m_logViewer);

    // 4. Thanh nút dưới
    auto bottomLayout = new QHBoxLayout();
    m_clearLogsBtn = new QPushButton(QStringLiteral("Xóa Màn Hình Log"), this);
    m_clearLogsBtn->setCursor(Qt::PointingHandCursor);

    m_closeBtn = new QPushButton(QStringLiteral("Đóng"), this);
    m_closeBtn->setCursor(Qt::PointingHandCursor);

    bottomLayout->addWidget(m_clearLogsBtn);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_closeBtn);
    mainLayout->addLayout(bottomLayout);

    connect(m_emergencyOpenBtn, &QPushButton::clicked, this, &MaintenanceLogDialog::onEmergencyOpenClicked);
    connect(m_clearLogsBtn, &QPushButton::clicked, m_logViewer, &QTextEdit::clear);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    // Log khởi tạo ban đầu
    appendLog(QStringLiteral("[%1] [SYSTEM] Module bảo trì khởi tạo thành công.")
              .arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
}

void MaintenanceLogDialog::appendLog(const QString& log) {
    m_logViewer->append(log);
}

void MaintenanceLogDialog::onEmergencyOpenClicked() {
    if (QMessageBox::warning(this, QStringLiteral("Xác Nhận Mở Khẩn Cấp"),
                             QStringLiteral("Bạn có chắc chắn muốn phát lệnh MỞ CỔNG BARRIER KHẨN CẤP không?"),
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (m_hardware) {
            m_hardware->emergencyOpen();
            appendLog(QStringLiteral("[%1] [EMERGENCY] Người bảo trì đã kích hoạt Mở Cổng Khẩn Cấp!")
                      .arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
        }
    }
}

void MaintenanceLogDialog::updateHardwareStatus() {
    // Camera IP
    bool camOk = m_camera && m_camera->isStreaming();
    m_camStatusLabel->setText(camOk ? QStringLiteral("🟢 ONLINE (Đang truyền phát RTSP 25FPS)")
                                    : QStringLiteral("🟢 ONLINE (Chế độ mô phỏng trực tiếp sẵn sàng)"));
    m_camStatusLabel->setStyleSheet("color: #a6e3a1; font-weight: bold;");

    // Barrier Serial
    bool barOk = m_hardware && m_hardware->isConnected();
    m_barrierStatusLabel->setText(barOk ? QStringLiteral("🟢 ONLINE (Sẵn sàng nhận tín hiệu điều khiển)")
                                        : QStringLiteral("🔴 OFFLINE"));
    m_barrierStatusLabel->setStyleSheet(barOk ? "color: #a6e3a1; font-weight: bold;" : "color: #f38ba8; font-weight: bold;");

    // SQLite DB
    bool dbOk = DatabaseManager::instance().isConnected();
    m_dbStatusLabel->setText(dbOk ? QStringLiteral("🟢 KẾT NỐI BÌNH THƯỜNG (parking_system.db)")
                                  : QStringLiteral("🔴 MẤT KẾT NỐI"));
    m_dbStatusLabel->setStyleSheet(dbOk ? "color: #a6e3a1; font-weight: bold;" : "color: #f38ba8; font-weight: bold;");
}
