//
// Created by JustinWCola on 2024/2/26.
//

#include <Arduino.h>
#include <motor.h>
#include <pid.h>
#include <encoder.h>

Pid motor_pid(5, 0, 0.5);
bool isReachTarget = false;
float y_to_mm = 2 * (float)PI * MOTOR_RADIUS / MOTOR_RATIO / ENCODER_PULSE;
float y_tar_pos = 0;

void MOTOR_Init()
{
    pinMode(MOTOR_CW,OUTPUT);
    pinMode(MOTOR_CCW,OUTPUT);
    pinMode(MOTOR_PWM,OUTPUT);
}

void MOTOR_Clear()
{
    encoder_count = 0;
    motor_pid.clear();
    MOTOR_SetPower(0);
}

void MOTOR_SetPower(int power)
{
    if (power > 0)
    {
        digitalWrite(MOTOR_CW, LOW);
        digitalWrite(MOTOR_CCW, HIGH);
        analogWrite(MOTOR_PWM, power);
    }
    else
    {
        digitalWrite(MOTOR_CW, HIGH);
        digitalWrite(MOTOR_CCW, LOW);
        analogWrite(MOTOR_PWM, power);
    }
}

void MOTOR_SetTarget(float target)
{
    y_tar_pos = target;
//    motor_pid.setTarget(target / y_to_mm);
//    isReachTarget = false;
}

bool MOTOR_Update()
{
    motor_pid.setTarget(y_tar_pos / y_to_mm);
    MOTOR_SetPower((int)motor_pid.calc((float)encoder_count));
//    if (abs(motor_pid.input_now - motor_pid.output_now) < 5.0f && !isReachTarget)
//    {
//        isReachTarget = true;
//        return true;
//    }
//    else
//        return false;
//    Serial.print(motor_pid.target_now);
//    Serial.print(",");
//    Serial.print(motor_pid.input_now);
//    Serial.print(",");
//    Serial.println(motor_pid.output_now);
}

bool MOTOR_SetUnitConvert(float y)
{
    y_to_mm = y;
}

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
    _pid.setTarget(target / _y_to_mm);
}

void Motor::update()
{
    setPower((int)_pid.calc((float)encoder_count));
    if (abs(_pid.input_now - _pid.target_now) < 5.0f)
        _is_reach = true;
    else
        _is_reach = false;

    uint8_t tx_data[3];
    tx_data[0] = 0xA1;
    tx_data[1] = 0xC3;
    tx_data[2] = _is_reach;
    Serial.write(tx_data,3);
}

void Motor::clear()
{
    encoder_count = 0;
    _pid.clear();
    setPower(0);
}

void Motor::setUnitConvert(float y)
{
    _y_to_mm = y;
}
