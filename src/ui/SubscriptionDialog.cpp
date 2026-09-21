#include "SubscriptionDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QFrame>

SubscriptionDialog::SubscriptionDialog(ParkingManager& manager, QWidget* parent)
    : QDialog(parent), m_manager(manager) {
    setupUi();
}

void SubscriptionDialog::setupUi() {
    setWindowTitle(QStringLiteral("Quản Lý & Đăng Ký Vé Tháng"));
    resize(780, 620);
    setAttribute(Qt::WA_DeleteOnClose, false);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    auto titleLabel = new QLabel(QStringLiteral("ĐĂNG KÝ & THEO DÕI VÉ THÁNG"), this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #89b4fa; letter-spacing: 0.05em;");
    mainLayout->addWidget(titleLabel);

    auto formFrame = new QFrame(this);
    formFrame->setStyleSheet("background-color: #252538; border: 1px solid #313244; border-radius: 8px; padding: 12px;");
    auto formGrid = new QGridLayout(formFrame);
    formGrid->setSpacing(10);
    formGrid->setContentsMargins(12, 12, 12, 12);

    formGrid->addWidget(new QLabel(QStringLiteral("Biển số xe:"), formFrame), 0, 0);
    m_plateEdit = new QLineEdit(formFrame);
    m_plateEdit->setPlaceholderText(QStringLiteral("Ví dụ: 29A-88888"));
    m_plateEdit->setMaxLength(15);
    formGrid->addWidget(m_plateEdit, 0, 1);

    formGrid->addWidget(new QLabel(QStringLiteral("Loại phương tiện:"), formFrame), 0, 2);
    m_typeCombo = new QComboBox(formFrame);
    m_typeCombo->addItem(VehicleUtils::getVehicleTypeName(VehicleType::MotorbikeGas), static_cast<int>(VehicleType::MotorbikeGas));
    m_typeCombo->addItem(VehicleUtils::getVehicleTypeName(VehicleType::MotorbikeElectric), static_cast<int>(VehicleType::MotorbikeElectric));
    m_typeCombo->addItem(VehicleUtils::getVehicleTypeName(VehicleType::CarGas), static_cast<int>(VehicleType::CarGas));
    m_typeCombo->addItem(VehicleUtils::getVehicleTypeName(VehicleType::CarElectric), static_cast<int>(VehicleType::CarElectric));
    formGrid->addWidget(m_typeCombo, 0, 3);

    formGrid->addWidget(new QLabel(QStringLiteral("Họ và tên chủ xe:"), formFrame), 1, 0);
    m_nameEdit = new QLineEdit(formFrame);
    m_nameEdit->setPlaceholderText(QStringLiteral("Nguyễn Văn A"));
    formGrid->addWidget(m_nameEdit, 1, 1);

    formGrid->addWidget(new QLabel(QStringLiteral("Số điện thoại:"), formFrame), 1, 2);
    m_phoneEdit = new QLineEdit(formFrame);
    m_phoneEdit->setPlaceholderText(QStringLiteral("0901234567"));
    m_phoneEdit->setMaxLength(12);
    formGrid->addWidget(m_phoneEdit, 1, 3);

    formGrid->addWidget(new QLabel(QStringLiteral("Gói thời hạn:"), formFrame), 2, 0);
    m_monthsCombo = new QComboBox(formFrame);
    m_monthsCombo->addItem(QStringLiteral("1 Tháng"), 1);
    m_monthsCombo->addItem(QStringLiteral("3 Tháng"), 3);
    m_monthsCombo->addItem(QStringLiteral("6 Tháng"), 6);
    m_monthsCombo->addItem(QStringLiteral("12 Tháng (1 Năm)"), 12);
    formGrid->addWidget(m_monthsCombo, 2, 1);

    formGrid->addWidget(new QLabel(QStringLiteral("Ngày hết hạn dự kiến:"), formFrame), 2, 2);
    m_expiryPreviewLabel = new QLabel(formFrame);
    m_expiryPreviewLabel->setStyleSheet("font-weight: bold; color: #89b4fa;");
    formGrid->addWidget(m_expiryPreviewLabel, 2, 3);

    formGrid->addWidget(new QLabel(QStringLiteral("Tổng chi phí:"), formFrame), 3, 0);
    m_pricePreviewLabel = new QLabel(formFrame);
    m_pricePreviewLabel->setObjectName("statValue");
    m_pricePreviewLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #a6e3a1;");
    formGrid->addWidget(m_pricePreviewLabel, 3, 1);

    m_registerBtn = new QPushButton(QStringLiteral("Xác Nhận Đăng Ký Gói"), formFrame);
    m_registerBtn->setObjectName("successBtn");
    m_registerBtn->setCursor(Qt::PointingHandCursor);
    formGrid->addWidget(m_registerBtn, 3, 2, 1, 2);

    mainLayout->addWidget(formFrame);

    auto listTitle = new QLabel(QStringLiteral("DANH SÁCH VÉ THÁNG TRONG HỆ THỐNG"), this);
    listTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #bac2de; margin-top: 6px;");
    mainLayout->addWidget(listTitle);

    m_subsTable = new QTableWidget(this);
    m_subsTable->setColumnCount(8);
    m_subsTable->setHorizontalHeaderLabels({
        QStringLiteral("Mã"), QStringLiteral("Biển Số"), QStringLiteral("Loại Phương Tiện"),
        QStringLiteral("Chủ Xe"), QStringLiteral("Số Điện Thoại"), QStringLiteral("Hiệu Lực Đến"),
        QStringLiteral("Số Tiền (VNĐ)"), QStringLiteral("Trạng Thái")
    });
    m_subsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_subsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_subsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(m_subsTable);

    auto bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    m_closeBtn = new QPushButton(QStringLiteral("Đóng"), this);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    bottomLayout->addWidget(m_closeBtn);
    mainLayout->addLayout(bottomLayout);

    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SubscriptionDialog::updatePricePreview);
    connect(m_monthsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SubscriptionDialog::updatePricePreview);
    connect(m_registerBtn, &QPushButton::clicked, this, &SubscriptionDialog::onRegister);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    updatePricePreview();
    loadSubscriptionTable();
}

void SubscriptionDialog::updatePricePreview() {
    VehicleType type = static_cast<VehicleType>(m_typeCombo->currentData().toInt());
    int months = m_monthsCombo->currentData().toInt();

    auto configs = m_manager.getPricingConfigs();
    double monthlyRate = 100000.0;
    for (const auto& c : configs) {
        if (c.getVehicleType() == type) {
            monthlyRate = c.getMonthlyRate();
            break;
        }
    }

    double total = monthlyRate * months;
    QDate expiry = QDate::currentDate().addMonths(months);

    m_expiryPreviewLabel->setText(expiry.toString("dd/MM/yyyy"));
    m_pricePreviewLabel->setText(QStringLiteral("%1 VNĐ").arg(QString::number(total, 'f', 0)));
}

void SubscriptionDialog::onRegister() {
    QString plate = m_plateEdit->text().trimmed().toUpper();
    if (plate.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Thiếu thông tin"), 
                             QStringLiteral("Vui lòng nhập biển số xe cần đăng ký!"));
        m_plateEdit->setFocus();
        return;
    }

    VehicleType type = static_cast<VehicleType>(m_typeCombo->currentData().toInt());
    int months = m_monthsCombo->currentData().toInt();
    QString name = m_nameEdit->text().trimmed();
    QString phone = m_phoneEdit->text().trimmed();

    if (name.isEmpty()) {
        name = QStringLiteral("Khách vãng lai");
    }

    QString msg;
    if (m_manager.registerSubscription(plate, type, name, phone, months, msg)) {
        QMessageBox::information(this, QStringLiteral("Đăng Ký Thành Công"), msg);
        m_plateEdit->clear();
        m_nameEdit->clear();
        m_phoneEdit->clear();
        loadSubscriptionTable();
    } else {
        QMessageBox::critical(this, QStringLiteral("Lỗi Đăng Ký"), msg);
    }
}

void SubscriptionDialog::loadSubscriptionTable() {
    auto list = m_manager.getAllSubscriptions();
    m_subsTable->setRowCount(static_cast<int>(list.size()));

    QDate today = QDate::currentDate();
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& sub = list[i];
        m_subsTable->setItem(i, 0, new QTableWidgetItem(QString::number(sub.getSubscriptionId())));
        m_subsTable->setItem(i, 1, new QTableWidgetItem(sub.getLicensePlate()));
        m_subsTable->setItem(i, 2, new QTableWidgetItem(VehicleUtils::getVehicleTypeName(sub.getVehicleType())));
        m_subsTable->setItem(i, 3, new QTableWidgetItem(sub.getCustomerName().isEmpty() ? "--" : sub.getCustomerName()));
        m_subsTable->setItem(i, 4, new QTableWidgetItem(sub.getPhoneNumber().isEmpty() ? "--" : sub.getPhoneNumber()));
        m_subsTable->setItem(i, 5, new QTableWidgetItem(sub.getEndDate().toString("dd/MM/yyyy")));
        m_subsTable->setItem(i, 6, new QTableWidgetItem(QString::number(sub.getPricePaid(), 'f', 0)));

        bool isActive = sub.isValidOn(today);
        auto statusItem = new QTableWidgetItem(isActive ? QStringLiteral("Còn hạn") : QStringLiteral("Đã hết hạn"));
        if (isActive) {
            statusItem->setForeground(QBrush(QColor("#a6e3a1")));
        } else {
            statusItem->setForeground(QBrush(QColor("#f38ba8")));
        }
        m_subsTable->setItem(i, 7, statusItem);
    }
}
