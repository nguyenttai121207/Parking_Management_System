#include "MainWindow.h"
#include "KioskGateDialog.h"
#include "SelfCheckoutDialog.h"
#include "MonthlyPassDialog.h"
#include "PricingAdminDialog.h"
#include "MaintenanceLogDialog.h"
#include "LoginDialog.h"
#include "HistoryWidget.h"
#include "../models/VehicleType.h"
#include "../services/SessionManager.h"

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
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_hardware(std::make_shared<HardwareController>("COM3", this)),
      m_camera(std::make_shared<CameraService>("rtsp://192.168.1.100:554/live", this)),
      m_clockTimer(new QTimer(this)) {
    setWindowTitle(QStringLiteral("Hệ Thống Quản Lý Bãi Đỗ Xe Tự Động (Smart Parking)"));
    resize(1240, 780);

    m_autoGate = std::make_shared<AutoGateManager>(m_camera, m_hardware, this);

    setupUi();
    applyRbacPermissions();
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
    m_tabWidget->addTab(createSlotsVisualTab(), QStringLiteral("Sơ Đồ Vị Trí Ô Đỗ"));

    m_historyWidget = new HistoryWidget(m_manager, this);
    m_tabWidget->addTab(m_historyWidget, QStringLiteral("Lịch Sử Ra Vào"));

    mainLayout->addWidget(m_tabWidget);
}

QWidget* MainWindow::createHeaderWidget() {
    auto headerWidget = new QWidget(this);
    auto layout = new QHBoxLayout(headerWidget);
    layout->setContentsMargins(0, 0, 0, 0);

    auto titleLayout = new QVBoxLayout();
    auto title = new QLabel(QStringLiteral("HỆ THỐNG QUẢN LÝ BÃI ĐỖ XE TỰ ĐỘNG"), headerWidget);
    title->setObjectName("titleLabel");
    title->setStyleSheet("font-size: 20px; font-weight: 800; color: #89b4fa; letter-spacing: 0.04em;");

    auto subtitle = new QLabel(QStringLiteral("Kiosk Camera ANPR  •  Self-Checkout VietQR  •  Vé Tháng Tự Động  •  Phân Quyền RBAC"), headerWidget);
    subtitle->setStyleSheet("color: #a6adc8; font-size: 13px;");

    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);
    layout->addLayout(titleLayout);

    layout->addStretch();

    // Huy hiệu người dùng đăng nhập & Role (Prompt 4)
    User curUser = SessionManager::instance().getCurrentUser();
    m_userRoleBadge = new QLabel(headerWidget);
    QString roleColor = curUser.isAdmin() ? "#89b4fa" : "#fab387";
    m_userRoleBadge->setStyleSheet(QString(
        "background-color: #252538; border: 1px solid %1; border-radius: 6px; padding: 6px 14px; "
        "color: %1; font-weight: bold; font-size: 12px;"
    ).arg(roleColor));
    m_userRoleBadge->setText(QStringLiteral("👤 %1 [%2]").arg(curUser.getUsername(), curUser.getRole()));
    layout->addWidget(m_userRoleBadge);

    auto logoutBtn = new QPushButton(QStringLiteral("Đăng Xuất"), headerWidget);
    logoutBtn->setCursor(Qt::PointingHandCursor);
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogout);
    layout->addWidget(logoutBtn);

    // Đồng hồ thời gian thực
    auto clockFrame = new QFrame(headerWidget);
    clockFrame->setStyleSheet("background-color: #252538; border: 1px solid #313244; border-radius: 6px; padding: 6px 14px;");
    auto clockLayout = new QVBoxLayout(clockFrame);
    clockLayout->setContentsMargins(4, 4, 4, 4);

    m_clockLabel = new QLabel(clockFrame);
    m_clockLabel->setStyleSheet("font-family: 'Consolas', monospace; font-size: 13px; font-weight: bold; color: #a6e3a1;");
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

    auto makeCard = [](const QString& title, QLabel*& valLabel, const QString& valColor) -> QFrame* {
        auto card = new QFrame();
        card->setStyleSheet("background-color: #252538; border: 1px solid #313244; border-radius: 8px; padding: 12px;");
        auto cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(10, 10, 10, 10);
        cardLayout->setSpacing(6);

        auto lblTitle = new QLabel(title, card);
        lblTitle->setStyleSheet("font-size: 11px; font-weight: 600; color: #a6adc8; text-transform: uppercase;");

        valLabel = new QLabel("0", card);
        valLabel->setStyleSheet(QString("font-size: 22px; font-weight: bold; color: %1;").arg(valColor));

        cardLayout->addWidget(lblTitle);
        cardLayout->addWidget(valLabel);
        return card;
    };

    layout->addWidget(makeCard(QStringLiteral("Tổng Số Ô Đỗ"), m_totalSlotsVal, "#cdd6f4"));
    layout->addWidget(makeCard(QStringLiteral("Chỗ Còn Trống"), m_availableSlotsVal, "#a6e3a1"));
    layout->addWidget(makeCard(QStringLiteral("Xe Đang Gửi"), m_activeVehiclesVal, "#89b4fa"));

    // Thẻ doanh thu: sẽ bị ẩn hoàn toàn nếu là Maintenance (Prompt 4)
    m_revenueCard = makeCard(QStringLiteral("Doanh Thu Hôm Nay"), m_todayRevenueVal, "#f9e2af");
    layout->addWidget(m_revenueCard);

    return metricsWidget;
}

QWidget* MainWindow::createActionToolbar() {
    auto bar = new QWidget(this);
    auto layout = new QHBoxLayout(bar);
    layout->setContentsMargins(0, 2, 0, 2);
    layout->setSpacing(10);

    // Prompt 1: Nút mở Kiosk Mode & Camera Check-in
    m_kioskBtn = new QPushButton(QStringLiteral("📹 Kiosk Check-in Camera"), bar);
    m_kioskBtn->setObjectName("successBtn");
    m_kioskBtn->setStyleSheet("font-weight: bold; padding: 8px 14px;");
    m_kioskBtn->setCursor(Qt::PointingHandCursor);

    // Prompt 2: Nút mở Self-Checkout
    m_selfCheckoutBtn = new QPushButton(QStringLiteral("💳 Self-Checkout VietQR"), bar);
    m_selfCheckoutBtn->setObjectName("primaryBtn");
    m_selfCheckoutBtn->setStyleSheet("font-weight: bold; padding: 8px 14px;");
    m_selfCheckoutBtn->setCursor(Qt::PointingHandCursor);

    // Prompt 3: Quản lý vé tháng
    m_subBtn = new QPushButton(QStringLiteral("🎫 Quản Lý Vé Tháng"), bar);
    m_subBtn->setCursor(Qt::PointingHandCursor);

    // Prompt 5: Cấu hình bảng giá (Chỉ Admin)
    m_priceBtn = new QPushButton(QStringLiteral("⚙️ Biểu Phí"), bar);
    m_priceBtn->setCursor(Qt::PointingHandCursor);

    // Prompt 4: Tính năng cho Maintenance
    m_emergencyBarrierBtn = new QPushButton(QStringLiteral("🚨 Mở Cổng Khẩn Cấp"), bar);
    m_emergencyBarrierBtn->setObjectName("dangerBtn");
    m_emergencyBarrierBtn->setCursor(Qt::PointingHandCursor);

    m_maintenanceLogBtn = new QPushButton(QStringLiteral("🛠️ Trạng Thái & Log Phần Cứng"), bar);
    m_maintenanceLogBtn->setCursor(Qt::PointingHandCursor);

    auto refreshBtn = new QPushButton(QStringLiteral("🔄 Làm Mới"), bar);
    refreshBtn->setCursor(Qt::PointingHandCursor);

    layout->addWidget(m_kioskBtn);
    layout->addWidget(m_selfCheckoutBtn);
    layout->addWidget(m_subBtn);
    layout->addWidget(m_priceBtn);
    layout->addWidget(m_emergencyBarrierBtn);
    layout->addWidget(m_maintenanceLogBtn);
    layout->addStretch();
    layout->addWidget(refreshBtn);

    connect(m_kioskBtn, &QPushButton::clicked, this, &MainWindow::openKioskGate);
    connect(m_selfCheckoutBtn, &QPushButton::clicked, [this]() { openSelfCheckout(); });
    connect(m_subBtn, &QPushButton::clicked, this, &MainWindow::openMonthlyPassDialog);
    connect(m_priceBtn, &QPushButton::clicked, this, &MainWindow::openPricingAdminDialog);
    connect(m_emergencyBarrierBtn, &QPushButton::clicked, this, &MainWindow::onEmergencyOpen);
    connect(m_maintenanceLogBtn, &QPushButton::clicked, this, &MainWindow::openMaintenanceLog);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshDashboard);

    return bar;
}

void MainWindow::applyRbacPermissions() {
    bool isAdmin = SessionManager::instance().isAdmin();
    bool isMaintenance = SessionManager::instance().isMaintenance();

    if (isMaintenance) {
        // Prompt 4: Nếu là 'Maintenance': Chỉ hiển thị tính năng 'Mở cổng khẩn cấp',
        // 'Xem Log hệ thống' và 'Trạng thái kết nối Camera/Phần cứng', ẩn các tab tài chính.
        m_revenueCard->hide();
        m_priceBtn->hide();
        m_subBtn->hide();

        m_emergencyBarrierBtn->show();
        m_maintenanceLogBtn->show();
        m_kioskBtn->show();
        m_selfCheckoutBtn->hide();
    } else {
        // Prompt 4: Nếu là 'Admin': Hiển thị đầy đủ menu
        // (Báo cáo doanh thu, Cấu hình giá, Quản lý tài khoản, Quản lý vé tháng).
        m_revenueCard->show();
        m_priceBtn->show();
        m_subBtn->show();
        m_kioskBtn->show();
        m_selfCheckoutBtn->show();
        m_emergencyBarrierBtn->hide();
        m_maintenanceLogBtn->show();
    }
}

QWidget* MainWindow::createActiveVehiclesTab() {
    auto tabWidget = new QWidget(this);
    auto layout = new QVBoxLayout(tabWidget);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    auto tipLabel = new QLabel(QStringLiteral("Nhấp đúp chuột vào phương tiện bất kỳ để mở nhanh màn hình Self-Checkout."), tabWidget);
    tipLabel->setStyleSheet("color: #89b4fa; font-size: 13px; font-style: italic;");
    layout->addWidget(tipLabel);

    m_activeTable = new QTableWidget(this);
    m_activeTable->setColumnCount(6);
    m_activeTable->setHorizontalHeaderLabels({
        QStringLiteral("Mã Phiên"), QStringLiteral("Biển Số Xe"), QStringLiteral("Loại Phương Tiện"),
        QStringLiteral("Mã Ô Đỗ"), QStringLiteral("Thời Điểm Vào"), QStringLiteral("Trạng Thái")
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

    auto greenDot = new QLabel(QStringLiteral("<span style='color: #a6e3a1; font-size: 16px;'>●</span> Chỗ Còn Trống"), tabWidget);
    auto redDot = new QLabel(QStringLiteral("<span style='color: #f38ba8; font-size: 16px;'>●</span> Đang Có Xe Đỗ"), tabWidget);
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

    auto slotList = m_manager.getAllSlots();
    const int columns = 5;

    for (size_t i = 0; i < slotList.size(); ++i) {
        const auto& s = slotList[i];
        int row = static_cast<int>(i) / columns;
        int col = static_cast<int>(i) % columns;

        auto card = new QFrame(m_slotsGridContainer);
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

    // Nạp danh sách xe đang gửi từ bảng ParkingSessions
    ParkingRepository repo;
    auto sessions = repo.getAllActiveSessions();
    m_activeTable->setRowCount(static_cast<int>(sessions.size()));

    for (size_t i = 0; i < sessions.size(); ++i) {
        const auto& s = sessions[i];
        m_activeTable->setItem(i, 0, new QTableWidgetItem(QString::number(s.getId())));
        m_activeTable->setItem(i, 1, new QTableWidgetItem(s.getLicensePlate()));
        m_activeTable->setItem(i, 2, new QTableWidgetItem(VehicleUtils::getVehicleTypeName(s.getVehicleType())));
        m_activeTable->setItem(i, 3, new QTableWidgetItem(QStringLiteral("Vào tự do")));
        m_activeTable->setItem(i, 4, new QTableWidgetItem(s.getCheckInTime().toString("HH:mm:ss - dd/MM/yyyy")));

        auto statusItem = new QTableWidgetItem(s.getStatus());
        statusItem->setForeground(QBrush(QColor(0xA6, 0xE3, 0xA1)));
        m_activeTable->setItem(i, 5, statusItem);
    }

    populateSlotsGrid();

    if (m_historyWidget) {
        m_historyWidget->loadHistory();
    }
}

void MainWindow::openKioskGate() {
    KioskGateDialog dialog(m_autoGate, this);
    dialog.exec();
    refreshDashboard();
}

void MainWindow::openSelfCheckout(const QString& plate) {
    SelfCheckoutDialog dialog(m_hardware, this);
    if (!plate.isEmpty()) {
        // Tìm nhanh qua plate
    }
    dialog.exec();
    refreshDashboard();
}

void MainWindow::openMonthlyPassDialog() {
    MonthlyPassDialog dialog(this);
    dialog.exec();
    refreshDashboard();
}

void MainWindow::openPricingAdminDialog() {
    PricingAdminDialog dialog(this);
    dialog.exec();
}

void MainWindow::openMaintenanceLog() {
    MaintenanceLogDialog dialog(m_hardware, m_camera, this);
    dialog.exec();
}

void MainWindow::onEmergencyOpen() {
    if (m_hardware) {
        m_hardware->emergencyOpen();
        QMessageBox::information(this, QStringLiteral("Khẩn Cấp"),
                                 QStringLiteral("Đã phát tín hiệu mở Barrier khẩn cấp!"));
    }
}

void MainWindow::switchToHistoryTab() {
    m_tabWidget->setCurrentIndex(2);
}

void MainWindow::onActiveTableDoubleClicked(int row, int /*column*/) {
    auto plateItem = m_activeTable->item(row, 1);
    if (plateItem) {
        openSelfCheckout(plateItem->text());
    }
}

void MainWindow::onLogout() {
    SessionManager::instance().logout();
    hide();

    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        applyRbacPermissions();
        User curUser = SessionManager::instance().getCurrentUser();
        QString roleColor = curUser.isAdmin() ? "#89b4fa" : "#fab387";
        m_userRoleBadge->setStyleSheet(QString(
            "background-color: #252538; border: 1px solid %1; border-radius: 6px; padding: 6px 14px; "
            "color: %1; font-weight: bold; font-size: 12px;"
        ).arg(roleColor));
        m_userRoleBadge->setText(QStringLiteral("👤 %1 [%2]").arg(curUser.getUsername(), curUser.getRole()));
        show();
        refreshDashboard();
    } else {
        close();
    }
}