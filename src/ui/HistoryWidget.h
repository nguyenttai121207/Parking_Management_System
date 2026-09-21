#ifndef HISTORY_WIDGET_H
#define HISTORY_WIDGET_H

#include <QWidget>
#include "../services/ParkingManager.h"

class QLineEdit;
class QPushButton;
class QTableWidget;

class HistoryWidget : public QWidget {
    Q_OBJECT

private:
    ParkingManager& m_manager;

    QLineEdit* m_searchEdit;
    QPushButton* m_searchBtn;
    QPushButton* m_resetBtn;
    QTableWidget* m_historyTable;

public:
    explicit HistoryWidget(ParkingManager& manager, QWidget* parent = nullptr);

public slots:
    void loadHistory(const QString& filter = "");

private slots:
    void onSearch();
    void onReset();
};

#endif // HISTORY_WIDGET_H
