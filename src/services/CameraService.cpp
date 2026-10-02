#include "CameraService.h"
#include <QPainter>
#include <QDateTime>
#include <QFont>
#include <QPen>

CameraService::CameraService(const QString& rtspUrl, QObject* parent)
    : QObject(parent), m_rtspUrl(rtspUrl), m_timer(new QTimer(this)),
      m_isRunning(false), m_frameCount(0) {
    connect(m_timer, &QTimer::timeout, this, &CameraService::grabNextFrame);
}

CameraService::~CameraService() {
    stopStream();
}

bool CameraService::startStream() {
    if (m_isRunning) return true;

#if HAS_REAL_OPENCV
    if (!m_cap.open(m_rtspUrl.toStdString())) {
        emit connectionStatusChanged(false);
        return false;
    }
#endif

    m_isRunning = true;
    m_timer->start(40); // ~25 FPS
    emit connectionStatusChanged(true);
    return true;
}

void CameraService::stopStream() {
    if (!m_isRunning) return;
    m_timer->stop();
#if HAS_REAL_OPENCV
    m_cap.release();
#endif
    m_isRunning = false;
    emit connectionStatusChanged(false);
}

void CameraService::grabNextFrame() {
    m_frameCount++;

#if HAS_REAL_OPENCV
    cv::Mat frame;
    if (m_cap.read(frame) && !frame.empty()) {
        m_latestMat = frame;
        // Chuyển BGR sang RGB và gói vào QImage
        cv::Mat rgb;
        cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
        QImage img(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
        emit frameReady(img.copy());
        return;
    }
#endif

    // Fallback: Tạo hình ảnh mô phỏng Camera IP RTSP độ phân giải 640x360
    QImage simImg(640, 360, QImage::Format_RGB32);
    simImg.fill(QColor(18, 20, 29));

    QPainter p(&simImg);
    p.setRenderHint(QPainter::Antialiasing);

    // Vẽ vạch kẻ làn đường vào bãi
    p.setPen(QPen(QColor(49, 50, 68), 2, Qt::DashLine));
    p.drawLine(100, 360, 240, 140);
    p.drawLine(540, 360, 400, 140);
    p.drawLine(320, 360, 320, 140);

    // Vẽ mô phỏng Barrier bar
    p.setPen(QPen(QColor(243, 139, 168), 6));
    p.drawLine(180, 220, 460, 220);

    // Vẽ khu vực nhận diện biển số (Bounding Box mô phỏng AI)
    int offset = (m_frameCount * 2) % 30;
    QRect targetBox(220, 180 + (offset / 10), 200, 70);
    p.setPen(QPen(QColor(166, 227, 161), 2));
    p.drawRoundedRect(targetBox, 6, 6);
    p.drawText(targetBox.adjusted(6, -18, 0, 0), Qt::AlignLeft, QStringLiteral("ANPR SCAN AREA [ACTIVE]"));

    // Vẽ biển số mẫu trong khung
    p.setPen(QColor(205, 214, 244));
    QFont plateFont("Monospace", 12, QFont::Bold);
    p.setFont(plateFont);
    p.drawText(targetBox, Qt::AlignCenter, QStringLiteral("29A - 839.21"));

    // OSD Header: Tên Camera, Timestamp, RTSP Status
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 160));
    p.drawRect(0, 0, 640, 32);

    p.setPen(QColor(166, 227, 161));
    QFont osdFont("Segoe UI", 10, QFont::Bold);
    p.setFont(osdFont);
    p.drawText(12, 22, QStringLiteral("● CAM-01 [RTSP GATE ENTRANCE] 25FPS 1080P"));

    p.setPen(QColor(205, 214, 244));
    QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    p.drawText(QRect(300, 0, 328, 32), Qt::AlignRight | Qt::AlignVCenter, timeStr);

    p.end();

    emit frameReady(simImg);
}
