//
// Created by JustinWCola on 2024/2/26.
//

#ifndef MOTOR_H
#define MOTOR_H

#include <pid.h>
#include <encoder.h>

#define MOTOR_RATIO 105
#define MOTOR_RADIUS 10
#define ENCODER_PULSE 10

class Motor{
public:
    Motor(uint8_t cw_pin, uint8_t ccw_pin, uint8_t pwm_pin, Encoder encoder, Pid pid):
    _cw_pin(cw_pin),_ccw_pin(ccw_pin),_pwm_pin(pwm_pin), _encoder(encoder),_pid(pid){}

    void init();
    void clear();
    void setPower(int power);
    void setTarget(float target);
    void update();
    void setUnitConvert(float y);

private:
    Encoder _encoder;
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
