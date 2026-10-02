#include <cassert>
#include <iostream>
#include <QCoreApplication>
#include <QDateTime>
#include <QCryptographicHash>
#include "models/VehicleType.h"
#include "models/PricingModel.h"
#include "db/DatabaseManager.h"
#include "db/ParkingRepository.h"
#include "services/SessionManager.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "[TEST] 1. Testing PricingModel calculateFee...\n";
    PricingModel model;
    QDateTime inTime = QDateTime::currentDateTime();

    // Test Bicycle (Xe đạp): first block 2000, next 1000
    double fee30min = model.calculateFee(VehicleType::Bicycle, inTime, inTime.addSecs(30 * 60));
    assert(fee30min == 2000.0);

    double fee90min = model.calculateFee(VehicleType::Bicycle, inTime, inTime.addSecs(90 * 60));
    assert(fee90min == 3000.0);

    // Test Car (Ô tô con): first block 25000, next 15000
    double feeCar120 = model.calculateFee(VehicleType::Car, inTime, inTime.addSecs(120 * 60));
    assert(feeCar120 == 40000.0);
    std::cout << "[PASS] PricingModel calculation verified.\n";

    std::cout << "[TEST] 2. Testing Database and Repository...\n";
    bool dbOk = DatabaseManager::instance().openDatabase(":memory:");
    assert(dbOk);

    ParkingRepository repo;

    // Test Users & RBAC
    auto adminUser = repo.findUserByUsername("admin");
    assert(adminUser.has_value());
    assert(adminUser->isAdmin());

    QString adminHash = QString::fromLatin1(QCryptographicHash::hash("admin123", QCryptographicHash::Sha256).toHex());
    auto verifiedAdmin = repo.verifyUserCredentials("admin", adminHash);
    assert(verifiedAdmin.has_value());

    auto techUser = repo.findUserByUsername("tech");
    assert(techUser.has_value());
    assert(techUser->isMaintenance());
    std::cout << "[PASS] User authentication & RBAC verified.\n";

    // Test MonthlyPasses
    auto validPass = repo.findValidMonthlyPass("29A-839.21", QDate::currentDate());
    assert(validPass.has_value());
    assert(validPass->isValid());

    // Test ParkingSessions
    auto session = repo.createSession("51F-999.88", VehicleType::MotorbikeManual, inTime, 0.0, "Đang đỗ");
    assert(session.has_value());
    assert(session->isActive());

    auto activeFound = repo.findActiveSessionByPlate("51F-999.88");
    assert(activeFound.has_value());
    assert(activeFound->getId() == session->getId());

    bool paid = repo.updateSessionPayment(session->getId(), inTime.addSecs(3600), 4000.0, "Đã thanh toán");
    assert(paid);

    auto activeAfterPaid = repo.findActiveSessionByPlate("51F-999.88");
    assert(!activeAfterPaid.has_value());
    std::cout << "[PASS] ParkingSessions check-in and checkout verified.\n";

    DatabaseManager::instance().closeDatabase();
    std::cout << "\n>>> ALL UNIT CHECKS PASSED SUCCESSFULLY! <<<\n";
    return 0;
}
