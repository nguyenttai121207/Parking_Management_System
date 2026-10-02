#ifndef MONTHLY_PASS_DIALOG_H
#define MONTHLY_PASS_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QTableWidget>
#include <QPushButton>
#include "../db/ParkingRepository.h"

class MonthlyPassDialog : public QDialog {
    Q_OBJECT

private:
    ParkingRepository m_repo;
    int m_selectedPassId;

    QLineEdit* m_customerNameEdit;
    QLineEdit* m_licensePlateEdit;
    QComboBox* m_vehicleTypeCombo;
    QDateEdit* m_startDateEdit;
    QDateEdit* m_expirationDateEdit;
    QLineEdit* m_searchEdit;

    QTableWidget* m_table;
    QPushButton* m_addBtn;
    QPushButton* m_updateBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_clearBtn;

public:
    explicit MonthlyPassDialog(QWidget* parent = nullptr);

private slots:
    void loadPasses();
    void onAddClicked();
    void onUpdateClicked();
    void onDeleteClicked();
    void onClearForm();
    void onTableSelectionChanged();
    void onSearchChanged(const QString& text);

private:
    void setupUi();
};

#endif // MONTHLY_PASS_DIALOG_H
