#include "CheckInDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFrame>

CheckInDialog::CheckInDialog(ParkingManager& manager, QWidget* parent)
    : QDialog(parent), m_manager(manager) {
    setupUi();
}

void CheckInDialog::setupUi() {
    setWindowTitle(QStringLiteral("Check-in Phương Tiện Vào Bãi"));
    setFixedSize(440, 320);
    setAttribute(Qt::WA_DeleteOnClose, false);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    auto titleLabel = new QLabel(QStringLiteral("CHECK-IN PHƯƠNG TIỆN"), this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #89b4fa; letter-spacing: 0.05em;");
    mainLayout->addWidget(titleLabel);

    auto formFrame = new QFrame(this);
    formFrame->setObjectName("formFrame");
    formFrame->setStyleSheet("QFrame#formFrame { background-color: #252538; border: 1px solid #313244; border-radius: 8px; }");
    auto formLayout = new QFormLayout(formFrame);
    formLayout->setSpacing(12);
    formLayout->setContentsMargins(16, 16, 16, 16);

    m_plateEdit = new QLineEdit(formFrame);
    m_plateEdit->setPlaceholderText(QStringLiteral("Ví dụ: 29A-12345, 59B1-99999"));
    m_plateEdit->setMaxLength(15);
    formLayout->addRow(QStringLiteral("Biển số xe:"), m_plateEdit);

    m_typeCombo = new QComboBox(formFrame);
    m_typeCombo->addItem(VehicleUtils::getVehicleTypeName(VehicleType::MotorbikeGas), static_cast<int>(VehicleType::MotorbikeGas));
    m_typeCombo->addItem(VehicleUtils::getVehicleTypeName(VehicleType::MotorbikeElectric), static_cast<int>(VehicleType::MotorbikeElectric));
    m_typeCombo->addItem(VehicleUtils::getVehicleTypeName(VehicleType::CarGas), static_cast<int>(VehicleType::CarGas));
    m_typeCombo->addItem(VehicleUtils::getVehicleTypeName(VehicleType::CarElectric), static_cast<int>(VehicleType::CarElectric));
    formLayout->addRow(QStringLiteral("Loại xe:"), m_typeCombo);

    m_ticketTypeCombo = new QComboBox(formFrame);
    m_ticketTypeCombo->addItem(QStringLiteral("Vé lượt (Tính theo giờ)"), 0);
    m_ticketTypeCombo->addItem(QStringLiteral("Vé tháng (Khách đăng ký gói)"), 1);
    formLayout->addRow(QStringLiteral("Hình thức vé:"), m_ticketTypeCombo);

    m_statusNoteLabel = new QLabel(formFrame);
    m_statusNoteLabel->setStyleSheet("font-size: 12px; color: #a6adc8;");
    formLayout->addRow(QStringLiteral("Tình trạng chỗ:"), m_statusNoteLabel);

    mainLayout->addWidget(formFrame);

    auto btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    m_cancelBtn = new QPushButton(QStringLiteral("Hủy"), this);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);

    m_confirmBtn = new QPushButton(QStringLiteral("Xác Nhận Check-in"), this);
    m_confirmBtn->setObjectName("primaryBtn");
    m_confirmBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addStretch();
    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addWidget(m_confirmBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_confirmBtn, &QPushButton::clicked, this, &CheckInDialog::onConfirm);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CheckInDialog::updateSlotAvailabilityHint);
    connect(m_ticketTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CheckInDialog::onTicketTypeChanged);

    updateSlotAvailabilityHint();
}

void CheckInDialog::updateSlotAvailabilityHint() {
    VehicleType type = static_cast<VehicleType>(m_typeCombo->currentData().toInt());
    SlotType slotType = VehicleUtils::getSlotTypeForVehicle(type);
    
    DashboardStats stats = m_manager.getDashboardStats();
    int total = (slotType == SlotType::MotorbikeSlot) ? stats.totalMotorbikeSlots : stats.totalCarSlots;
    int occupied = (slotType == SlotType::MotorbikeSlot) ? stats.occupiedMotorbikeSlots : stats.occupiedCarSlots;
    int available = total - occupied;

    if (available > 0) {
        m_statusNoteLabel->setText(QStringLiteral("<span style='color: #a6e3a1;'>Còn %1 / %2 chỗ trống (%3)</span>")
                                      .arg(available).arg(total).arg(VehicleUtils::getSlotTypeName(slotType)));
    } else {
        m_statusNoteLabel->setText(QStringLiteral("<span style='color: #f38ba8;'>Hết chỗ cho %1!</span>")
                                      .arg(VehicleUtils::getSlotTypeName(slotType)));
    }
}

void CheckInDialog::onTicketTypeChanged(int index) {
    if (index == 1) {
        m_statusNoteLabel->setToolTip(QStringLiteral("Hệ thống sẽ kiểm tra hạn vé tháng của biển số khi check-in."));
    }
}

void CheckInDialog::onConfirm() {
    QString plate = m_plateEdit->text().trimmed().toUpper();
    if (plate.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Thiếu thông tin"), 
                             QStringLiteral("Vui lòng nhập biển số xe trước khi thực hiện check-in!"));
        m_plateEdit->setFocus();
        return;
    }

    VehicleType type = static_cast<VehicleType>(m_typeCombo->currentData().toInt());
    bool isMonthlyRequested = (m_ticketTypeCombo->currentData().toInt() == 1);

    if (isMonthlyRequested) {
        auto subList = m_manager.getAllSubscriptions();
        bool hasActiveSub = false;
        QDate today = QDate::currentDate();
        for (const auto& sub : subList) {
            if (sub.getLicensePlate().compare(plate, Qt::CaseInsensitive) == 0 && sub.isValidOn(today)) {
                hasActiveSub = true;
                break;
            }
        }

        if (!hasActiveSub) {
            auto reply = QMessageBox::question(this, QStringLiteral("Chưa có vé tháng"),
                QStringLiteral("Biển số %1 chưa đăng ký gói vé tháng còn hiệu lực!\nBạn có muốn chuyển sang check-in bằng Vé Lượt không?")
                    .arg(plate),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            
            if (reply == QMessageBox::Yes) {
                m_ticketTypeCombo->setCurrentIndex(0);
            } else {
                return;
            }
        }
    }

    CheckInResult result = m_manager.checkIn(type, plate);

    if (result.success) {
        QString slotName = result.ticket.has_value() ? result.ticket->getSlotNumber() : QStringLiteral("Đã cấp");
        QMessageBox::information(this, QStringLiteral("Check-in Thành Công"),
            QStringLiteral("Phương tiện: <b>%1</b> (%2)\nVị trí đỗ gán: <b>%3</b>\nThời gian vào: %4")
                .arg(plate, VehicleUtils::getVehicleTypeName(type), slotName, 
                     QDateTime::currentDateTime().toString("HH:mm:ss - dd/MM/yyyy")));
        accept();
    } else {
        QMessageBox::critical(this, QStringLiteral("Không Thể Check-in"), result.message);
    }
}
