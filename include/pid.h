//
// Created by JustinWCola on 2024/2/26.
//

#ifndef PID_H
#define PID_H

class Pid
{
public:
    typedef enum
    {
        PID_NORMAL = 0, //正常模式
        PID_RAMP = 1    //斜坡模式
    } ePidMode;

public:
    Pid() = default;

    Pid(float kp, float ki, float kd) :
            _kp(kp), _ki(ki), _kd(kd)
    {}

    Pid(float kp, float ki, float kd, float error_max, float output_max, float output_step_max, float ramp_step) :
            _kp(kp), _ki(ki), _kd(kd), _error_max(error_max), _output_max(output_max), _output_step_max(output_step_max), ramp_step(ramp_step)
    {}

    float calc(float target, float input);
    void setParam(float p, float i, float d);
    void clear();

private:
    static inline float LIMIT(float var, float max, float min)
    {
        return ((var) < (min) ? (min) : ((var) > (max) ? (max) : (var)));
    }

    void ramp();

public:
    float input_now = 0;            //当前输入值
    float target_now = 0;           //当前目标值
    float output_now = 0;           //当前输出值

    float ramp_step = 10;           //斜坡步进值（加速度）

private:
    float _kp = 0;                   //比例项系数
    float _ki = 0;                   //积分项系数
    float _kd = 0;                   //微分项系数

    float _error_sum = 0;            //累计误差值
    float _error_now = 0;            //当前误差值
    float _error_last = 0;           //上次误差值
    float _error_prev = 0;           //上上次误差值
    float _error_max = 655350;       //最大误差值

    float _output_last = 0;          //上次输出值
    float _output_max = 10000;       //最大输出值
    float _output_step_max = 10000;  //最大步进输出值

    float _precision = 0;            //精确度

    bool _pid_mode = PID_RAMP;       //PID模式
};

#endif //PID_H
