//
// Created by JustinWCola on 2024/2/26.
//

#ifndef MOTOR_H
#define MOTOR_H

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

#endif //MOTOR_H
