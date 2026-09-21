#include "MainWindow.h"
#include "CheckInDialog.h"
#include "CheckOutDialog.h"
#include "SubscriptionDialog.h"
#include "PricingAdminDialog.h"
#include "HistoryWidget.h"
#include "../models/VehicleType.h" // Khai báo cho VehicleUtils

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QTabWidget>
#include <QFrame>
#include <QScrollArea>
#include <QDateTime>
#include <QTableWidgetItem>
#include <QBrush>
#include <QColor>
#include <QLayoutItem>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_clockTimer(new QTimer(this)) {
    setWindowTitle(QStringLiteral("Hệ Thống Quản Lý Bãi Đỗ Xe Thông Minh"));
    resize(1200, 760);

    setupUi();
    refreshDashboard();

    connect(m_clockTimer, &QTimer::timeout, this, &MainWindow::updateClock);
    m_clockTimer->start(1000);
    updateClock();
}

void MainWindow::setupUi() {
    auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    mainLayout->addWidget(createHeaderWidget());
    mainLayout->addWidget(createMetricsWidget());
    mainLayout->addWidget(createActionToolbar());

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->addTab(createActiveVehiclesTab(), QStringLiteral("Phương Tiện Đang Đỗ"));
    m_tabWidget->addTab(createSlotsVisualTab(), QStringLiteral("Sơ Đồ Lưới Vị Trí Ô Đỗ"));

    m_historyWidget = new HistoryWidget(m_manager, this);
    m_tabWidget->addTab(m_historyWidget, QStringLiteral("Lịch Sử Ra Vào"));

    mainLayout->addWidget(m_tabWidget);
}

QWidget* MainWindow::createHeaderWidget() {
    auto headerWidget = new QWidget(this);
    auto layout = new QHBoxLayout(headerWidget);
    layout->setContentsMargins(0, 0, 0, 0);

    auto titleLayout = new QVBoxLayout();
    auto title = new QLabel(QStringLiteral("HỆ THỐNG QUẢN LÝ BÃI ĐỖ XE"), headerWidget);
    title->setObjectName("titleLabel");
    title->setStyleSheet("font-size: 20px; font-weight: 800; color: #89b4fa; letter-spacing: 0.04em;");

    auto subtitle = new QLabel(QStringLiteral("Giải pháp điều hành bãi giữ xe đa phương tiện | Hỗ trợ Vé lượt & Vé tháng"), headerWidget);
    subtitle->setStyleSheet("color: #a6adc8; font-size: 13px;");

    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);
    layout->addLayout(titleLayout);

    layout->addStretch();

    auto clockFrame = new QFrame(headerWidget);
    clockFrame->setStyleSheet("background-color: #252538; border: 1px solid #313244; border-radius: 6px; padding: 6px 14px;");
    auto clockLayout = new QVBoxLayout(clockFrame);
    clockLayout->setContentsMargins(4, 4, 4, 4);

    m_clockLabel = new QLabel(clockFrame);
    m_clockLabel->setStyleSheet("font-family: 'Consolas', 'Segoe UI Mono', monospace; font-size: 14px; font-weight: bold; color: #a6e3a1;");
    m_clockLabel->setAlignment(Qt::AlignCenter);
    clockLayout->addWidget(m_clockLabel);

    layout->addWidget(clockFrame);

    return headerWidget;
}

QWidget* MainWindow::createMetricsWidget() {
    auto metricsWidget = new QWidget(this);
    auto layout = new QHBoxLayout(metricsWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto makeCard = [this, metricsWidget](const QString& title, QLabel*& valLabel, const QString& valColor) -> QFrame* {
        auto card = new QFrame(metricsWidget);
        card->setStyleSheet("background-color: #252538; border: 1px solid #313244; border-radius: 8px; padding: 12px;");
        auto cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(10, 10, 10, 10);
        cardLayout->setSpacing(6);

        auto lblTitle = new QLabel(title, card);
        lblTitle->setStyleSheet("font-size: 12px; font-weight: 600; color: #a6adc8; text-transform: uppercase; letter-spacing: 0.05em;");

        valLabel = new QLabel("0", card);
        valLabel->setStyleSheet(QString("font-size: 24px; font-weight: bold; color: %1;").arg(valColor));

        cardLayout->addWidget(lblTitle);
        cardLayout->addWidget(valLabel);
        return card;
    };

    layout->addWidget(makeCard(QStringLiteral("Tổng Số Ô Đỗ"), m_totalSlotsVal, "#cdd6f4"));
    layout->addWidget(makeCard(QStringLiteral("Chỗ Còn Trống"), m_availableSlotsVal, "#a6e3a1"));
    layout->addWidget(makeCard(QStringLiteral("Xe Đang Gửi"), m_activeVehiclesVal, "#89b4fa"));
    layout->addWidget(makeCard(QStringLiteral("Doanh Thu Hôm Nay"), m_todayRevenueVal, "#f9e2af"));

    return metricsWidget;
}

QWidget* MainWindow::createActionToolbar() {
    auto bar = new QWidget(this);
    auto layout = new QHBoxLayout(bar);
    layout->setContentsMargins(0, 2, 0, 2);
    layout->setSpacing(10);

    auto checkInBtn = new QPushButton(QStringLiteral("Check-in Xe Vào"), bar);
    checkInBtn->setObjectName("successBtn");
    checkInBtn->setCursor(Qt::PointingHandCursor);

    auto checkOutBtn = new QPushButton(QStringLiteral("Check-out Xe Ra"), bar);
    checkOutBtn->setObjectName("primaryBtn");
    checkOutBtn->setCursor(Qt::PointingHandCursor);

    auto subBtn = new QPushButton(QStringLiteral("Đăng Ký Vé Tháng"), bar);
    subBtn->setCursor(Qt::PointingHandCursor);

    auto historyBtn = new QPushButton(QStringLiteral("Lịch Sử Ra Vào"), bar);
    historyBtn->setCursor(Qt::PointingHandCursor);

    auto priceBtn = new QPushButton(QStringLiteral("Quản Lý Bảng Giá"), bar);
    priceBtn->setCursor(Qt::PointingHandCursor);

    auto refreshBtn = new QPushButton(QStringLiteral("Làm Mới"), bar);
    refreshBtn->setCursor(Qt::PointingHandCursor);

    layout->addWidget(checkInBtn);
    layout->addWidget(checkOutBtn);
    layout->addWidget(subBtn);
    layout->addWidget(historyBtn);
    layout->addWidget(priceBtn);
    layout->addStretch();
    layout->addWidget(refreshBtn);

    connect(checkInBtn, &QPushButton::clicked, this, &MainWindow::openCheckInDialog);
    connect(checkOutBtn, &QPushButton::clicked, [this]() { openCheckOutDialog(); });
    connect(subBtn, &QPushButton::clicked, this, &MainWindow::openSubscriptionDialog);
    connect(historyBtn, &QPushButton::clicked, this, &MainWindow::switchToHistoryTab);
    connect(priceBtn, &QPushButton::clicked, this, &MainWindow::openPricingAdminDialog);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshDashboard);

    return bar;
}

QWidget* MainWindow::createActiveVehiclesTab() {
    auto tabWidget = new QWidget(this);
    auto layout = new QVBoxLayout(tabWidget);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    auto tipLabel = new QLabel(QStringLiteral("Nhấp đúp chuột vào bất kỳ dòng nào để mở nhanh màn hình Check-out cho phương tiện đó."), tabWidget);
    tipLabel->setStyleSheet("color: #89b4fa; font-size: 13px; font-style: italic;");
    layout->addWidget(tipLabel);

    m_activeTable = new QTableWidget(this);
    m_activeTable->setColumnCount(6);
    m_activeTable->setHorizontalHeaderLabels({
        QStringLiteral("Mã Vé"), QStringLiteral("Biển Số Xe"), QStringLiteral("Loại Phương Tiện"),
        QStringLiteral("Mã Slot"), QStringLiteral("Thời Điểm Vào"), QStringLiteral("Trạng Thái")
    });
    m_activeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_activeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_activeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(m_activeTable);

    connect(m_activeTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::onActiveTableDoubleClicked);

    return tabWidget;
}

QWidget* MainWindow::createSlotsVisualTab() {
    auto tabWidget = new QWidget(this);
    auto tabLayout = new QVBoxLayout(tabWidget);
    tabLayout->setContentsMargins(10, 10, 10, 10);
    tabLayout->setSpacing(8);

    auto legendLayout = new QHBoxLayout();
    legendLayout->setSpacing(16);

    auto greenDot = new QLabel(QStringLiteral("<span style='color: #a6e3a1; font-size: 16px;'>●</span> Chỗ Còn Trống (Nhấp để Check-in)"), tabWidget);
    auto redDot = new QLabel(QStringLiteral("<span style='color: #f38ba8; font-size: 16px;'>●</span> Đã Có Xe Đỗ (Nhấp để Check-out)"), tabWidget);
    legendLayout->addWidget(greenDot);
    legendLayout->addWidget(redDot);
    legendLayout->addStretch();
    tabLayout->addLayout(legendLayout);

    auto scrollArea = new QScrollArea(tabWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("border: 1px solid #313244; background-color: #181825; border-radius: 6px;");

    m_slotsGridContainer = new QWidget();
    m_slotsGridContainer->setStyleSheet("background-color: #181825;");
    m_slotsGridLayout = new QGridLayout(m_slotsGridContainer);
    m_slotsGridLayout->setSpacing(12);
    m_slotsGridLayout->setContentsMargins(14, 14, 14, 14);

    scrollArea->setWidget(m_slotsGridContainer);
    tabLayout->addWidget(scrollArea);

    return tabWidget;
}

void MainWindow::populateSlotsGrid() {
    QLayoutItem* item;
    while ((item = m_slotsGridLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    // Đã đổi tên biến 'slots' thành 'slotList' để tránh xung đột macro Qt
    auto slotList = m_manager.getAllSlots();
    const int columns = 5;

    for (size_t i = 0; i < slotList.size(); ++i) {
        const auto& s = slotList[i];
        int row = static_cast<int>(i) / columns;
        int col = static_cast<int>(i) % columns;

        auto card = new QFrame(m_slotsGridContainer);
        card->setCursor(Qt::PointingHandCursor);

        QString borderColor = s.isOccupied() ? "#f38ba8" : "#a6e3a1";
        QString bgColor = s.isOccupied() ? "#2d1f2d" : "#1f2d24";
        card->setStyleSheet(QString(
                                "QFrame { background-color: %1; border: 1px solid %2; border-radius: 6px; padding: 8px; }"
                                "QFrame:hover { border: 2px solid %2; }"
                                ).arg(bgColor, borderColor));

        auto cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(8, 8, 8, 8);
        cardLayout->setSpacing(4);

        auto slotNumLabel = new QLabel(s.getSlotNumber(), card);
        slotNumLabel->setStyleSheet(QString("font-weight: bold; font-size: 15px; color: %1;").arg(borderColor));
        cardLayout->addWidget(slotNumLabel);

        auto typeLabel = new QLabel(VehicleUtils::getSlotTypeName(s.getSlotType()), card);
        typeLabel->setStyleSheet("font-size: 11px; color: #a6adc8;");
        cardLayout->addWidget(typeLabel);

        QString statusText = s.isOccupied() ? QStringLiteral("Xe: %1").arg(s.getCurrentLicensePlate()) : QStringLiteral("Đang trống");
        auto statusLabel = new QLabel(statusText, card);
        statusLabel->setStyleSheet(QString("font-size: 12px; font-weight: 600; color: %1;").arg(s.isOccupied() ? "#cdd6f4" : "#a6e3a1"));
        cardLayout->addWidget(statusLabel);

        QString plate = s.getCurrentLicensePlate();
        bool isOccupied = s.isOccupied();

        auto btnAction = new QPushButton(s.isOccupied() ? QStringLiteral("Xuất bãi") : QStringLiteral("Check-in"), card);
        btnAction->setStyleSheet("font-size: 11px; padding: 3px 8px; margin-top: 4px;");
        connect(btnAction, &QPushButton::clicked, [this, isOccupied, plate]() {
            if (isOccupied) {
                openCheckOutDialog(plate);
            } else {
                openCheckInDialog();
            }
        });
        cardLayout->addWidget(btnAction);

        m_slotsGridLayout->addWidget(card, row, col);
    }
}

void MainWindow::updateClock() {
    m_clockLabel->setText(QDateTime::currentDateTime().toString("HH:mm:ss  |  dd/MM/yyyy"));
}

void MainWindow::refreshDashboard() {
    DashboardStats stats = m_manager.getDashboardStats();
    int totalSlots = stats.totalMotorbikeSlots + stats.totalCarSlots;
    int occupiedSlots = stats.occupiedMotorbikeSlots + stats.occupiedCarSlots;
    int availableSlots = totalSlots - occupiedSlots;

    m_totalSlotsVal->setText(QString::number(totalSlots));
    m_availableSlotsVal->setText(QString::number(availableSlots));
    m_activeVehiclesVal->setText(QString::number(stats.activeVehiclesCount));
    m_todayRevenueVal->setText(QStringLiteral("%1 VNĐ").arg(QString::number(stats.todayRevenue, 'f', 0)));

    auto activeList = m_manager.getAllActiveTickets();
    m_activeTable->setRowCount(static_cast<int>(activeList.size()));
    for (size_t i = 0; i < activeList.size(); ++i) {
        const auto& t = activeList[i];
        m_activeTable->setItem(i, 0, new QTableWidgetItem(QString::number(t.getTicketId())));
        m_activeTable->setItem(i, 1, new QTableWidgetItem(t.getLicensePlate()));
        m_activeTable->setItem(i, 2, new QTableWidgetItem(VehicleUtils::getVehicleTypeName(t.getVehicleType())));
        m_activeTable->setItem(i, 3, new QTableWidgetItem(t.getSlotNumber()));
        m_activeTable->setItem(i, 4, new QTableWidgetItem(t.getCheckInTime().toString("HH:mm:ss - dd/MM/yyyy")));

        auto statusItem = new QTableWidgetItem(QStringLiteral("Đang đỗ"));
        statusItem->setForeground(QBrush(QColor(0xA6, 0xE3, 0xA1)));
        m_activeTable->setItem(i, 5, statusItem);
    }

    populateSlotsGrid();

    if (m_historyWidget) {
        m_historyWidget->loadHistory();
    }
}

void MainWindow::openCheckInDialog() {
    CheckInDialog dialog(m_manager, this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshDashboard();
    }
}

void MainWindow::openCheckOutDialog(const QString& plate) {
    CheckOutDialog dialog(m_manager, plate, this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshDashboard();
    }
}

void MainWindow::openSubscriptionDialog() {
    SubscriptionDialog dialog(m_manager, this);
    dialog.exec();
    refreshDashboard();
}

void MainWindow::openPricingAdminDialog() {
    PricingAdminDialog dialog(m_manager, this);
    dialog.exec();
}

void MainWindow::switchToHistoryTab() {
    m_tabWidget->setCurrentIndex(2);
}

void MainWindow::onActiveTableDoubleClicked(int row, int /*column*/) {
    auto plateItem = m_activeTable->item(row, 1);
    if (plateItem) {
        QString plate = plateItem->text();
        openCheckOutDialog(plate);
    }
}