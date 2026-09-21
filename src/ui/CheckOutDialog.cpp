#include "CheckOutDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>

CheckOutDialog::CheckOutDialog(ParkingManager& manager, const QString& defaultPlate, QWidget* parent)
    : QDialog(parent), m_manager(manager) {
    setupUi(defaultPlate);
}

void CheckOutDialog::setupUi(const QString& defaultPlate) {
    setWindowTitle(QStringLiteral("Check-out & Xuất Bãi Phương Tiện"));
    setFixedSize(520, 500);
    setAttribute(Qt::WA_DeleteOnClose, false);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    auto titleLabel = new QLabel(QStringLiteral("CHECK-OUT & THANH TOÁN XUẤT BÃI"), this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #89b4fa; letter-spacing: 0.05em;");
    mainLayout->addWidget(titleLabel);

    auto searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(8);

    m_plateEdit = new QLineEdit(this);
    m_plateEdit->setPlaceholderText(QStringLiteral("Nhập hoặc quét biển số xe..."));
    m_plateEdit->setText(defaultPlate.trimmed().toUpper());
    m_plateEdit->setMaxLength(15);

    m_searchBtn = new QPushButton(QStringLiteral("Tra Cứu & Tính Phí"), this);
    m_searchBtn->setObjectName("primaryBtn");
    m_searchBtn->setCursor(Qt::PointingHandCursor);

    searchLayout->addWidget(m_plateEdit);
    searchLayout->addWidget(m_searchBtn);
    mainLayout->addLayout(searchLayout);

    m_receiptFrame = new QFrame(this);
    m_receiptFrame->setObjectName("receiptFrame");
    m_receiptFrame->setStyleSheet("QFrame#receiptFrame { background-color: #252538; border: 1px solid #313244; border-radius: 8px; }");
    auto formLayout = new QFormLayout(m_receiptFrame);
    formLayout->setSpacing(10);
    formLayout->setContentsMargins(16, 16, 16, 16);

    m_plateVal = new QLabel("--", m_receiptFrame);
    m_plateVal->setStyleSheet("font-weight: bold; color: #cdd6f4;");

    m_vTypeVal = new QLabel("--", m_receiptFrame);
    m_slotVal = new QLabel("--", m_receiptFrame);
    m_inTimeVal = new QLabel("--", m_receiptFrame);
    m_outTimeVal = new QLabel("--", m_receiptFrame);
    m_durationVal = new QLabel("--", m_receiptFrame);
    m_pricingTypeVal = new QLabel("--", m_receiptFrame);

    m_totalFeeVal = new QLabel(QStringLiteral("0 VNĐ"), m_receiptFrame);
    m_totalFeeVal->setObjectName("statValue");
    m_totalFeeVal->setStyleSheet("font-size: 22px; font-weight: bold; color: #a6e3a1;");

    formLayout->addRow(QStringLiteral("Biển số xe:"), m_plateVal);
    formLayout->addRow(QStringLiteral("Loại phương tiện:"), m_vTypeVal);
    formLayout->addRow(QStringLiteral("Vị trí đỗ hiện tại:"), m_slotVal);
    formLayout->addRow(QStringLiteral("Thời gian vào bãi:"), m_inTimeVal);
    formLayout->addRow(QStringLiteral("Thời gian xuất bãi:"), m_outTimeVal);
    formLayout->addRow(QStringLiteral("Thời lượng gửi:"), m_durationVal);
    formLayout->addRow(QStringLiteral("Loại vé áp dụng:"), m_pricingTypeVal);
    formLayout->addRow(QStringLiteral("TỔNG TIỀN PHẢI THU:"), m_totalFeeVal);

    mainLayout->addWidget(m_receiptFrame);

    auto btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    m_closeBtn = new QPushButton(QStringLiteral("Đóng"), this);
    m_closeBtn->setCursor(Qt::PointingHandCursor);

    m_confirmCheckoutBtn = new QPushButton(QStringLiteral("Xác Nhận Thanh Toán & Mở Barrier"), this);
    m_confirmCheckoutBtn->setObjectName("successBtn");
    m_confirmCheckoutBtn->setCursor(Qt::PointingHandCursor);
    m_confirmCheckoutBtn->setEnabled(false);

    btnLayout->addStretch();
    btnLayout->addWidget(m_closeBtn);
    btnLayout->addWidget(m_confirmCheckoutBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_searchBtn, &QPushButton::clicked, this, &CheckOutDialog::onSearchAndCalculate);
    connect(m_plateEdit, &QLineEdit::returnPressed, this, &CheckOutDialog::onSearchAndCalculate);
    connect(m_confirmCheckoutBtn, &QPushButton::clicked, this, &CheckOutDialog::onConfirmCheckout);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    if (!defaultPlate.isEmpty()) {
        onSearchAndCalculate();
    }
}

void CheckOutDialog::resetReceiptView() {
    m_plateVal->setText("--");
    m_vTypeVal->setText("--");
    m_slotVal->setText("--");
    m_inTimeVal->setText("--");
    m_outTimeVal->setText("--");
    m_durationVal->setText("--");
    m_pricingTypeVal->setText("--");
    m_totalFeeVal->setText(QStringLiteral("0 VNĐ"));
    m_confirmCheckoutBtn->setEnabled(false);
}

void CheckOutDialog::onSearchAndCalculate() {
    QString plate = m_plateEdit->text().trimmed().toUpper();
    if (plate.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Thiếu thông tin"), 
                             QStringLiteral("Vui lòng nhập hoặc quét biển số xe!"));
        m_plateEdit->setFocus();
        return;
    }

    m_currentReceipt = m_manager.previewCheckOut(plate);
    if (!m_currentReceipt.success) {
        resetReceiptView();
        QMessageBox::warning(this, QStringLiteral("Không tìm thấy dữ liệu"), m_currentReceipt.message);
        return;
    }

    m_plateVal->setText(m_currentReceipt.ticket.getLicensePlate());
    m_vTypeVal->setText(VehicleUtils::getVehicleTypeName(m_currentReceipt.ticket.getVehicleType()));
    m_slotVal->setText(m_currentReceipt.ticket.getSlotNumber());
    m_inTimeVal->setText(m_currentReceipt.ticket.getCheckInTime().toString("HH:mm:ss - dd/MM/yyyy"));
    m_outTimeVal->setText(m_currentReceipt.checkOutTime.toString("HH:mm:ss - dd/MM/yyyy"));

    qint64 totalSecs = m_currentReceipt.ticket.getCheckInTime().secsTo(m_currentReceipt.checkOutTime);
    qint64 totalMinutes = totalSecs / 60;
    int durationHours = static_cast<int>(totalMinutes / 60);
    int durationMins = static_cast<int>(totalMinutes % 60);

    if (m_currentReceipt.isMonthlySubscriber) {
        m_durationVal->setText(QStringLiteral("%1 giờ %2 phút").arg(durationHours).arg(durationMins));
        m_pricingTypeVal->setText(QStringLiteral("<span style='color: #a6e3a1; font-weight: bold;'>VÉ THÁNG (Miễn phí lượt)</span>"));
    } else {
        m_durationVal->setText(QStringLiteral("%1 giờ %2 phút (Tính %3 giờ)")
                                  .arg(durationHours).arg(durationMins).arg(m_currentReceipt.billableHours));
        m_pricingTypeVal->setText(QStringLiteral("Vé lượt (Theo giờ)"));
    }

    m_totalFeeVal->setText(QStringLiteral("%1 VNĐ").arg(QString::number(m_currentReceipt.fee, 'f', 0)));
    m_confirmCheckoutBtn->setEnabled(true);
}

void CheckOutDialog::onConfirmCheckout() {
    if (!m_currentReceipt.success) return;

    if (m_manager.confirmCheckOut(m_currentReceipt)) {
        QMessageBox::information(this, QStringLiteral("Xuất Bãi Thành Công"),
            QStringLiteral("<b>THANH TOÁN THÀNH CÔNG!</b><br><br>"
                           "Biển số: <b>%1</b><br>"
                           "Vị trí giải phóng: <b>%2</b><br>"
                           "Số tiền thu: <b>%3 VNĐ</b><br><br>"
                           "<i>Hệ thống phát tín hiệu: <b>MỞ BARRIER CHO XE XUẤT BÃI!</b></i>")
                .arg(m_currentReceipt.ticket.getLicensePlate(),
                     m_currentReceipt.ticket.getSlotNumber(),
                     QString::number(m_currentReceipt.fee, 'f', 0)));
        accept();
    } else {
        QMessageBox::critical(this, QStringLiteral("Lỗi xử lý"), 
                             QStringLiteral("Không thể hoàn tất check-out trên hệ thống CSDL!"));
    }
}
