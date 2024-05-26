//
// Created by JustinWCola on 2024/2/29.
//
#include <canopen.h>
#include <servo.h>
#include <crc8.h>
#include <led.h>

/**
 * 初始化函数
 */
void Servo::init(Led* led)
{
    uint32_t data = 0;
    bool servo_init_judge=false;
    
    Serial.print("begin to init servo");
    while(servo_init_judge!=true)
    {
        // Serial.print("Servo init 0");
        // id 10 00
        getInfo();//获取电机信息

        // Serial.print("Servo init 1");
        clearError();
        
        // Serial.print("Servo init 2");
        servo_init_judge=setZero(led);
    }

    // Serial.print("Servo init 3");
    disableMotor();//切换模式之前要先失能电机

    // Serial.print("Servo init 4");
    setCtrlMode(eCtrlMode::CiA402);//设置为CiA402模式

    // Serial.print("Servo init 5");
    setMotionMode(eMotionMode::PP);//设置为轮廓位置模式

    // Serial.print("Servo init 6");
    setMotorReady();//电机准备

    // Serial.print("Servo init 7");
    disableMotor();//电机失能

    // Serial.print("Servo init 8");
    enableMotor();//电机使能

    // Serial.print("Servo init 9");
}

/**
 * 设置伺服电机绝对位置
 * @param pos 绝对位置
 */
void Servo::setAbsPos(float pos)
{
    _target_now = pos;
}

/**
 * 设置伺服电机相对位置
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
 * 伺服电机更新函数，需要在传送机构任务循环中调用
 */
void Servo::update()
{
    setAbsPosition((int32_t)(_target_now / _pulse_to_mm));
}

/**
 * 更新伺服电机状态
 */
void Servo::updateStatus()
{
    _input_now = (float)getAbsPosition() * _pulse_to_mm;

    if(abs(_input_now - _target_now) < 0.01f)
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
 * 获取伺服电机到达状态
 * @return 是否达到
 */
bool Servo::getReach()
{
    return _is_reach;
}

/**
 * 获取伺服电机绝对位置
 * @return 当前位置
 */
float Servo::getAbsPos()
{
    return _input_now;
}

/**
 * 切换控制模式
 * @param ctrl_mode
 * @return true
 * 
#define I_CTRL_PARAM      0x2002  //控制参数
#define SI_CTRL_MODE      0x01    //控制模式
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
#define I_CONTROL_WORD    0x6040  //控制字
 */
void Servo::disableMotor()
{
    // id 60 40 07
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)0x07);
    Serial.print("motor disable, ID:");
    Serial.println(_id);
}

/**
 * 电机使能
#define I_CONTROL_WORD    0x6040  //控制字
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
    for(int i = 0; i < 20; i++)
        _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)0x80);
    enableMotor();//电机使能
    return true;
}

/**
 * 设置电机回零
 * @return 回零完成
 */
bool Servo::setZero(Led* led)
{
    delay(10);
    disableMotor();//切换模式之前要先失能电机
    delay(10);
    setCtrlMode(eCtrlMode::CiA402);//设置为CiA402模式
    delay(10);
    setMotionMode(eMotionMode::HM);//设置为原点回归模式
    delay(10);

    _can.write(_id, I_POSITION_CONTROL, SI_ZERO_TIME_LIMIT, (uint16_t)50000);
    delay(10);
    _can.write(_id, I_ZERO_MODE, 0, (uint8_t)eZeroMode::NegativeStuck);
    delay(10);
    _can.write(_id, I_ZERO_VELOCITY, SI_LOW_VELOCITY, (uint32_t)50000);
    delay(10);
    _can.write(_id, I_ZERO_ACCELERATION, 0, (uint32_t)409600);
    delay(10);

    _can.write(_id, I_STUCK_CHECK, SI_STUCK_TORQUE, (uint16_t)500);
    delay(10);
    _can.write(_id, I_STUCK_CHECK, SI_STUCK_TIME, (uint16_t)10);
    delay(10);

    setMotorReady();//电机准备
    delay(10);
    disableMotor();//电机失能
    delay(10);
    enableMotor();//电机使能
    delay(10);

    delay(100);
    //触发电机运行
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)eTriggerMode::AbsPos);
    delay(10);
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)(eTriggerMode::AbsPos + 0x10));
    delay(10);

    //检测回零是否完成，电机状态字的第12位会在回零完成后置1
    volatile uint32_t status = 0;
    uint8_t servo_init_time=0;
    while((status&(1<<12))>>12 == 0)
    {
        delay(100);
        Serial.print("setting zero, ID:");
        Serial.println(_id);
        
        // #define I_STATUS_WORD     0x6041  //状态字
        _can.read(_id,I_STATUS_WORD,0,(uint32_t*)&status);
        if(servo_init_time%9==0)
        {
        led[0].setColor(Red);
        }
        else if(servo_init_time%9==3)
        {
        led[0].setColor(Green);
        }
        else if(servo_init_time%9==6)
        {
        led[0].setColor(Yellow);
        }
        // Red = 0x01,
        // Green = 0x10,
        // Yellow = 0x11,
        
        servo_init_time+=1;
        if(servo_init_time>=50)
        {
            // 超时判断没成功初始化
            return false;
        }
    }

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

    // 此处修改运行速度，注意各数据的设置值会以其最大值作为上限
    _can.write(_id,I_MAX_PROFILE_VELOCITY,0,(uint32_t)300000000);
    _can.write(_id,I_MAX_PROFILE_ACCELERATION,0,(uint32_t)1000000000);
    _can.write(_id,I_MAX_PROFILE_DECELERATION,0,(uint32_t)1000000000);
    _can.write(_id,I_PROFILE_VELOCITY,0,(uint32_t)300000000);
    _can.write(_id,I_PROFILE_ACCELERATION,0,(uint32_t)1000000000);
    _can.write(_id,I_PROFILE_DECELERATION,0,(uint32_t)1000000000);
    // 主要修改这两个参数
    _can.write(_id,I_PROFILE_JERK,SI_PROFILE_JERK_ACC,(uint32_t)10000000);
    _can.write(_id,I_PROFILE_JERK,SI_PROFILE_JERK_DEC,(uint32_t)10000000);

    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)eTriggerMode::AbsPos);
    _can.write(_id, I_CONTROL_WORD, 0, (uint16_t)(eTriggerMode::AbsPos + 0x10));

    // Serial.print("setting point:");
    // Serial.print(pos);
    // Serial.print("\n");
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
 * 获取电机信息
#define I_DEVICE_TYPE     0x1000  //设备类型
 */
void Servo::getInfo()
{
    uint32_t data = 0;
    _can.read(_id, I_DEVICE_TYPE, 0, &data); //你是谁
}