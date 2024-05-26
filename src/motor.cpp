//
// Created by JustinWCola on 2024/2/26.
//

#include <Arduino.h>
#include <motor.h>
#include <crc8.h>
#include <Arduino_FreeRTOS.h>

/**
 * 初始化电机引脚并归零
 */
void Motor::init()
{
    pinMode(_cw_pin,OUTPUT);
    pinMode(_ccw_pin,OUTPUT);
    pinMode(_pwm_pin,OUTPUT);
}

/**
 * 设置电机速度（占空比）
 * @param power 占空比
 */
void Motor::setPower(int power)
{
    if (power > 0)
    {
        digitalWrite(_cw_pin, LOW);
        digitalWrite(_ccw_pin, HIGH);
        analogWrite(_pwm_pin, power);
    }
    else if (power < 0)
    {
        digitalWrite(_cw_pin, HIGH);
        digitalWrite(_ccw_pin, LOW);
        analogWrite(_pwm_pin, power);
    }
    else
    {
        digitalWrite(_cw_pin, LOW);
        digitalWrite(_ccw_pin, LOW);
    }
}

/**
 * 设置电机目标值
 * @param target 目标值
 */
void Motor::setTarget(float target)
{
    _is_reach = false;  //到达标志位只在接收到新指令后清零
    // 设置目标位置，单位mm
    _target_now = target;
    _is_cmd = true;
}


/**
 * 设置电机回零（进入任务前初始化阶段）
 */
void Motor::setZeroInit()
{
    uint8_t time = 0;
    uint8_t busy_time=0;
    //会阻塞，需要手动清零
    _is_reach = false;  //到达标志位只在接收到新指令后清零
    while(1)
    {
        // 设置点击倒转直到sw为高或者达到5次
        setPower(-1);
        delay(10);
        if(_sw->getKey() == HIGH)
        {
            if(time > 15)
                // 触发限位开关时，继续运动50ms，避免刚接触限位开关的抖动
                break;
            time++;
        }

        if(busy_time>100)break;
        else if(busy_time<=100)busy_time++;
    }
    clear();
}

/**
 * 设置电机回零（进入任务后）
 */
void Motor::setZero()
{
    uint8_t time = 0;
    //会阻塞，需要手动清零
    _is_reach = false;  //到达标志位只在接收到新指令后清零
    while(1)
    {
        setPower(-1);
        // 任务延时函数
        vTaskDelay(10/portTICK_PERIOD_MS);
        // 读取限位开关的当前状态
        // 在触发HIGH之前一直-1的速度运行，则限位开关应该是触发时置高，不触发则悬空（IO口应该默认下拉处理）
        if(_sw->getKey() == HIGH)
        {
            if(time > 15)
                // 触发限位开关时，继续运动50ms，避免刚接触限位开关的抖动
                break;
            time++;
        }
    }
    // 清空电机状态
    clear();
}

/**
 * 设置电机单位转换，脉冲->mm
 * @param y 转换率
 */
void Motor::setUnitConvert(float y)
{
    _y_to_mm = y;
}

/**
 * 电机更新函数，需要在执行机构任务循环中调用
 */
void Motor::update()
{
    static uint8_t time = 0;
    // 得到编码器脉冲数
    // 获取距离上一次计数差距的脉冲个数
    _input_now = (float)_encoder->getCount();

    // 每次收回之后清空编码器数据，这样就可以消除打滑造成的误差了
    // 由于两个开关为并联，需要判断是否处于收回阶段，这里通过编码器判断
    if(_sw->getKey()==HIGH && _input_now < 100)
    {
        time++;
        // 触发限位开关时，继续运动10ms，避免刚接触限位开关的抖动
        if(time > 2)
        {
            time = 0;
            clear();
        }
    }

    // //判断是否进行归零
    // if(_target_now < 0)
    // {
    //     //阻塞式，若电机未归零，任务无法继续
    //     setZero();
    // }
    // else
    // {
    //     //更新目标值
    //     if(_is_cmd)
    //     {
    //         _pid->setTarget(_target_now / _y_to_mm);
    //         _is_cmd = false;
    //     }
    //     // Serial.println(_pid->target_now);
    //     //PID核心计算并输出
    //     setPower((int)_pid->calc(_input_now));
    //     // print();
    // }

    
    // 统一PID处理，避免收回动作堵塞了
    //更新目标值
    if(_is_cmd)
    {
        _pid->setTarget(_target_now / _y_to_mm);
        _is_cmd = false;
    }
    // Serial.println(_pid->target_now);
    //PID核心计算并输出
    setPower((int)_pid->calc(_input_now));
    // print();
}

/**
 * 更新电机状态
 */
void Motor::updateStatus()
{
    //编码器位置不变，说明电机已停止
    if(abs(_input_now - _input_last) < 2.0f)
    {
        // 若目标值与实际位置相同，说明电机已到达位置；若不相同，说明电机堵转
        // 脉冲数转换为实际长度mm制
        // float _y_to_mm = 2 * (float)PI * _radius / _ratio / _pulse; //单位转换率，脉冲->mm
        // 5mm以内认为已到达
        if(_target_now - _input_now * _y_to_mm < 2.0f)
        {
            // 计时到达时间，消抖20*10=200ms
            if(_reach_time > 20)
                _is_reach = true;
                // 如果出现了定位误差，应该修正这个input值
                // _input_now=_target_now/_y_to_mm;
            else
                _reach_time++;
        }
        else
        {
            // 计时堵转时间，消抖100*10=1000ms
            if(_stuck_time > 100)
                _is_stuck = true;
            else
                _stuck_time++;
        }
    }
    else
    {
        //清空计时
        _is_reach = false;
        _is_stuck = false;
        _reach_time = 0;
        _stuck_time = 0;
    }
    //记录上次编码器的脉冲数
    _input_last = _input_now;
    // if(_is_stuck)
    // {
    //     setZero();
    //     for(;;)
    //         Serial.println("ERROR: loader stuck");
    // }
}

/**
 * 获取电机到达状态
 * @return 是否到达
 */
bool Motor::getReach()
{
    return _is_reach;
}

float Motor::getPos()
{
    return _input_now * _y_to_mm;
}

float Motor::get_sw_Pos()
{
    if(_sw->getKey() == HIGH)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * 清除电机信息
 * 同时置位目标位置为0，电机控制中为脉冲差数
 */
void Motor::clear()
{
    setPower(0);
    _encoder->clear();
    _pid->clear();
    _target_now = 0.0f;
    _input_now = 0.0f;
    _input_last = 0.0f;
}

/**
 * 打印调参数据
 */
void Motor::print()
{
    Serial.print(_pid->target_now);
    Serial.print(",");
    Serial.print(_pid->input_now);
    Serial.print(",");
    Serial.println(_pid->output_now);
}
