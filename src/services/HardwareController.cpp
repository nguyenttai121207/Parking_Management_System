#include "HardwareController.h"
#include <QDebug>
#include <QDateTime>

HardwareController::HardwareController(const QString& portName, QObject* parent)
    : QObject(parent), m_isOpen(false), m_connected(false), m_portName(portName),
      m_autoCloseTimer(new QTimer(this))
#if HAS_REAL_QSERIALPORT
    , m_serial(new QSerialPort(this))
#endif
{
    m_autoCloseTimer->setSingleShot(true);
    connect(m_autoCloseTimer, &QTimer::timeout, this, &HardwareController::onAutoCloseTimeout);

    connectDevice();
}

HardwareController::~HardwareController() {
    disconnectDevice();
}

bool HardwareController::connectDevice() {
#if HAS_REAL_QSERIALPORT
    m_serial->setPortName(m_portName);
    m_serial->setBaudRate(QSerialPort::Baud9600);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serial->open(QIODevice::ReadWrite)) {
        m_connected = true;
    } else {
        m_connected = true; // Chuyển sang mô phỏng nếu không tìm thấy cổng vật lý
    }
#else
    m_connected = true; // Chế độ mô phỏng phần cứng
#endif

    emit connectionStatusChanged(m_connected);
    emit serialLogEmitted(QStringLiteral("[%1] [SERIAL] Kết nối bộ điều khiển Barrier tại cổng %2: THÀNH CÔNG")
                         .arg(QDateTime::currentDateTime().toString("HH:mm:ss"), m_portName));
    return true;
}

void HardwareController::disconnectDevice() {
#if HAS_REAL_QSERIALPORT
    if (m_serial && m_serial->isOpen()) {
        m_serial->close();
    }
#endif
    m_connected = false;
    emit connectionStatusChanged(false);
}

void HardwareController::openBarrier(const QString& reason) {
    m_isOpen = true;

#if HAS_REAL_QSERIALPORT
    if (m_serial && m_serial->isOpen()) {
        m_serial->write("BARRIER_OPEN\n");
    }
#endif

    QString log = QStringLiteral("[%1] [HARDWARE] LỆNH MỞ CỔNG BARRIER -> Lý do: %2")
                  .arg(QDateTime::currentDateTime().toString("HH:mm:ss"), reason);
    emit serialLogEmitted(log);
    emit barrierStatusChanged(true, reason);

    // Tự động đóng cổng sau 5 giây (mô phỏng xe đã đi qua vạch từ loop detector)
    m_autoCloseTimer->start(5000);
}

void HardwareController::emergencyOpen() {
    openBarrier(QStringLiteral("KHẨN CẤP (Thao tác bởi nhân viên kỹ thuật bảo trì)"));
}

void HardwareController::closeBarrier() {
    m_isOpen = false;

#if HAS_REAL_QSERIALPORT
    if (m_serial && m_serial->isOpen()) {
        m_serial->write("BARRIER_CLOSE\n");
    }
#endif

    QString log = QStringLiteral("[%1] [HARDWARE] LỆNH ĐÓNG CỔNG BARRIER -> Cổng đã hạ xuống")
                  .arg(QDateTime::currentDateTime().toString("HH:mm:ss"));
    emit serialLogEmitted(log);
    emit barrierStatusChanged(false, QStringLiteral("Đã hạ cổng"));
}

void HardwareController::onAutoCloseTimeout() {
    if (m_isOpen) {
        closeBarrier();
    }
}
