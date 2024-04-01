//
// Created by JustinWCola on 2024/2/26.
//

#include <Arduino.h>
#include <motor.h>
#include <pid.h>
#include "encoder.h"

Pid motor_pid(1, 0, 0.5);
bool isReachTarget = false;

void MOTOR_Init()
{
    pinMode(MOTOR_CW,OUTPUT);
    pinMode(MOTOR_CCW,OUTPUT);
    pinMode(MOTOR_PWM,OUTPUT);
}

void MOTOR_SetPower(int power)
{
    if (power > 0)
    {
        digitalWrite(MOTOR_CW, HIGH);
        digitalWrite(MOTOR_CCW, LOW);
        analogWrite(MOTOR_PWM, power);
    }
    else if (power < 0)
    {
        digitalWrite(MOTOR_CW, LOW);
        digitalWrite(MOTOR_CCW, HIGH);
        analogWrite(MOTOR_PWM, power);
    }
    else
    {
        digitalWrite(MOTOR_CW, LOW);
        digitalWrite(MOTOR_CCW, LOW);
    }
}

void MOTOR_SetTarget(float target)
{
    motor_pid.setTarget(target);
    isReachTarget = false;
}

bool MOTOR_Update()
{
    MOTOR_SetPower((int)motor_pid.calc((float)encoder_count));
    if (abs(motor_pid.input_now - motor_pid.output_now) < 0.1f && !isReachTarget)
    {
        isReachTarget = true;
        return true;
    }
    else
        return false;
//    Serial.print(motor_pid.target_now);
//    Serial.print(",");
//    Serial.print(motor_pid.input_now);
//    Serial.print(",");
//    Serial.println(motor_pid.output_now);
}
