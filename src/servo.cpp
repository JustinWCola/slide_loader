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
    uint16_t new_mode = 5;
    do
    {
        _can.read(_id, I_CTRL_PARAM, SI_CTRL_MODE, (uint32_t*)&new_mode);
        new_mode = (uint16_t)new_mode;
        Serial.print("setting ctrl mode, ID:");
        Serial.println(_id);
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
//    uint8_t new_mode = 0x00;
//    do
//    {
//        _can.read(_id, I_MOTION_MODE, 0, (uint32_t*)&new_mode);
//        new_mode = (uint8_t)new_mode;
//        Serial.print("setting motion mode, ID:");
//        Serial.println(_id);
//    } while(new_mode != motion_mode);
    return true;
}

/**
 * 电机准备
 */
void Servo::setMotorReady()
{
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)0x06);
    Serial.print("motor ready, ID:");
    Serial.println(_id);
}

/**
 * 电机失能
 */
void Servo::disableMotor()
{
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)0x07);
    Serial.print("motor disable, ID:");
    Serial.println(_id);
}

/**
 * 电机使能
 */
void Servo::enableMotor()
{
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)0x0F);
    Serial.print("motor enable, ID:");
    Serial.println(_id);
}

/**
 * 设置绝对目标点
 * @param pos 绝对目标位置
 * @param vel 轮廓速度
 * @return true
 */
bool Servo::setAbsPosition(int32_t pos, uint32_t vel)
{
    _can.write(_id, I_TARGET_POSITION, 0, (uint32_t)pos);
//    can.write(id,I_PROFILE_VELOCITY,0,(uint32_t)velocity);
//    can.write(id,I_END_VELOCITY,0,0x0);
//    can.write(id,I_PROFILE_ACCELERATION,0,acc);
//    can.write(id,I_PROFILE_DECELERATION,0,dec);

    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)eTriggerMode::AbsPos);
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)(eTriggerMode::AbsPos + 0x10));

//    Serial.print("setting point:");
//    Serial.print(pos);
    return true;
}

/**
 * 设置绝对目标点
 * @param pos 绝对目标位置
 * @return true
 */
bool Servo::setAbsPosition(int32_t pos)
{
    _can.write(_id, I_TARGET_POSITION, 0, (uint32_t)pos);

    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)eTriggerMode::AbsPos);
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)(eTriggerMode::AbsPos + 0x10));

//    Serial.print("setting point:");
//    Serial.print(pos);
    return true;
}

/**
 * 设置相对目标点
 * @param pos 相对目标位置
 * @param vel 轮廓速度
 * @return true
 */
bool Servo::setRevPosition(int32_t pos, uint32_t vel)
{
    _can.write(_id, I_TARGET_POSITION, 0, (uint32_t)pos);
//    can.write(id,I_PROFILE_VELOCITY,0,(uint32_t)velocity);
//    can.write(id,I_END_VELOCITY,0,0x0);
//    can.write(id,I_PROFILE_ACCELERATION,0,acc);
//    can.write(id,I_PROFILE_DECELERATION,0,dec);

    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)eTriggerMode::RevPos);
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)(eTriggerMode::RevPos + 0x10));

//    Serial.print("setting point:");
//    Serial.print(pos);
    return true;
}

/**
 * 设置相对目标点
 * @param pos 相对目标位置
 * @return true
 */
bool Servo::setRevPosition(int32_t pos)
{
    _can.write(_id, I_TARGET_POSITION, 0, (uint32_t)pos);

    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)eTriggerMode::RevPos);
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)(eTriggerMode::RevPos + 0x10));

//    Serial.print("setting point:");
//    Serial.print(pos);
    return true;
}

/**
 * 获取当前绝对位置
 * @return 当前绝对位置
 */
int32_t Servo::getAbsPosition()
{
    int32_t position;
    _can.read(_id,I_NOW_POSITION,0,(uint32_t*)&position);
    return position;
}

bool Servo::getReach()
{
    uint16_t status;
    _can.read(_id,I_STATUS_WORD,0,(uint32_t*)&status);
    return (status&(0x01<<10))==0;
}
