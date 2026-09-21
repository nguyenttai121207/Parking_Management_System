#include "PricingAdminDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFrame>

PricingAdminDialog::PricingAdminDialog(ParkingManager& manager, QWidget* parent)
    : QDialog(parent), m_manager(manager) {
    setupUi();
}

void PricingAdminDialog::setupUi() {
    setWindowTitle(QStringLiteral("Cấu Hình Biểu Phí Bãi Giữ Xe"));
    setFixedSize(580, 380);
    setAttribute(Qt::WA_DeleteOnClose, false);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    auto titleLabel = new QLabel(QStringLiteral("QUẢN LÝ BIỂU PHÍ THEO LOẠI XE"), this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #89b4fa; letter-spacing: 0.05em;");
    mainLayout->addWidget(titleLabel);

    auto descLabel = new QLabel(QStringLiteral("Đơn giá được áp dụng ngay lập tức cho các lượt tính phí tiếp theo trong hệ thống."), this);
    descLabel->setStyleSheet("color: #a6adc8; font-size: 12px; margin-bottom: 4px;");
    mainLayout->addWidget(descLabel);

    auto formFrame = new QFrame(this);
    formFrame->setStyleSheet("background-color: #252538; border: 1px solid #313244; border-radius: 8px; padding: 12px;");
    auto grid = new QGridLayout(formFrame);
    grid->setSpacing(12);
    grid->setContentsMargins(12, 12, 12, 12);

    auto headerType = new QLabel(QStringLiteral("Loại Phương Tiện"), formFrame);
    headerType->setStyleSheet("font-weight: bold; color: #bac2de;");
    auto headerHourly = new QLabel(QStringLiteral("Giá Theo Giờ (VNĐ)"), formFrame);
    headerHourly->setStyleSheet("font-weight: bold; color: #bac2de;");
    auto headerMonthly = new QLabel(QStringLiteral("Giá Gói Tháng (VNĐ)"), formFrame);
    headerMonthly->setStyleSheet("font-weight: bold; color: #bac2de;");

    grid->addWidget(headerType, 0, 0);
    grid->addWidget(headerHourly, 0, 1);
    grid->addWidget(headerMonthly, 0, 2);

    VehicleType types[] = {
        VehicleType::MotorbikeGas,
        VehicleType::MotorbikeElectric,
        VehicleType::CarGas,
        VehicleType::CarElectric
    };

    int row = 1;
    for (auto t : types) {
        auto lbl = new QLabel(VehicleUtils::getVehicleTypeName(t), formFrame);
        lbl->setStyleSheet("color: #cdd6f4; font-weight: 500;");
        grid->addWidget(lbl, row, 0);

        auto hourly = new QDoubleSpinBox(formFrame);
        hourly->setRange(1000.0, 5000000.0);
        hourly->setSingleStep(1000.0);
        hourly->setDecimals(0);
        hourly->setSuffix(QStringLiteral(" đ"));

        auto monthly = new QDoubleSpinBox(formFrame);
        monthly->setRange(10000.0, 50000000.0);
        monthly->setSingleStep(50000.0);
        monthly->setDecimals(0);
        monthly->setSuffix(QStringLiteral(" đ"));

        grid->addWidget(hourly, row, 1);
        grid->addWidget(monthly, row, 2);

        m_rows.push_back({t, hourly, monthly});
        row++;
    }
    mainLayout->addWidget(formFrame);

    auto btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    m_cancelBtn = new QPushButton(QStringLiteral("Hủy"), this);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);

    m_saveBtn = new QPushButton(QStringLiteral("Lưu Bảng Giá"), this);
    m_saveBtn->setObjectName("successBtn");
    m_saveBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addStretch();
    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addWidget(m_saveBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_saveBtn, &QPushButton::clicked, this, &PricingAdminDialog::onSave);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    loadConfigs();
}

void PricingAdminDialog::loadConfigs() {
    auto configs = m_manager.getPricingConfigs();
    for (const auto& c : configs) {
        for (auto& row : m_rows) {
            if (row.type == c.getVehicleType()) {
                row.hourlySpin->setValue(c.getHourlyRate());
                row.monthlySpin->setValue(c.getMonthlyRate());
                break;
            }
        }
    }
}

void PricingAdminDialog::onSave() {
    bool allSuccess = true;
    for (const auto& row : m_rows) {
        PricingConfig config(row.type, row.hourlySpin->value(), row.monthlySpin->value());
        if (!m_manager.updatePricingConfig(config)) {
            allSuccess = false;
        }
    }

    if (allSuccess) {
        QMessageBox::information(this, QStringLiteral("Cập Nhật Thành Công"), 
                                 QStringLiteral("Biểu phí các loại phương tiện đã được lưu thành công vào cơ sở dữ liệu!"));
        accept();
    } else {
        QMessageBox::critical(this, QStringLiteral("Lỗi Lưu Dữ Liệu"), 
                             QStringLiteral("Đã xảy ra lỗi khi ghi dữ liệu cấu hình vào CSDL!"));
    }
}
