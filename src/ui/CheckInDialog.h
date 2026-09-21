#ifndef CHECK_IN_DIALOG_H
#define CHECK_IN_DIALOG_H

#include <QDialog>
#include "../services/ParkingManager.h"

class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;

class CheckInDialog : public QDialog {
    Q_OBJECT

private:
    ParkingManager& m_manager;
    QLineEdit* m_plateEdit;
    QComboBox* m_typeCombo;
    QComboBox* m_ticketTypeCombo;
    QLabel* m_statusNoteLabel;
    QPushButton* m_confirmBtn;
    QPushButton* m_cancelBtn;

public:
    explicit CheckInDialog(ParkingManager& manager, QWidget* parent = nullptr);

private slots:
    void onConfirm();
    void onTicketTypeChanged(int index);
    void updateSlotAvailabilityHint();

private:
    void setupUi();
};

#endif // CHECK_IN_DIALOG_H
