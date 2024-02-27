//
// Created by JustinWCola on 2024/2/26.
//

#ifndef ENCODER_H
#define ENCODER_H

#define ENCODER_A 2
#define ENCODER_B 3

#define ENCODER_PULSE 10

extern volatile long int encoder_count;

void ENCODER_Init();

#endif //ENCODER_H
