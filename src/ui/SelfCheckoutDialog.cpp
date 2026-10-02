#include "SelfCheckoutDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QDateTime>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QRandomGenerator>

SelfCheckoutDialog::SelfCheckoutDialog(std::shared_ptr<HardwareController> hardware, QWidget* parent)
    : QDialog(parent), m_hardware(std::move(hardware)),
      m_webhookSimulator(std::make_unique<BankWebhookSimulator>(this)),
      m_currentFee(0.0) {
    setupUi();

    connect(m_webhookSimulator.get(), &BankWebhookSimulator::paymentConfirmed,
            this, &SelfCheckoutDialog::onPaymentConfirmed);
}

SelfCheckoutDialog::~SelfCheckoutDialog() {
    if (m_webhookSimulator) {
        m_webhookSimulator->stopListening();
    }
}

void SelfCheckoutDialog::setupUi() {
    setWindowTitle(QStringLiteral("Thanh Toán Tự Động (Self-Checkout)"));
    resize(860, 680);
    setAttribute(Qt::WA_DeleteOnClose, false);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(22, 22, 22, 22);

    auto title = new QLabel(QStringLiteral("TRẠM TỰ ĐỘNG THANH TOÁN XUẤT BÃI"), this);
    title->setObjectName("titleLabel");
    title->setStyleSheet("font-size: 20px; font-weight: 800; color: #89b4fa;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // 1. Thanh tìm kiếm biển số / Quét thẻ RFID giả lập
    auto searchFrame = new QFrame(this);
    searchFrame->setStyleSheet("background-color: #252538; border: 1px solid #313244; border-radius: 8px; padding: 10px;");
    auto searchLayout = new QHBoxLayout(searchFrame);
    searchLayout->setSpacing(10);

    auto lblPlate = new QLabel(QStringLiteral("Nhập Biển Số Xe:"), searchFrame);
    lblPlate->setStyleSheet("font-weight: 600; color: #bac2de;");
    searchLayout->addWidget(lblPlate);

    m_plateSearchEdit = new QLineEdit(searchFrame);
    m_plateSearchEdit->setPlaceholderText(QStringLiteral("Ví dụ: 29A-839.21"));
    searchLayout->addWidget(m_plateSearchEdit);

    m_searchBtn = new QPushButton(QStringLiteral("🔍 Tra Cứu"), searchFrame);
    m_searchBtn->setObjectName("primaryBtn");
    m_searchBtn->setCursor(Qt::PointingHandCursor);
    searchLayout->addWidget(m_searchBtn);

    m_rfidMockBtn = new QPushButton(QStringLiteral("💳 Quẹt Thẻ RFID (Mô Phỏng)"), searchFrame);
    m_rfidMockBtn->setCursor(Qt::PointingHandCursor);
    searchLayout->addWidget(m_rfidMockBtn);

    mainLayout->addWidget(searchFrame);

    // 2. Nội dung chi tiết phiên đỗ xe & VietQR (chia 2 cột)
    auto contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(16);

    // Cột trái: Thông tin phiên gửi xe & Bảng giá
    m_infoFrame = new QFrame(this);
    m_infoFrame->setStyleSheet("background-color: #1e1e2e; border: 1px solid #313244; border-radius: 8px; padding: 14px;");
    auto infoLayout = new QVBoxLayout(m_infoFrame);
    infoLayout->setSpacing(10);

    auto infoTitle = new QLabel(QStringLiteral("CHI TIẾT PHIÊN ĐỖ XE"), m_infoFrame);
    infoTitle->setStyleSheet("font-size: 15px; font-weight: 700; color: #89b4fa;");
    infoLayout->addWidget(infoTitle);

    auto grid = new QGridLayout();
    grid->setSpacing(8);

    auto makeRow = [grid](int row, const QString& title, QLabel*& valLabel) {
        auto t = new QLabel(title);
        t->setStyleSheet("color: #a6adc8; font-size: 12px; font-weight: 500;");
        valLabel = new QLabel("-");
        valLabel->setStyleSheet("color: #cdd6f4; font-size: 13px; font-weight: 600;");
        grid->addWidget(t, row, 0);
        grid->addWidget(valLabel, row, 1);
    };

    makeRow(0, QStringLiteral("Mã Phiên (ID):"), m_sessionIdLabel);
    makeRow(1, QStringLiteral("Biển Số Xe:"), m_plateValLabel);
    makeRow(2, QStringLiteral("Loại Phương Tiện:"), m_typeValLabel);
    makeRow(3, QStringLiteral("Thời Gian Vào:"), m_timeInLabel);
    makeRow(4, QStringLiteral("Thời Gian Đỗ:"), m_durationLabel);

    infoLayout->addLayout(grid);

    infoLayout->addSpacing(8);
    auto amtTitle = new QLabel(QStringLiteral("SỐ TIỀN CẦN THANH TOÁN:"), m_infoFrame);
    amtTitle->setStyleSheet("color: #fab387; font-weight: bold; font-size: 12px;");
    infoLayout->addWidget(amtTitle);

    m_amountLabel = new QLabel(QStringLiteral("0 VNĐ"), m_infoFrame);
    m_amountLabel->setStyleSheet("font-size: 26px; font-weight: 800; color: #f9e2af; padding: 6px 0;");
    infoLayout->addWidget(m_amountLabel);

    infoLayout->addStretch();
    contentLayout->addWidget(m_infoFrame, 1);

    // Cột phải: VietQR thanh toán & Trạng thái Webhook
    m_qrFrame = new QFrame(this);
    m_qrFrame->setStyleSheet("background-color: #1e1e2e; border: 1px solid #313244; border-radius: 8px; padding: 14px;");
    auto qrLayout = new QVBoxLayout(m_qrFrame);
    qrLayout->setSpacing(10);
    qrLayout->setAlignment(Qt::AlignCenter);

    auto qrTitle = new QLabel(QStringLiteral("MÃ THANH TOÁN VIETQR"), m_qrFrame);
    qrTitle->setStyleSheet("font-size: 15px; font-weight: 700; color: #a6e3a1;");
    qrLayout->addWidget(qrTitle);

    m_qrCodeImgLabel = new QLabel(m_qrFrame);
    m_qrCodeImgLabel->setFixedSize(220, 220);
    m_qrCodeImgLabel->setStyleSheet("background-color: #ffffff; border-radius: 8px; border: 2px solid #45475a;");
    m_qrCodeImgLabel->setAlignment(Qt::AlignCenter);
    m_qrCodeImgLabel->setText(QStringLiteral("Chưa có phiên"));
    qrLayout->addWidget(m_qrCodeImgLabel);

    m_transferMemoLabel = new QLabel(QStringLiteral("Nội dung CK: -"), m_qrFrame);
    m_transferMemoLabel->setStyleSheet("color: #cdd6f4; font-weight: bold; font-size: 13px;");
    qrLayout->addWidget(m_transferMemoLabel);

    m_webhookStatusLabel = new QLabel(QStringLiteral("Đang chờ bạn chọn xe để thanh toán..."), m_qrFrame);
    m_webhookStatusLabel->setStyleSheet("color: #a6adc8; font-size: 12px; font-style: italic;");
    m_webhookStatusLabel->setAlignment(Qt::AlignCenter);
    qrLayout->addWidget(m_webhookStatusLabel);

    m_simulatePaidBtn = new QPushButton(QStringLiteral("⚡ Mô Phỏng Ngân Hàng Báo Thanh Toán Thành Công"), m_qrFrame);
    m_simulatePaidBtn->setObjectName("successBtn");
    m_simulatePaidBtn->setCursor(Qt::PointingHandCursor);
    m_simulatePaidBtn->setEnabled(false);
    qrLayout->addWidget(m_simulatePaidBtn);

    contentLayout->addWidget(m_qrFrame, 1);
    mainLayout->addLayout(contentLayout);

    // 3. Thông báo lớn: "Cảm ơn quý khách" và mở Barrier
    m_thankYouLabel = new QLabel(this);
    m_thankYouLabel->setStyleSheet(
        "font-size: 20px; font-weight: 800; color: #a6e3a1; background-color: #1a2f23; "
        "border: 2px solid #a6e3a1; border-radius: 8px; padding: 12px;"
    );
    m_thankYouLabel->setAlignment(Qt::AlignCenter);
    m_thankYouLabel->hide();
    mainLayout->addWidget(m_thankYouLabel);

    // Nút đóng
    auto bottomLayout = new QHBoxLayout();
    auto closeBtn = new QPushButton(QStringLiteral("Đóng"), this);
    closeBtn->setCursor(Qt::PointingHandCursor);
    bottomLayout->addStretch();
    bottomLayout->addWidget(closeBtn);
    mainLayout->addLayout(bottomLayout);

    connect(m_searchBtn, &QPushButton::clicked, this, &SelfCheckoutDialog::onSearchClicked);
    connect(m_rfidMockBtn, &QPushButton::clicked, this, &SelfCheckoutDialog::onRfidMockClicked);
    connect(m_simulatePaidBtn, &QPushButton::clicked, this, &SelfCheckoutDialog::onSimulatePaymentClicked);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_plateSearchEdit, &QLineEdit::returnPressed, this, &SelfCheckoutDialog::onSearchClicked);
}

void SelfCheckoutDialog::onSearchClicked() {
    QString plate = m_plateSearchEdit->text().trimmed().toUpper();
    if (plate.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Thiếu Thông Tin"), QStringLiteral("Vui lòng nhập biển số xe để tra cứu!"));
        return;
    }

    auto sessionOpt = m_repo.findActiveSessionByPlate(plate);
    if (!sessionOpt.has_value()) {
        QMessageBox::information(this, QStringLiteral("Không Tìm Thấy"),
                                 QStringLiteral("Không tìm thấy phiên gửi xe đang đỗ cho biển số '%1'!").arg(plate));
        return;
    }

    displaySessionDetails(sessionOpt.value());
}

void SelfCheckoutDialog::onRfidMockClicked() {
    auto activeSessions = m_repo.getAllActiveSessions();
    if (activeSessions.empty()) {
        QMessageBox::information(this, QStringLiteral("Bãi Trống"),
                                 QStringLiteral("Hiện không có phương tiện nào đang đỗ trong bãi!"));
        return;
    }

    // Chọn ngẫu nhiên một xe đang đỗ để mô phỏng quẹt thẻ RFID
    int idx = QRandomGenerator::global()->bounded(static_cast<int>(activeSessions.size()));
    const auto& s = activeSessions[idx];
    m_plateSearchEdit->setText(s.getLicensePlate());
    displaySessionDetails(s);
}

void SelfCheckoutDialog::displaySessionDetails(const ParkingSession& session) {
    m_currentSession = session;
    m_thankYouLabel->hide();

    QDateTime now = QDateTime::currentDateTime();
    qint64 totalSecs = session.getCheckInTime().secsTo(now);
    qint64 hours = totalSecs / 3600;
    qint64 mins = (totalSecs % 3600) / 60;

    // Tính tiền theo cấu hình PricingModel
    PricingModel pricing = m_repo.getPricingModel();
    m_currentFee = pricing.calculateFee(session.getVehicleType(), session.getCheckInTime(), now);

    // Cập nhật UI
    m_sessionIdLabel->setText(QStringLiteral("#%1").arg(session.getId()));
    m_plateValLabel->setText(session.getLicensePlate());
    m_typeValLabel->setText(VehicleUtils::getVehicleTypeName(session.getVehicleType()));
    m_timeInLabel->setText(session.getCheckInTime().toString("HH:mm:ss  dd/MM/yyyy"));
    m_durationLabel->setText(QStringLiteral("%1 giờ %2 phút").arg(hours).arg(mins));
    m_amountLabel->setText(QStringLiteral("%1 VNĐ").arg(QString::number(m_currentFee, 'f', 0)));

    // Nội dung chuyển khoản chứa ID phiên đỗ xe theo yêu cầu
    QString memo = QStringLiteral("TT %1").arg(session.getId());
    m_transferMemoLabel->setText(QStringLiteral("Nội dung CK: %1").arg(memo));

    // Hiển thị VietQR
    QImage qrImg = generateVietQRImage(session.getId(), m_currentFee, memo);
    m_qrCodeImgLabel->setPixmap(QPixmap::fromImage(qrImg));

    // Kích hoạt background worker lắng nghe webhook
    m_webhookStatusLabel->setText(QStringLiteral("⏳ Đang lắng nghe Webhook ngân hàng cho phiên #%1...").arg(session.getId()));
    m_webhookStatusLabel->setStyleSheet("color: #f9e2af; font-weight: bold;");
    m_simulatePaidBtn->setEnabled(true);

    m_webhookSimulator->startListening(session.getId(), m_currentFee, 5);
}

void SelfCheckoutDialog::onSimulatePaymentClicked() {
    m_webhookSimulator->triggerInstantPayment();
}

void SelfCheckoutDialog::onPaymentConfirmed(int sessionId, double amount, const QString& transId) {
    if (!m_currentSession.has_value() || m_currentSession->getId() != sessionId) {
        return;
    }

    // 1. Cập nhật SQLite trạng thái 'Đã thanh toán'
    m_repo.updateSessionPayment(sessionId, QDateTime::currentDateTime(), amount, QStringLiteral("Đã thanh toán"));

    // 2. Mở Barrier qua HardwareController
    if (m_hardware) {
        m_hardware->openBarrier(QStringLiteral("Đã thanh toán thành công phiên #%1 (GD: %2)").arg(sessionId).arg(transId));
    }

    // 3. Hiển thị thông báo "Cảm ơn quý khách"
    m_webhookStatusLabel->setText(QStringLiteral("✔ Đã nhận Webhook thanh toán thành công! Mã GD: %1").arg(transId));
    m_webhookStatusLabel->setStyleSheet("color: #a6e3a1; font-weight: bold;");
    m_simulatePaidBtn->setEnabled(false);

    m_thankYouLabel->setText(QStringLiteral("🎉 CẢM ƠN QUÝ KHÁCH!\nThanh toán %1 VNĐ thành công. Cổng Barrier đã mở, mời xe xuất bãi.")
                            .arg(QString::number(amount, 'f', 0)));
    m_thankYouLabel->show();
}

QImage SelfCheckoutDialog::generateVietQRImage(int sessionId, double amount, const QString& memo) {
    QImage img(220, 220, QImage::Format_RGB32);
    img.fill(Qt::white);

    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);

    // Vẽ khung VietQR Header
    p.fillRect(0, 0, 220, 36, QColor(0, 82, 155));
    p.setPen(Qt::white);
    QFont hFont("Arial", 10, QFont::Bold);
    p.setFont(hFont);
    p.drawText(QRect(0, 0, 220, 36), Qt::AlignCenter, QStringLiteral("VietQR | MB BANK"));

    // Vẽ mô phỏng ma trận QR 2D
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);

    // 3 Finder patterns ở 3 góc
    auto drawFinder = [&p](int x, int y) {
        p.fillRect(x, y, 32, 32, Qt::black);
        p.fillRect(x + 5, y + 5, 22, 22, Qt::white);
        p.fillRect(x + 10, y + 10, 12, 12, Qt::black);
    };

    drawFinder(20, 48);
    drawFinder(168, 48);
    drawFinder(20, 140);

    // Vẽ các chấm QR ma trận giả lập dựa theo sessionId
    QRandomGenerator rng(sessionId * 997 + static_cast<int>(amount));
    for (int r = 0; r < 14; ++r) {
        for (int c = 0; c < 14; ++c) {
            int px = 60 + c * 7;
            int py = 75 + r * 7;
            if (px >= 168 && py <= 80) continue;
            if (rng.bounded(10) > 4) {
                p.fillRect(px, py, 5, 5, Qt::black);
            }
        }
    }

    // Footer ghi số tiền và ID
    p.setPen(QColor(0, 82, 155));
    QFont fFont("Arial", 8, QFont::Bold);
    p.setFont(fFont);
    p.drawText(QRect(0, 192, 220, 24), Qt::AlignCenter, QStringLiteral("%1 VNĐ - %2").arg(QString::number(amount, 'f', 0), memo));

    p.end();
    return img;
}
