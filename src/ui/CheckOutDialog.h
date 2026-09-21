#ifndef CHECK_OUT_DIALOG_H
#define CHECK_OUT_DIALOG_H

#include <QDialog>
#include "../services/ParkingManager.h"

class QLineEdit;
class QLabel;
class QPushButton;
class QFrame;

class CheckOutDialog : public QDialog {
    Q_OBJECT

private:
    ParkingManager& m_manager;
    CheckOutReceipt m_currentReceipt;

    QLineEdit* m_plateEdit;
    QPushButton* m_searchBtn;

    QFrame* m_receiptFrame;
    QLabel* m_plateVal;
    QLabel* m_vTypeVal;
    QLabel* m_slotVal;
    QLabel* m_inTimeVal;
    QLabel* m_outTimeVal;
    QLabel* m_durationVal;
    QLabel* m_pricingTypeVal;
    QLabel* m_totalFeeVal;

    QPushButton* m_confirmCheckoutBtn;
    QPushButton* m_closeBtn;

public:
    explicit CheckOutDialog(ParkingManager& manager, const QString& defaultPlate = "", QWidget* parent = nullptr);

private slots:
    void onSearchAndCalculate();
    void onConfirmCheckout();

private:
    void setupUi(const QString& defaultPlate);
    void resetReceiptView();
};

#endif // CHECK_OUT_DIALOG_H
