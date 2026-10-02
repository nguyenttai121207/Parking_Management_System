#ifndef CAMERA_SERVICE_H
#define CAMERA_SERVICE_H

#include <QObject>
#include <QImage>
#include <QTimer>
#include <QString>

#if __has_include(<opencv2/opencv.hpp>)
#include <opencv2/opencv.hpp>
#define HAS_REAL_OPENCV 1
#else
#define HAS_REAL_OPENCV 0
namespace cv {
    class Mat {
    public:
        int rows = 480;
        int cols = 640;
        bool empty() const { return false; }
    };
    class VideoCapture {
    public:
        bool open(const std::string&) { return true; }
        bool isOpened() const { return true; }
        bool read(Mat&) { return true; }
        void release() {}
    };
}
#endif

class CameraService : public QObject {
    Q_OBJECT
private:
    QString m_rtspUrl;
    QTimer* m_timer;
    cv::VideoCapture m_cap;
    bool m_isRunning;
    int m_frameCount;
    cv::Mat m_latestMat;

public:
    explicit CameraService(const QString& rtspUrl = "rtsp://192.168.1.100:554/live", QObject* parent = nullptr);
    ~CameraService() override;

    bool startStream();
    void stopStream();
    bool isStreaming() const { return m_isRunning; }
    QString getRtspUrl() const { return m_rtspUrl; }
    void setRtspUrl(const QString& url) { m_rtspUrl = url; }

    cv::Mat getCurrentFrameMat() const { return m_latestMat; }

signals:
    void frameReady(const QImage& frame);
    void connectionStatusChanged(bool connected);

private slots:
    void grabNextFrame();
};

#endif // CAMERA_SERVICE_H
