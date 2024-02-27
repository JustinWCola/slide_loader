//
// Created by JustinWCola on 2024/2/26.
//

#ifndef MOTOR_H
#define MOTOR_H

#define MOTOR_CW 7
#define MOTOR_CCW 8
#define MOTOR_PWM 9

#define MOTOR_RATIO 150

void MOTOR_Init();
void MOTOR_SetPower(int power);
void MOTOR_Update(float target);

#endif //MOTOR_H
