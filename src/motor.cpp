//
// Created by JustinWCola on 2024/2/26.
//

#include <Arduino.h>
#include <motor.h>

void Motor::init()
{
    pinMode(_cw_pin,OUTPUT);
    pinMode(_ccw_pin,OUTPUT);
    pinMode(_pwm_pin,OUTPUT);
}


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

void Motor::setTarget(float target)
{
    _pid->setTarget(target / _y_to_mm);
}

void Motor::update()
{
    setPower((int)_pid->calc((float)_encoder->getCount()));
    if (abs(_pid->input_now - _pid->target_now) < 5.0f)
        _reach_time++;
    else
        _reach_time = 0;
    if(_reach_time > 200)
        _is_reach = true;
    else
        _is_reach = false;

    uint8_t tx_data[3];
    tx_data[0] = 0xA1;
    tx_data[1] = 0xC3;
    tx_data[2] = _is_reach;
    Serial.write(tx_data,3);

    // _pid->calc((float)_encoder->getCount());
    // Serial.print(_pid->target_now);
    // Serial.print(",");
    // Serial.print(_pid->input_now);
    // Serial.print(",");
    // Serial.println(_pid->output_now);
}

void Motor::clear()
{
    _encoder->clear();
    _pid->clear();
    setPower(0);
}

void Motor::setUnitConvert(float y)
{
    _y_to_mm = y;
}

