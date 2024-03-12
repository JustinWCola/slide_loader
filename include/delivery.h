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
    Delivery(CANopen can) : _can(can){}

    void init();
    bool setPoint(int32_t x, int32_t z);

private:
    CANopen _can;
    Servo _axis_x{_can, 1};
    Servo _axis_z{_can, 2};
};

#endif //DELIVERY_H
