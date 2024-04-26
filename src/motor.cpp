//
// Created by JustinWCola on 2024/2/26.
//

#include <Arduino.h>
#include <motor.h>

/**
 * 初始化电机引脚并归零
 */
void Motor::init()
{
    pinMode(_cw_pin,OUTPUT);
    pinMode(_ccw_pin,OUTPUT);
    pinMode(_pwm_pin,OUTPUT);

    //电机归零
    setZero();
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
    else
    {
        digitalWrite(_cw_pin, HIGH);
        digitalWrite(_ccw_pin, LOW);
        analogWrite(_pwm_pin, power);
    }
}

/**
 * 设置电机目标值
 * @param target 目标值
 */
void Motor::setTarget(float target)
{
    _target_now = target;
}

/**
 * 设置电机回零
 */
void Motor::setZero()
{
    while (_sw->getKey() != HIGH)
        setPower(-5);
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
    //得到编码器脉冲数
    _input_now = (float)_encoder->getCount();

    //判断是否归零
    if(_target_now <= 0)
        //阻塞式，若电机未归零，任务无法继续
        setZero();
    else
    {
        //更新目标值
        _pid->setTarget(_target_now / _y_to_mm);
        //PID核心计算并输出
        setPower((int)_pid->calc(_input_now));
        // print();

        //编码器位置不变，说明电机已停止
        if(abs(_input_now - _input_last) < 0.1f)
            //若目标值与实际位置相同，说明电机已到达位置；若不相同，说明电机堵转
            if((_target_now - _input_now * _y_to_mm) < 1.5f)
                //计时到达时间，消抖20*10=200ms
                if(_reach_time > 20)
                    _is_reach = true;
                else
                    _reach_time++;
            else
                //计时堵转时间，消抖100*10=1000ms
                if(_stuck_time > 100)
                    _is_stuck = true;
                else
                    _stuck_time++;
        else
        {
            //清空计时
            _is_reach = false;
            _is_stuck = false;
            _reach_time = 0;
            _stuck_time = 0;
        }
    }
    //记录上次编码器的脉冲数
    _input_last = _input_now;
}

/**
 * 发送电机到达报文，需要在串口任务循环中调用，注意分时
 */
void Motor::send()
{
    // static uint32_t send_time = 0;
    //
    // if(send_time % 50 == 0)
    // {
        static uint8_t tx_data[3];
        tx_data[0] = 0xA1;
        tx_data[1] = 0xC3;
        tx_data[2] = _is_reach;
        Serial.write(tx_data,3);
    // }
    // send_time++;
}


/**
 * 清除电机信息
 */
void Motor::clear()
{
    _encoder->clear();
    _pid->clear();
    setPower(0);
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
