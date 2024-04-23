//
// Created by JustinWCola on 2024/3/12.
//

#ifndef DELIVERY_H
#define DELIVERY_H

#include <canopen.h>
#include <servo.h>

#define X_LENGTH 310.0f //X轴丝杆行程
#define Z_LENGTH 120.0f //Z轴丝杆行程

#define X_GUIDE  5.0f   //X轴丝杆导程
#define Z_GUIDE  2.0f   //Z轴丝杆导程

#define SERVO_PULSE 10000.0f    //伺服电机脉冲数

#define X_PULSE_TO_MM (X_GUIDE/SERVO_PULSE) //X轴编码器位置 -> 实际位置(mm)
#define Z_PULSE_TO_MM (Z_GUIDE/SERVO_PULSE) //Z轴编码器位置 -> 实际位置(mm)

#define X_MM_TO_PULSE (SERVO_PULSE/X_GUIDE) //X轴实际位置(mm) -> 编码器位置
#define Z_MM_TO_PULSE (SERVO_PULSE/Z_GUIDE) //Z轴实际位置(mm) -> 编码器位置

class Delivery
{
public:
    explicit Delivery(CANopen can) : _can(can){}

    void init();
    bool setAbsPoint(float x, float z);
    bool setRevPoint(float x, float z);
    bool setUnitConvert(float x, float z);

    bool getAbsPoint();
    bool update();

private:
    CANopen _can;
    Servo _axis_x{_can, 1};
    Servo _axis_z{_can, 2};

    float _x_tar_pos = 309.5;
    float _z_tar_pos = 115.8;

    float _x_now_pos = 0;
    float _z_now_pos = 0;

    float _x_to_mm = X_PULSE_TO_MM;
    float _z_to_mm = Z_PULSE_TO_MM;

    bool _is_reach = false;

    static inline float LIMIT(float var, float max, float min)
    {
        return ((var) < (min) ? (min) : ((var) > (max) ? (max) : (var)));
    }
};

#endif //DELIVERY_H
