#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "../services/ParkingManager.h"

class QLabel;
class QPushButton;
class QTableWidget;
class QTabWidget;
class QGridLayout;
class QWidget;
class HistoryWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    ParkingManager m_manager;
    QTimer* m_clockTimer;

    QLabel* m_clockLabel;

    QLabel* m_totalSlotsVal;
    QLabel* m_availableSlotsVal;
    QLabel* m_activeVehiclesVal;
    QLabel* m_todayRevenueVal;

    QTabWidget* m_tabWidget;
    QTableWidget* m_activeTable;
    QWidget* m_slotsGridContainer;
    QGridLayout* m_slotsGridLayout;
    HistoryWidget* m_historyWidget;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

public slots:
    void refreshDashboard();

private slots:
    void updateClock();
    void openCheckInDialog();
    void openCheckOutDialog(const QString& plate = "");
    void openSubscriptionDialog();
    void openPricingAdminDialog();
    void switchToHistoryTab();
    void onActiveTableDoubleClicked(int row, int column);

private:
    void setupUi();
    QWidget* createHeaderWidget();
    QWidget* createMetricsWidget();
    QWidget* createActionToolbar();
    QWidget* createActiveVehiclesTab();
    QWidget* createSlotsVisualTab();
    void populateSlotsGrid();
};

#endif // MAIN_WINDOW_H
