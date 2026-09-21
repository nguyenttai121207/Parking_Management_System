#ifndef PRICING_ADMIN_DIALOG_H
#define PRICING_ADMIN_DIALOG_H

#include <QDialog>
#include "../services/ParkingManager.h"
#include <vector>

class QDoubleSpinBox;
class QPushButton;

class PricingAdminDialog : public QDialog {
    Q_OBJECT

private:
    ParkingManager& m_manager;
    struct RowInputs {
        VehicleType type;
        QDoubleSpinBox* hourlySpin;
        QDoubleSpinBox* monthlySpin;
    };
    std::vector<RowInputs> m_rows;

    QPushButton* m_saveBtn;
    QPushButton* m_cancelBtn;

public:
    explicit PricingAdminDialog(ParkingManager& manager, QWidget* parent = nullptr);

private slots:
    void loadConfigs();
    void onSave();

private:
    void setupUi();
};

#endif // PRICING_ADMIN_DIALOG_H
