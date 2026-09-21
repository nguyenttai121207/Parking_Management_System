#include "HistoryWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>

HistoryWidget::HistoryWidget(ParkingManager& manager, QWidget* parent)
    : QWidget(parent), m_manager(manager) {
    auto mainLayout = new QVBoxLayout(this);

    auto topLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(QStringLiteral("Nhập biển số xe để tìm kiếm lịch sử..."));
    m_searchBtn = new QPushButton(QStringLiteral("Tìm kiếm"), this);
    m_searchBtn->setObjectName("primaryBtn");
    m_resetBtn = new QPushButton(QStringLiteral("Xem tất cả"), this);

    topLayout->addWidget(new QLabel(QStringLiteral("Biển số:"), this));
    topLayout->addWidget(m_searchEdit);
    topLayout->addWidget(m_searchBtn);
    topLayout->addWidget(m_resetBtn);
    mainLayout->addLayout(topLayout);

    m_historyTable = new QTableWidget(this);
    m_historyTable->setColumnCount(9);
    m_historyTable->setHorizontalHeaderLabels({
        QStringLiteral("Mã Vé"), QStringLiteral("Biển Số"), QStringLiteral("Loại Phương Tiện"),
        QStringLiteral("Vị Trí"), QStringLiteral("Giờ Vào"), QStringLiteral("Giờ Ra"),
        QStringLiteral("Hình Thức"), QStringLiteral("Số Tiền (VNĐ)"), QStringLiteral("Trạng Thái")
    });
    m_historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(m_historyTable);

    connect(m_searchBtn, &QPushButton::clicked, this, &HistoryWidget::onSearch);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &HistoryWidget::onSearch);
    connect(m_resetBtn, &QPushButton::clicked, this, &HistoryWidget::onReset);

    loadHistory();
}

void HistoryWidget::loadHistory(const QString& filter) {
    auto list = m_manager.searchHistory(filter);
    m_historyTable->setRowCount(static_cast<int>(list.size()));

    for (size_t i = 0; i < list.size(); ++i) {
        const auto& t = list[i];
        m_historyTable->setItem(i, 0, new QTableWidgetItem(QString::number(t.getTicketId())));
        m_historyTable->setItem(i, 1, new QTableWidgetItem(t.getLicensePlate()));
        m_historyTable->setItem(i, 2, new QTableWidgetItem(VehicleUtils::getVehicleTypeName(t.getVehicleType())));
        m_historyTable->setItem(i, 3, new QTableWidgetItem(t.getSlotNumber()));
        m_historyTable->setItem(i, 4, new QTableWidgetItem(t.getCheckInTime().toString("dd/MM/yyyy HH:mm:ss")));
        
        QString outStr = t.getCheckOutTime().isValid() ? t.getCheckOutTime().toString("dd/MM/yyyy HH:mm:ss") : "--";
        m_historyTable->setItem(i, 5, new QTableWidgetItem(outStr));

        QString pricingStr = (t.getPricingType() == "MONTHLY_PASS") ? QStringLiteral("Vé tháng") : QStringLiteral("Vé theo giờ");
        m_historyTable->setItem(i, 6, new QTableWidgetItem(pricingStr));

        m_historyTable->setItem(i, 7, new QTableWidgetItem(QString::number(t.getTotalFee(), 'f', 0)));

        QString statusStr = t.isActive() ? QStringLiteral("Đang đỗ") : QStringLiteral("Đã ra");
        auto statusItem = new QTableWidgetItem(statusStr);
        if (t.isActive()) {
            statusItem->setForeground(QBrush(QColor("#a6e3a1")));
        } else {
            statusItem->setForeground(QBrush(QColor("#bac2de")));
        }
        m_historyTable->setItem(i, 8, statusItem);
    }
}

void HistoryWidget::onSearch() {
    loadHistory(m_searchEdit->text().trimmed());
}

void HistoryWidget::onReset() {
    m_searchEdit->clear();
    loadHistory();
}
