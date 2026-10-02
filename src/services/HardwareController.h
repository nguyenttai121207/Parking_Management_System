#ifndef HARDWARE_CONTROLLER_H
#define HARDWARE_CONTROLLER_H

#include <QObject>
#include <QString>
#include <QTimer>

#if __has_include(<QSerialPort>)
#include <QSerialPort>
#define HAS_REAL_QSERIALPORT 1
#else
#define HAS_REAL_QSERIALPORT 0
#endif

class HardwareController : public QObject {
    Q_OBJECT
private:
    bool m_isOpen;
    bool m_connected;
    QString m_portName;
    QTimer* m_autoCloseTimer;
#if HAS_REAL_QSERIALPORT
    QSerialPort* m_serial;
#endif

public:
    explicit HardwareController(const QString& portName = "COM3", QObject* parent = nullptr);
    ~HardwareController() override;

    bool connectDevice();
    void disconnectDevice();

    void openBarrier(const QString& reason = QStringLiteral("Check-in hợp lệ"));
    void emergencyOpen();
    void closeBarrier();

    bool isOpen() const { return m_isOpen; }
    bool isConnected() const { return m_connected; }
    QString getPortName() const { return m_portName; }

signals:
    void barrierStatusChanged(bool isOpen, const QString& reason);
    void serialLogEmitted(const QString& log);
    void connectionStatusChanged(bool connected);

private slots:
    void onAutoCloseTimeout();
};

#endif // HARDWARE_CONTROLLER_H
