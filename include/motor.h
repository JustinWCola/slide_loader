//
// Created by JustinWCola on 2024/2/26.
//

#ifndef MOTOR_H
#define MOTOR_H

#define MOTOR_DIR 8
#define MOTOR_PWM 9

#define MOTOR_RATIO 105
#define MOTOR_RADIUS 10

void MOTOR_Init();
void MOTOR_SetPower(int power);
void MOTOR_SetTarget(float target);
bool MOTOR_Update();

#endif //MOTOR_H
