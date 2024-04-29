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

    disableMotor();//切换模式之前要先失能电机
    setCtrlMode(eCtrlMode::CiA402);//设置为CiA402模式
    setMotionMode(eMotionMode::PP);//设置为轮廓位置模式
    setMotorReady();//电机准备
    disableMotor();//电机失能
    enableMotor();//电机使能
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
    uint8_t new_mode = 0x00;
    do
    {
        _can.read(_id, I_MOTION_MODE, 0, (uint32_t*)&new_mode);
        new_mode = (uint8_t)new_mode;
        Serial.print("setting motion mode, ID:");
        Serial.println(_id);
    } while(new_mode != motion_mode);
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
 * 设置电机回零
 * @return 回零完成
 */
bool Servo::setZero()
{
    _can.read(_id, I_DEVICE_TYPE, 0, 0);//你是谁

    disableMotor();//切换模式之前要先失能电机
    setCtrlMode(eCtrlMode::CiA402);//设置为CiA402模式
    setMotionMode(eMotionMode::HM);//设置为原点回归模式

    // 这些参数需要设置之后给伺服电机下电才会生效，因此不能在这里设置
    // _can.write(_id, I_POSITION_CONTROL, SI_ZERO_TIME_LIMIT, (uint16_t)50000);
    // _can.write(_id, I_ZERO_MODE, 0, (uint8_t)eZeroMode::NegativeStuck);
    // _can.write(_id, I_ZERO_VELOCITY, SI_LOW_VELOCITY, (uint32_t)50000);
    // _can.write(_id, I_ZERO_ACCELERATION, 0, (uint32_t)409600);
    //
    // _can.write(_id, I_STUCK_CHECK, SI_STUCK_TORQUE, (uint16_t)500);
    // _can.write(_id, I_STUCK_CHECK, SI_STUCK_TIME, (uint16_t)10);

    setMotorReady();//电机准备
    disableMotor();//电机失能
    enableMotor();//电机使能

    //触发电机运行
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)eTriggerMode::AbsPos);
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)(eTriggerMode::AbsPos + 0x10));

    //检测回零是否完成，电机状态字的第12位会在回零完成后置1
    volatile uint32_t status = 0;
    while((status&(1<<12))>>12 == 0)
    {
        delay(100);
        Serial.print("setting zero, ID:");
        Serial.println(_id);
        _can.read(_id,I_STATUS_WORD,0,(uint32_t*)&status);
    }

    return true;
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

/**
 * 获取电机到达信号
 * @return 是否到达
 */
bool Servo::getReach()
{
    //电机状态字的第10位会在位置到达后置1
    uint16_t status;
    _can.read(_id,I_STATUS_WORD,0,(uint32_t*)&status);
    return (status&(1<<10))>>10;
}
