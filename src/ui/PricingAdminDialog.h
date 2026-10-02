#ifndef PRICING_ADMIN_DIALOG_H
#define PRICING_ADMIN_DIALOG_H

#include <QDialog>
#include "../models/VehicleType.h"
#include "../models/PricingModel.h"
#include "../db/ParkingRepository.h"
#include <vector>

class QDoubleSpinBox;
class QPushButton;

class PricingAdminDialog : public QDialog {
    Q_OBJECT

private:
    ParkingRepository m_repo;
    struct RowInputs {
        VehicleType type;
        QString typeName;
        QDoubleSpinBox* firstBlockSpin;
        QDoubleSpinBox* nextBlockSpin;
    };
    std::vector<RowInputs> m_rows;

    QPushButton* m_saveBtn;
    QPushButton* m_cancelBtn;

public:
    explicit PricingAdminDialog(QWidget* parent = nullptr);

private slots:
    void loadConfigs();
    void onSave();

private:
    void setupUi();
};

#endif // PRICING_ADMIN_DIALOG_H
