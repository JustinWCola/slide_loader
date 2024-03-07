//
// Created by JustinWCola on 2024/2/29.
//
#include <canopen.h>
#include <servo.h>

Servo myServo;

uint8_t Servo::id = 1;
CANopen Servo::can = CANopen();

void Servo::init()
{
    can.begin(CanBitRate::BR_1000k);
    can.read(id, I_DEVICE_TYPE, 0, 0);//你是谁

    setCtrlMode(eCtrlMode::CiA402);//设置为CiA402模式
    setMotionMode(eMotionMode::PP);//设置为轮廓位置模式
    setMotorReady();
    disableMotor();
    enableMotor();
}

bool Servo::setCtrlMode(eCtrlMode ctrl_mode)
{
    can.write(id, I_CTRL_PARAM, SI_CTRL_MODE, ctrl_mode);
    uint16_t new_mode = 0;
    do
    {
        can.read(id,I_CTRL_PARAM,SI_CTRL_MODE,(uint32_t*)&new_mode);
        new_mode = (uint16_t)new_mode;
        Serial1.println("setting ctrl mode");
    } while(new_mode != ctrl_mode);
    return true;
}

bool Servo::setMotionMode(eMotionMode motion_mode)
{
    can.write(id,I_MOTION_MODE,0,motion_mode);
    uint8_t new_mode = 0;
    do
    {
        can.read(id,I_MOTION_MODE,0,(uint32_t*)&new_mode);
        new_mode = (uint8_t)new_mode;
        Serial1.println("setting motion mode");
    } while(new_mode != motion_mode);
    return true;
}

void Servo::setMotorReady()
{
    can.write(id, I_CONTROL_WORD, 0, (uint16_t)0x06);
}

void Servo::disableMotor()
{
    can.write(id, I_CONTROL_WORD, 0, (uint16_t)0x07);
}

void Servo::enableMotor()
{
    can.write(id, I_CONTROL_WORD, 0, (uint16_t)0x0F);
}

bool Servo::setPoint(int32_t position, uint32_t velocity)
{
    can.write(id, I_TARGET_POSITION,0,(uint32_t)position);
//    can.write(id,I_PROFILE_VELOCITY,0,(uint32_t)velocity);
//    can.write(id,I_END_VELOCITY,0,0x0);
//    can.write(id,I_PROFILE_ACCELERATION,0,acc);
//    can.write(id,I_PROFILE_DECELERATION,0,dec);

    can.write(id, I_CONTROL_WORD, 0, (uint16_t)0x2F);
    uint16_t status = 0;
    do {
        can.read(id, I_STATUS_WORD, 0, (uint32_t*)&status);
    } while ((status&0x1000)!=0x1000);
    can.write(id, I_CONTROL_WORD, 0, (uint16_t)0x3F);
    return true;

}
