//
// Created by JustinWCola on 2024/2/26.
//

#include <Arduino.h>
#include <motor.h>

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