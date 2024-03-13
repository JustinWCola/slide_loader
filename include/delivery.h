//
// Created by JustinWCola on 2024/3/12.
//

#ifndef DELIVERY_H
#define DELIVERY_H

#include <canopen.h>
#include <servo.h>

#define X_ENCODER_TO_MM (360.0f/10000.0f)*2.0f //X轴编码器位置 -> 实际位置(mm)
#define Z_ENCODER_TO_MM (360.0f/10000.0f)*2.0f //Z轴编码器位置 -> 实际位置(mm)

class Delivery
{
public:
    explicit Delivery(CANopen can) : _can(can){}

    void init();
    bool setAbsPoint(int32_t x, int32_t z);
    bool setRevPoint(int32_t x, int32_t z);

    bool getAbsPoint();

private:
    CANopen _can;
    Servo _axis_x{_can, 1};
    Servo _axis_z{_can, 2};

    int32_t _pos_x;
    int32_t _pos_z;
};

#endif //DELIVERY_H
