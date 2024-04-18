//
// Created by JustinWCola on 2024/2/26.
//

#ifndef MOTOR_H
#define MOTOR_H

#include <pid.h>
#include <encoder.h>

#define MOTOR_CW 8
#define MOTOR_CCW 1
#define MOTOR_PWM 9

#define MOTOR_RATIO 105
#define MOTOR_RADIUS 10

void MOTOR_Init();
void MOTOR_Clear();
void MOTOR_SetPower(int power);
void MOTOR_SetTarget(float target);
bool MOTOR_Update();
bool MOTOR_SetUnitConvert(float y);

class Motor{
public:
    Motor(uint8_t cw_pin, uint8_t ccw_pin, uint8_t pwm_pin, Pid pid):
    _cw_pin(cw_pin),_ccw_pin(ccw_pin),_pwm_pin(pwm_pin),_pid(pid){}

    void init();
    void clear();
    void setPower(int power);
    void setTarget(float target);
    void update();
    void setUnitConvert(float y);

private:
    Pid _pid;

    uint8_t _cw_pin;
    uint8_t _ccw_pin;
    uint8_t _pwm_pin;

    bool _is_reach = false;

    float _ratio = MOTOR_RADIUS;
    float _radius = MOTOR_RATIO;

    float _y_to_mm = 2 * (float)PI * _radius / _ratio / ENCODER_PULSE;
};

#endif //MOTOR_H
