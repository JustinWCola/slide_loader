//
// Created by JustinWCola on 2024/2/29.
//
#include <canopen.h>
#include <servo.h>

/**
 * 初始化函数
 */
void Servo::init()
{
    _can.read(_id, I_DEVICE_TYPE, 0, 0);//你是谁

    setCtrlMode(eCtrlMode::CiA402);//设置为CiA402模式
    setMotionMode(eMotionMode::PP);//设置为轮廓位置模式
    setMotorReady();
    disableMotor();
    enableMotor();
}

/**
 * 切换控制模式
 * @param ctrl_mode
 * @return true
 */
bool Servo::setCtrlMode(eCtrlMode ctrl_mode)
{
    _can.write(_id, I_CTRL_PARAM, SI_CTRL_MODE, (uint16_t)ctrl_mode);
    uint16_t new_mode = 0;
    do
    {
        _can.read(_id, I_CTRL_PARAM, SI_CTRL_MODE, (uint32_t*)&new_mode);
        new_mode = (uint16_t)new_mode;
        Serial1.print("setting ctrl mode, ID:");
        Serial1.println(_id);
    } while(new_mode != ctrl_mode);
    return true;
}

/**
 * 切换运动模式
 * @param motion_mode
 * @return true
 */
bool Servo::setMotionMode(eMotionMode motion_mode)
{
    _can.write(_id, I_MOTION_MODE, 0, (uint8_t)motion_mode);
    uint8_t new_mode = 0;
    do
    {
        _can.read(_id, I_MOTION_MODE, 0, (uint32_t*)&new_mode);
        new_mode = (uint8_t)new_mode;
        Serial1.print("setting motion mode, ID:");
        Serial1.println(_id);
    } while(new_mode != motion_mode);
    return true;
}

/**
 * 电机准备
 */
void Servo::setMotorReady()
{
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)0x06);
    Serial1.print("motor ready, ID:");
    Serial1.println(_id);
}

/**
 * 电机失能
 */
void Servo::disableMotor()
{
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)0x07);
    Serial1.print("motor disable, ID:");
    Serial1.println(_id);
}

/**
 * 电机使能
 */
void Servo::enableMotor()
{
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)0x0F);
    Serial1.print("motor enable, ID:");
    Serial1.println(_id);
}

/**
 * 设置目标点
 * @param pos 目标位置
 * @param vel 轮廓速度
 * @return true
 */
bool Servo::setPoint(int32_t pos, uint32_t vel)
{
    _can.write(_id, I_TARGET_POSITION, 0, (uint32_t)pos);
//    can.write(id,I_PROFILE_VELOCITY,0,(uint32_t)velocity);
//    can.write(id,I_END_VELOCITY,0,0x0);
//    can.write(id,I_PROFILE_ACCELERATION,0,acc);
//    can.write(id,I_PROFILE_DECELERATION,0,dec);

    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)0x0F);
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)0x1F);

//    Serial1.print("setting point:");
//    Serial1.print(pos);
    return true;
}

/**
 * 设置目标点
 * @param pos 目标位置
 * @return true
 */
bool Servo::setPoint(int32_t pos)
{
    _can.write(_id, I_TARGET_POSITION, 0, (uint32_t)pos);

    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)0x0F);
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)0x1F);

//    Serial1.print("setting point:");
//    Serial1.print(pos);
    return true;
}
