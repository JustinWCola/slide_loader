//
// Created by JustinWCola on 2024/2/26.
//
#include <pid.h>

/**
 * 位置式PID核心，得到输出值
 * @param input   PID输入值
 * @return output PID输出值
 */
float Pid::calc(float input)
{
    //传递目标与输入值
    input_now = input;

    //各种优化算法
    if (_pid_mode == PID_RAMP)
        ramp();

    //计算误差值
//    error_prev = error_last;    //位置式PID不需要
    _error_last = _error_now;
    _error_now = target_now - input_now;

    //在精准度范围内，可认为已到达目标值
    if (_error_now < _precision && _error_now > -_precision)
        _error_now = 0;

    //积分限幅
    _error_sum += _error_now;
    _error_sum = LIMIT(_error_sum, _error_max, -_error_max);

    //位置式PID核心公式
    output_now = _kp * _error_now + _ki * _error_sum + _kd * (_error_now - _error_last);

    //输出限幅
    output_now = LIMIT(output_now, _output_max, -_output_max);

    //微分限幅
    output_now = LIMIT(output_now, _output_last + _output_step_max, _output_last - _output_step_max);

    //传递输出值
    _output_last = output_now;
    return output_now;
}

///斜坡函数更新目标值
void Pid::ramp()
{
    static bool ramp_flag = false;
    static float target_last = 0;
    static float ramp_target = 0;

    if (target_now != target_last)
        ramp_flag = true;
    if (ramp_flag)
    {
        ramp_target = target_now;
        target_now = target_last;
        if (target_now < ramp_target)
        {
            target_now += ramp_step;
            if (target_now >= ramp_target)
                target_now = ramp_target;
        }
        else if (target_now > ramp_target)
        {
            target_now -= ramp_step;
            if (target_now <= ramp_target)
                target_now = ramp_target;
        }
        else
            ramp_flag = false;
    }
    target_last = target_now;
}

float Pid::setTarget(float target)
{
    target_now = target;
}

void Pid::setParam(float p, float i, float d)
{
    _kp = p;
    _ki = i;
    _kd = d;
}

void Pid::clear()
{
    target_now = 0;
    input_now = 0;
    output_now = 0;
    _output_last = 0;
    _error_now = 0;
    _error_sum = 0;
    _error_last = 0;
    _error_prev = 0;
}