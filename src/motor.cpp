//
// Created by JustinWCola on 2024/2/26.
//

#include <Arduino.h>
#include <motor.h>
#include <pid.h>
#include "encoder.h"

/* TB6612驱动接线:
 * AIN1-CW
 * AIN2-CCW
 * PWMA-PWM
 * STBY-5V
 * GND-GND
 * AOUT1-M1
 * AOUT2-M2
 * */

Pid motor_pid(1, 0, 0.5);

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

void MOTOR_Update(float target)
{
    MOTOR_SetPower((int)motor_pid.calc(target, (float)encoder_count));
    Serial1.print(motor_pid.target_now);
    Serial1.print(",");
    Serial1.print(motor_pid.input_now);
    Serial1.print(",");
    Serial1.println(motor_pid.output_now);
}
