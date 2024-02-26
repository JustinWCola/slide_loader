//
// Created by JustinWCola on 2024/2/26.
//
#include <Arduino.h>
#include <encoder.h>

volatile long int encoder_count;

void ENCODER_Update()
{
    if(digitalRead(ENCODER_B) == HIGH)
        encoder_count++;
    else
        encoder_count--;
}

void ENCODER_Init()
{
    pinMode(ENCODER_A,INPUT);
    pinMode(ENCODER_B,INPUT);
    attachInterrupt(digitalPinToInterrupt(ENCODER_A), ENCODER_Update, RISING);
}
