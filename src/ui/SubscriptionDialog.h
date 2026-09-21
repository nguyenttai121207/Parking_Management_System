#ifndef SUBSCRIPTION_DIALOG_H
#define SUBSCRIPTION_DIALOG_H

#include <QDialog>
#include "../services/ParkingManager.h"

class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;
class QTableWidget;

class SubscriptionDialog : public QDialog {
    Q_OBJECT

private:
    ParkingManager& m_manager;

    QLineEdit* m_plateEdit;
    QComboBox* m_typeCombo;
    QLineEdit* m_nameEdit;
    QLineEdit* m_phoneEdit;
    QComboBox* m_monthsCombo;
    QLabel* m_pricePreviewLabel;
    QLabel* m_expiryPreviewLabel;

    QPushButton* m_registerBtn;
    QPushButton* m_closeBtn;
    QTableWidget* m_subsTable;

public:
    explicit SubscriptionDialog(ParkingManager& manager, QWidget* parent = nullptr);

private slots:
    void updatePricePreview();
    void onRegister();
    void loadSubscriptionTable();

private:
    void setupUi();
};

#endif // SUBSCRIPTION_DIALOG_H
