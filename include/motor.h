//
// Created by JustinWCola on 2024/2/26.
//

#ifndef MOTOR_H
#define MOTOR_H

#include <pid.h>
#include <encoder.h>
#include <key.h>

#define ENCODER_PULSE 10

class Motor{
public:
    Motor(uint8_t cw_pin, uint8_t ccw_pin, uint8_t pwm_pin, Encoder* encoder, Pid* pid, Key* sw):
    _cw_pin(cw_pin),_ccw_pin(ccw_pin),_pwm_pin(pwm_pin), _encoder(encoder), _pid(pid), _sw(sw){}

    void init();
    void setPower(int power);
    void setTarget(float target);
    void setUnitConvert(float y);
    void setZeroInit();
    void setZero();
    void update();
    void updateStatus();
    bool getReach();
    void send();

private:
    void clear();
    void print();

private:
    uint8_t _cw_pin;    //顺时针旋转引脚
    uint8_t _ccw_pin;   //逆时针旋转引脚
    uint8_t _pwm_pin;   //PWM输出引脚

    Encoder *_encoder;  //编码器指针
    Pid *_pid;          //PID指针
    Key *_sw;           //限位开关指针

    bool _is_reach = false;
    bool _is_push = false;
    bool _is_stuck = false;
    uint8_t _reach_time = 0;
    uint8_t _stuck_time = 0;

    float _target_now = 0.0f;
    float _input_now = 0.0f;
    float _input_last = 0.0f;

    float _radius = 10.0f;  //输出轴半径
    float _ratio = 105.0f;  //电机减速比
    float _pulse = 7.0f;   //编码器分辨率
    float _y_to_mm = 2 * (float)PI * _radius / _ratio / _pulse; //单位转换率，脉冲->mm
};

#endif //MOTOR_H
