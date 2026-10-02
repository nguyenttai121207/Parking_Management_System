#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <memory>
#include "../services/ParkingManager.h"
#include "../services/HardwareController.h"
#include "../services/CameraService.h"
#include "../services/AutoGateManager.h"

class QLabel;
class QPushButton;
class QTableWidget;
class QTabWidget;
class QGridLayout;
class QWidget;
class QFrame;
class HistoryWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    ParkingManager m_manager;
    std::shared_ptr<HardwareController> m_hardware;
    std::shared_ptr<CameraService> m_camera;
    std::shared_ptr<AutoGateManager> m_autoGate;

    QTimer* m_clockTimer;
    QLabel* m_clockLabel;
    QLabel* m_userRoleBadge;

    QLabel* m_totalSlotsVal;
    QLabel* m_availableSlotsVal;
    QLabel* m_activeVehiclesVal;
    QLabel* m_todayRevenueVal;
    QFrame* m_revenueCard;

    QTabWidget* m_tabWidget;
    QTableWidget* m_activeTable;
    QWidget* m_slotsGridContainer;
    QGridLayout* m_slotsGridLayout;
    HistoryWidget* m_historyWidget;

    // Action buttons subject to RBAC
    QPushButton* m_kioskBtn;
    QPushButton* m_selfCheckoutBtn;
    QPushButton* m_subBtn;
    QPushButton* m_priceBtn;
    QPushButton* m_emergencyBarrierBtn;
    QPushButton* m_maintenanceLogBtn;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

public slots:
    void refreshDashboard();

private slots:
    void updateClock();
    void openKioskGate();
    void openSelfCheckout(const QString& plate = "");
    void openMonthlyPassDialog();
    void openPricingAdminDialog();
    void openMaintenanceLog();
    void onEmergencyOpen();
    void switchToHistoryTab();
    void onActiveTableDoubleClicked(int row, int column);
    void onLogout();

private:
    void setupUi();
    void applyRbacPermissions();
    QWidget* createHeaderWidget();
    QWidget* createMetricsWidget();
    QWidget* createActionToolbar();
    QWidget* createActiveVehiclesTab();
    QWidget* createSlotsVisualTab();
    void populateSlotsGrid();
};

#endif // MAIN_WINDOW_H
