//
// Created by JustinWCola on 2024/2/29.
//
#include <canopen.h>
#include <servo.h>
#include <crc8.h>

/**
 * 初始化函数
 */
void Servo::init()
{
    _can.read(_id, I_DEVICE_TYPE, 0, 0);//你是谁

    // setZero();
    clearError();
    disableMotor();//切换模式之前要先失能电机
    setCtrlMode(eCtrlMode::CiA402);//设置为CiA402模式
    setMotionMode(eMotionMode::PP);//设置为轮廓位置模式
    setMotorReady();//电机准备
    disableMotor();//电机失能
    enableMotor();//电机使能
}

/**
 * 设置传送机构绝对位置
 * @param pos 绝对位置
 */
void Servo::setAbsPos(float pos)
{
    _target_now = pos;
}

/**
 * 设置传送机构相对位置
 * @param pos 相对位置
 */
void Servo::setRevPos(float pos)
{
    _target_now += pos;
}

/**
 * 设置单位转换
 * @param unit 单位转换率
 */
void Servo::setUnitConvert(float unit)
{
    _pulse_to_mm = unit;
}

/**
 * 传送机构更新函数，需要在传送机构任务循环中调用
 */
void Servo::update()
{
    setAbsPosition((int32_t)(_target_now / _pulse_to_mm));
}

/**
 * 更新传送机构状态
 */
void Servo::updateStatus()
{
    _input_now = (float)getAbsPosition() * _pulse_to_mm;

    if(abs(_input_now - _target_now) < 0.1f)
    {
        //计时到达时间，消抖20*10=200ms
        if(_reach_time > 20)
            _is_reach = true;
        else
            _reach_time++;
    }
    else
    {
        //清空计时
        _is_reach = false;
        _reach_time = 0;
    }
}

/**
 * 获取传送机构到达状态
 * @return 是否达到
 */
bool Servo::getReach()
{
    return _is_reach;
}

/**
 * 发送传送机构位置与到达报文，需要在串口任务循环中调用，注意分时
 */
void Servo::send()
{
    uint8_t tx_data[10] = {0};
    static bool last_reach;

    //检测到达标志位的上升沿，只在到达后发送一次消息
    if(_is_reach && !last_reach)
    {
        tx_data[0] = 0xAA;
        tx_data[1] = 0xC2;
        tx_data[2] = _is_reach;
        tx_data[3] = crc8Check(tx_data,3);
        tx_data[4] = 0xFF;
        Serial.write(tx_data,5);
    }
    // else
    // {
    //     tx_data[0] = 0xA1;
    //     tx_data[1] = 0xC1;
    //     memcpy(tx_data+2,&_x_now_pos,4);
    //     memcpy(tx_data+6,&_z_now_pos,4);
    //     Serial.write(tx_data,10);
    // }
    last_reach = _is_reach;
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

bool Servo::clearError()
{
    disableMotor();//故障复位之前要先失能电机
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)0x80);
    enableMotor();//电机使能
}

/**
 * 设置电机回零
 * @return 回零完成
 */
bool Servo::setZero()
{
    disableMotor();//切换模式之前要先失能电机
    setCtrlMode(eCtrlMode::CiA402);//设置为CiA402模式
    setMotionMode(eMotionMode::HM);//设置为原点回归模式

    _can.write(_id, I_POSITION_CONTROL, SI_ZERO_TIME_LIMIT, (uint16_t)50000);
    _can.write(_id, I_ZERO_MODE, 0, (uint8_t)eZeroMode::NegativeStuck);
    _can.write(_id, I_ZERO_VELOCITY, SI_LOW_VELOCITY, (uint32_t)50000);
    _can.write(_id, I_ZERO_ACCELERATION, 0, (uint32_t)409600);

    _can.write(_id, I_STUCK_CHECK, SI_STUCK_TORQUE, (uint16_t)500);
    _can.write(_id, I_STUCK_CHECK, SI_STUCK_TIME, (uint16_t)10);

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

    // _can.write(_id,I_MAX_PROFILE_VELOCITY,0,(uint32_t)300000000);
    // _can.write(_id,I_MAX_PROFILE_ACCELERATION,0,(uint32_t)1000000000);
    // _can.write(_id,I_MAX_PROFILE_DECELERATION,0,(uint32_t)1000000000);
    // _can.write(_id,I_PROFILE_VELOCITY,0,(uint32_t)300000000);
    // _can.write(_id,I_PROFILE_ACCELERATION,0,(uint32_t)1000000000);
    // _can.write(_id,I_PROFILE_DECELERATION,0,(uint32_t)1000000000);
    // _can.write(_id,I_PROFILE_JERK,SI_PROFILE_JERK_ACC,(uint32_t)500000);
    // _can.write(_id,I_PROFILE_JERK,SI_PROFILE_JERK_DEC,(uint32_t)500000);

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
