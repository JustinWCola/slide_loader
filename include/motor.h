//
// Created by JustinWCola on 2024/2/26.
//

#ifndef MOTOR_H
#define MOTOR_H

#include <pid.h>
#include <encoder.h>

#define ENCODER_PULSE 10

class Motor{
public:
    Motor(uint8_t cw_pin, uint8_t ccw_pin, uint8_t pwm_pin, Encoder* encoder, Pid* pid):
    _cw_pin(cw_pin),_ccw_pin(ccw_pin),_pwm_pin(pwm_pin), _encoder(encoder),_pid(pid){}

    void init();
    void clear();
    void setPower(int power);
    void setTarget(float target);
    void update();
    void setUnitConvert(float y);

private:
    Encoder* _encoder;
    Pid* _pid;

    uint8_t _cw_pin;
    uint8_t _ccw_pin;
    uint8_t _pwm_pin;

    bool _is_reach = false;
    uint32_t _reach_time = 0;

    float _radius = 10.0f;
    float _ratio = 105.0f;
    float _pulse = 10.0f;

    float _y_to_mm = 2 * (float)PI * _radius / _ratio / _pulse;
};

#endif //MOTOR_H
