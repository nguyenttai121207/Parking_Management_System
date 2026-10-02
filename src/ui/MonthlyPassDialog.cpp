#include "MonthlyPassDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QFrame>

MonthlyPassDialog::MonthlyPassDialog(QWidget* parent)
    : QDialog(parent), m_selectedPassId(-1) {
    setupUi();
    loadPasses();
}

void MonthlyPassDialog::setupUi() {
    setWindowTitle(QStringLiteral("Quản Lý Vé Tháng (Monthly Pass Manager)"));
    resize(920, 600);
    setAttribute(Qt::WA_DeleteOnClose, false);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto title = new QLabel(QStringLiteral("QUẢN LÝ VÉ THÁNG PHƯƠNG TIỆN"), this);
    title->setObjectName("titleLabel");
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #89b4fa;");
    mainLayout->addWidget(title);

    // 1. Form nhập liệu thêm / sửa / xóa
    auto formCard = new QFrame(this);
    formCard->setStyleSheet("background-color: #252538; border: 1px solid #313244; border-radius: 8px; padding: 12px;");
    auto formLayout = new QGridLayout(formCard);
    formLayout->setSpacing(10);

    formLayout->addWidget(new QLabel(QStringLiteral("Tên Khách Hàng:"), formCard), 0, 0);
    m_customerNameEdit = new QLineEdit(formCard);
    m_customerNameEdit->setPlaceholderText(QStringLiteral("Ví dụ: Nguyễn Văn A"));
    formLayout->addWidget(m_customerNameEdit, 0, 1);

    formLayout->addWidget(new QLabel(QStringLiteral("Biển Số Xe:"), formCard), 0, 2);
    m_licensePlateEdit = new QLineEdit(formCard);
    m_licensePlateEdit->setPlaceholderText(QStringLiteral("Ví dụ: 29A-839.21"));
    formLayout->addWidget(m_licensePlateEdit, 0, 3);

    formLayout->addWidget(new QLabel(QStringLiteral("Loại Phương Tiện:"), formCard), 1, 0);
    m_vehicleTypeCombo = new QComboBox(formCard);
    m_vehicleTypeCombo->addItem(QStringLiteral("Xe đạp"), static_cast<int>(VehicleType::Bicycle));
    m_vehicleTypeCombo->addItem(QStringLiteral("Xe máy số"), static_cast<int>(VehicleType::MotorbikeManual));
    m_vehicleTypeCombo->addItem(QStringLiteral("Xe tay ga"), static_cast<int>(VehicleType::MotorbikeScooter));
    m_vehicleTypeCombo->addItem(QStringLiteral("Ô tô con"), static_cast<int>(VehicleType::Car));
    formLayout->addWidget(m_vehicleTypeCombo, 1, 1);

    formLayout->addWidget(new QLabel(QStringLiteral("Ngày Bắt Đầu:"), formCard), 1, 2);
    m_startDateEdit = new QDateEdit(QDate::currentDate(), formCard);
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDisplayFormat("dd/MM/yyyy");
    formLayout->addWidget(m_startDateEdit, 1, 3);

    formLayout->addWidget(new QLabel(QStringLiteral("Ngày Hết Hạn:"), formCard), 2, 2);
    m_expirationDateEdit = new QDateEdit(QDate::currentDate().addMonths(1), formCard);
    m_expirationDateEdit->setCalendarPopup(true);
    m_expirationDateEdit->setDisplayFormat("dd/MM/yyyy");
    formLayout->addWidget(m_expirationDateEdit, 2, 3);

    // Hàng nút thao tác
    auto btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(8);

    m_addBtn = new QPushButton(QStringLiteral("➕ Thêm Vé Tháng"), formCard);
    m_addBtn->setObjectName("successBtn");
    m_addBtn->setCursor(Qt::PointingHandCursor);

    m_updateBtn = new QPushButton(QStringLiteral("✏️ Cập Nhật"), formCard);
    m_updateBtn->setObjectName("primaryBtn");
    m_updateBtn->setCursor(Qt::PointingHandCursor);
    m_updateBtn->setEnabled(false);

    m_deleteBtn = new QPushButton(QStringLiteral("🗑️ Xóa"), formCard);
    m_deleteBtn->setObjectName("dangerBtn");
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    m_deleteBtn->setEnabled(false);

    m_clearBtn = new QPushButton(QStringLiteral("Làm Mới Form"), formCard);
    m_clearBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_updateBtn);
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addStretch();

    formLayout->addLayout(btnLayout, 3, 0, 1, 4);
    mainLayout->addWidget(formCard);

    // 2. Thanh lọc tìm kiếm
    auto searchLayout = new QHBoxLayout();
    searchLayout->addWidget(new QLabel(QStringLiteral("Tìm kiếm biển số / khách hàng:"), this));
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(QStringLiteral("Nhập biển số để lọc nhanh..."));
    searchLayout->addWidget(m_searchEdit);
    mainLayout->addLayout(searchLayout);

    // 3. Bảng danh sách vé tháng
    m_table = new QTableWidget(this);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({
        QStringLiteral("ID"), QStringLiteral("Tên Khách Hàng"), QStringLiteral("Biển Số Xe"),
        QStringLiteral("Loại Xe"), QStringLiteral("Ngày Bắt Đầu"), QStringLiteral("Ngày Hết Hạn"),
        QStringLiteral("Trạng Thái")
    });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(m_table);

    // Nút đóng
    auto bottomLayout = new QHBoxLayout();
    auto closeBtn = new QPushButton(QStringLiteral("Đóng"), this);
    closeBtn->setCursor(Qt::PointingHandCursor);
    bottomLayout->addStretch();
    bottomLayout->addWidget(closeBtn);
    mainLayout->addLayout(bottomLayout);

    connect(m_addBtn, &QPushButton::clicked, this, &MonthlyPassDialog::onAddClicked);
    connect(m_updateBtn, &QPushButton::clicked, this, &MonthlyPassDialog::onUpdateClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &MonthlyPassDialog::onDeleteClicked);
    connect(m_clearBtn, &QPushButton::clicked, this, &MonthlyPassDialog::onClearForm);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &MonthlyPassDialog::onTableSelectionChanged);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MonthlyPassDialog::onSearchChanged);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void MonthlyPassDialog::loadPasses() {
    auto passes = m_repo.getAllMonthlyPasses();
    QString filter = m_searchEdit->text().trimmed().toUpper();

    m_table->setRowCount(0);
    int row = 0;
    QDate today = QDate::currentDate();

    for (const auto& p : passes) {
        if (!filter.isEmpty() && !p.getLicensePlate().contains(filter) && !p.getCustomerName().toUpper().contains(filter)) {
            continue;
        }

        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(p.getId())));
        m_table->setItem(row, 1, new QTableWidgetItem(p.getCustomerName()));
        m_table->setItem(row, 2, new QTableWidgetItem(p.getLicensePlate()));
        m_table->setItem(row, 3, new QTableWidgetItem(VehicleUtils::getVehicleTypeName(p.getVehicleType())));
        m_table->setItem(row, 4, new QTableWidgetItem(p.getStartDate().toString("dd/MM/yyyy")));
        m_table->setItem(row, 5, new QTableWidgetItem(p.getExpirationDate().toString("dd/MM/yyyy")));

        bool valid = p.isValid(today);
        auto statusItem = new QTableWidgetItem(valid ? QStringLiteral("Còn Hạn") : QStringLiteral("Hết Hạn"));
        statusItem->setForeground(valid ? QBrush(QColor(0xA6, 0xE3, 0xA1)) : QBrush(QColor(0xF3, 0x8B, 0xA8)));
        m_table->setItem(row, 6, statusItem);

        row++;
    }
}

void MonthlyPassDialog::onAddClicked() {
    QString name = m_customerNameEdit->text().trimmed();
    QString plate = m_licensePlateEdit->text().trimmed().toUpper();
    VehicleType vType = static_cast<VehicleType>(m_vehicleTypeCombo->currentData().toInt());
    QDate start = m_startDateEdit->date();
    QDate end = m_expirationDateEdit->date();

    if (name.isEmpty() || plate.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Thiếu Thông Tin"), QStringLiteral("Vui lòng nhập tên khách hàng và biển số xe!"));
        return;
    }

    if (end < start) {
        QMessageBox::warning(this, QStringLiteral("Ngày Không Hợp Lệ"), QStringLiteral("Ngày hết hạn phải sau ngày bắt đầu!"));
        return;
    }

    MonthlyPass pass(0, name, plate, vType, start, end);
    if (m_repo.addMonthlyPass(pass)) {
        QMessageBox::information(this, QStringLiteral("Thành Công"), QStringLiteral("Đã thêm vé tháng mới cho xe '%1'!").arg(plate));
        onClearForm();
        loadPasses();
    } else {
        QMessageBox::critical(this, QStringLiteral("Lỗi"), QStringLiteral("Không thể thêm vé tháng! Biển số có thể đã tồn tại trong danh sách."));
    }
}

void MonthlyPassDialog::onUpdateClicked() {
    if (m_selectedPassId < 0) return;

    QString name = m_customerNameEdit->text().trimmed();
    QString plate = m_licensePlateEdit->text().trimmed().toUpper();
    VehicleType vType = static_cast<VehicleType>(m_vehicleTypeCombo->currentData().toInt());
    QDate start = m_startDateEdit->date();
    QDate end = m_expirationDateEdit->date();

    MonthlyPass pass(m_selectedPassId, name, plate, vType, start, end);
    if (m_repo.updateMonthlyPass(pass)) {
        QMessageBox::information(this, QStringLiteral("Thành Công"), QStringLiteral("Đã cập nhật vé tháng cho xe '%1'!").arg(plate));
        onClearForm();
        loadPasses();
    } else {
        QMessageBox::critical(this, QStringLiteral("Lỗi"), QStringLiteral("Lỗi cập nhật vé tháng vào CSDL!"));
    }
}

void MonthlyPassDialog::onDeleteClicked() {
    if (m_selectedPassId < 0) return;

    if (QMessageBox::question(this, QStringLiteral("Xác Nhận Xóa"),
                              QStringLiteral("Bạn có chắc chắn muốn xóa vé tháng này không?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (m_repo.deleteMonthlyPass(m_selectedPassId)) {
            QMessageBox::information(this, QStringLiteral("Thành Công"), QStringLiteral("Đã xóa vé tháng thành công!"));
            onClearForm();
            loadPasses();
        } else {
            QMessageBox::critical(this, QStringLiteral("Lỗi"), QStringLiteral("Không thể xóa vé tháng này!"));
        }
    }
}

void MonthlyPassDialog::onClearForm() {
    m_selectedPassId = -1;
    m_customerNameEdit->clear();
    m_licensePlateEdit->clear();
    m_vehicleTypeCombo->setCurrentIndex(1);
    m_startDateEdit->setDate(QDate::currentDate());
    m_expirationDateEdit->setDate(QDate::currentDate().addMonths(1));

    m_addBtn->setEnabled(true);
    m_updateBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);
}

void MonthlyPassDialog::onTableSelectionChanged() {
    auto selected = m_table->selectedItems();
    if (selected.isEmpty()) return;

    int row = selected.first()->row();
    m_selectedPassId = m_table->item(row, 0)->text().toInt();
    m_customerNameEdit->setText(m_table->item(row, 1)->text());
    m_licensePlateEdit->setText(m_table->item(row, 2)->text());

    QString typeStr = m_table->item(row, 3)->text();
    for (int i = 0; i < m_vehicleTypeCombo->count(); ++i) {
        if (m_vehicleTypeCombo->itemText(i) == typeStr) {
            m_vehicleTypeCombo->setCurrentIndex(i);
            break;
        }
    }

    m_startDateEdit->setDate(QDate::fromString(m_table->item(row, 4)->text(), "dd/MM/yyyy"));
    m_expirationDateEdit->setDate(QDate::fromString(m_table->item(row, 5)->text(), "dd/MM/yyyy"));

    m_addBtn->setEnabled(false);
    m_updateBtn->setEnabled(true);
    m_deleteBtn->setEnabled(true);
}

void MonthlyPassDialog::onSearchChanged(const QString& /*text*/) {
    loadPasses();
}
