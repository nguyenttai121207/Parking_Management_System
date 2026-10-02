#include "PricingAdminDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFrame>

PricingAdminDialog::PricingAdminDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
    loadConfigs();
}

void PricingAdminDialog::setupUi() {
    setWindowTitle(QStringLiteral("Cấu Hình Biểu Phí Bãi Giữ Xe"));
    setMinimumSize(680, 460);
    resize(680, 460);
    setAttribute(Qt::WA_DeleteOnClose, false);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    auto titleLabel = new QLabel(QStringLiteral("CẤU HÌNH BIỂU PHÍ THEO LOẠI XE"), this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #89b4fa; letter-spacing: 0.04em;");
    mainLayout->addWidget(titleLabel);

    auto descLabel = new QLabel(QStringLiteral("Biểu phí được áp dụng tự động cho module Self-Checkout để tính tiền gửi xe."), this);
    descLabel->setStyleSheet("color: #a6adc8; font-size: 12px; margin-bottom: 4px;");
    mainLayout->addWidget(descLabel);

    auto formFrame = new QFrame(this);
    formFrame->setStyleSheet("background-color: #252538; border: 1px solid #313244; border-radius: 8px; padding: 12px;");
    auto grid = new QGridLayout(formFrame);
    grid->setVerticalSpacing(12);
    grid->setHorizontalSpacing(12);
    grid->setContentsMargins(16, 16, 16, 16);
    grid->setColumnStretch(0, 2);
    grid->setColumnStretch(1, 3);
    grid->setColumnStretch(2, 3);
    grid->setColumnMinimumWidth(0, 120);
    grid->setRowMinimumHeight(0, 40); // header row

    // Tiêu đề cột
    auto headerType = new QLabel(QStringLiteral("Loại xe"), formFrame);
    headerType->setMinimumHeight(40);
    headerType->setStyleSheet("font-weight: bold; color: #89b4fa; font-size: 13px; padding: 6px 0;");

    auto headerFirst = new QLabel(QStringLiteral("Phí block đầu (VNĐ)"), formFrame);
    headerFirst->setMinimumHeight(40);
    headerFirst->setStyleSheet("font-weight: bold; color: #89b4fa; font-size: 13px; padding: 6px 4px;");

    auto headerNext = new QLabel(QStringLiteral("Phí block tiếp theo (VNĐ)"), formFrame);
    headerNext->setMinimumHeight(40);
    headerNext->setStyleSheet("font-weight: bold; color: #89b4fa; font-size: 13px; padding: 6px 4px;");

    grid->addWidget(headerType, 0, 0);
    grid->addWidget(headerFirst, 0, 1);
    grid->addWidget(headerNext, 0, 2);

    // Sửa lỗi mất label ở cột 1, map cứng danh sách text: 'Xe đạp', 'Xe máy số', 'Xe tay ga', 'Ô tô con'
    struct HardcodedType {
        VehicleType type;
        const char* name;
    };

    const HardcodedType fixedTypes[] = {
        {VehicleType::Bicycle, "Xe đạp"},
        {VehicleType::MotorbikeManual, "Xe máy số"},
        {VehicleType::MotorbikeScooter, "Xe tay ga"},
        {VehicleType::Car, "Ô tô con"}
    };

    int row = 1;
    m_rows.clear();

    for (const auto& item : fixedTypes) {
        auto lbl = new QLabel(QString::fromUtf8(item.name), formFrame);
        lbl->setStyleSheet(
            "color: #cdd6f4;"
            "font-weight: 600;"
            "font-size: 13px;"
            "padding: 2px 6px;"
            "background: transparent;"
        );
        lbl->setMinimumHeight(40);
        lbl->setMinimumWidth(120);
        lbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        grid->addWidget(lbl, row, 0);

        // ponytail: ::up-button/::down-button phải được style tường minh,
        // nếu không Qt dùng native size (~20px mỗi nút) vượt min-height:28px
        const QString spinStyle = QStringLiteral(
            "QDoubleSpinBox {"
            "  min-height: 36px;"
            "  padding: 2px 4px;"
            "}"
            "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {"
            "  width: 18px;"
            "  border: none;"
            "}"
        );

        auto firstSpin = new QDoubleSpinBox(formFrame);
        firstSpin->setRange(0.0, 5000000.0);
        firstSpin->setSingleStep(1000.0);
        firstSpin->setDecimals(0);
        firstSpin->setSuffix(QStringLiteral(" đ"));
        firstSpin->setMinimumWidth(150);
        firstSpin->setStyleSheet(spinStyle);
        grid->addWidget(firstSpin, row, 1);

        auto nextSpin = new QDoubleSpinBox(formFrame);
        nextSpin->setRange(0.0, 5000000.0);
        nextSpin->setSingleStep(1000.0);
        nextSpin->setDecimals(0);
        nextSpin->setSuffix(QStringLiteral(" đ"));
        nextSpin->setMinimumWidth(150);
        nextSpin->setStyleSheet(spinStyle);
        grid->addWidget(nextSpin, row, 2);

        m_rows.push_back({item.type, QString::fromUtf8(item.name), firstSpin, nextSpin});
        row++;
    }

    mainLayout->addWidget(formFrame);

    auto btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    m_cancelBtn = new QPushButton(QStringLiteral("Hủy"), this);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);

    m_saveBtn = new QPushButton(QStringLiteral("Lưu Biểu Phí"), this);
    m_saveBtn->setObjectName("successBtn");
    m_saveBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addStretch();
    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addWidget(m_saveBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_saveBtn, &QPushButton::clicked, this, &PricingAdminDialog::onSave);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void PricingAdminDialog::loadConfigs() {
    PricingModel model = m_repo.getPricingModel();

    // Điền chính xác dữ liệu cấu hình vào từng dòng mà không bị render rỗng
    for (auto& row : m_rows) {
        PricingRate rate = model.getRate(row.type);
        row.firstBlockSpin->setValue(rate.firstBlockFee);
        row.nextBlockSpin->setValue(rate.nextBlockFee);
    }
}

void PricingAdminDialog::onSave() {
    PricingModel model;
    for (const auto& row : m_rows) {
        model.setRate(row.type, row.firstBlockSpin->value(), row.nextBlockSpin->value());
    }

    if (m_repo.savePricingModel(model)) {
        QMessageBox::information(this, QStringLiteral("Cập Nhật Thành Công"),
                                 QStringLiteral("Biểu phí đã được lưu thành công vào cơ sở dữ liệu!"));
        accept();
    } else {
        QMessageBox::critical(this, QStringLiteral("Lỗi Lưu Dữ Liệu"),
                              QStringLiteral("Đã xảy ra lỗi khi ghi dữ liệu cấu hình biểu phí vào CSDL!"));
    }
}
