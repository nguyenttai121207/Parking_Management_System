#ifndef ANPR_SERVICE_H
#define ANPR_SERVICE_H

#include "CameraService.h"
#include <QString>
#include <vector>

class IANPRService {
public:
    virtual ~IANPRService() = default;
    virtual QString recognizeLicensePlate(const cv::Mat& frame) = 0;
};

class MockANPRService : public IANPRService {
private:
    std::vector<QString> m_mockPlates;

public:
    MockANPRService();
    QString recognizeLicensePlate(const cv::Mat& frame) override;
};

#endif // ANPR_SERVICE_H
