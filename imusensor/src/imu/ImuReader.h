#ifndef __IMU_IMU_READER_H__
#define __IMU_IMU_READER_H__

#include <M5Unified.h>
#include "mahony/MahonyAHRS.h"
#include "ImuData.h"

namespace imu {

class ImuReader {
public:
    void acc_rotate(imu::ImuData& imuData);
    explicit ImuReader(m5::IMU_Class& m5);
    bool initialize();
    bool writeGyroOffset(float x, float y, float z);
    bool update();
    bool read(ImuData& outImuData) const;
private:
    m5::IMU_Class& m5Imu;
    mahony::MahonyAHRS ahrs;
    ImuData imuData;
    uint32_t lastUpdated;
    float gyroOffsets[ImuXyz];
};

} // imu

#endif // __IMU_IMU_READER_H__
