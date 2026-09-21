#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include "db/DatabaseManager.h"
#include "ui/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QApplication::setApplicationName("ParkingManagementSystem");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("CSStudent");

    QFile styleFile(":/styles/dark_theme.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        app.setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qWarning() << "Không thể tải tài nguyên giao diện dark_theme.qss, dùng giao diện mặc định.";
    }

    if (!DatabaseManager::instance().openDatabase("parking_system.db")) {
        QMessageBox::critical(nullptr, QStringLiteral("Lỗi kết nối CSDL"),
            QStringLiteral("Không thể mở hoặc tạo tệp cơ sở dữ liệu SQLite (parking_system.db).\nỨng dụng sẽ thoát."));
        return 1;
    }

    MainWindow mainWindow;
    mainWindow.show();

    int execCode = app.exec();

    DatabaseManager::instance().closeDatabase();

    return execCode;
}
