//
// Created by JustinWCola on 2024/2/26.
//
#include <Arduino.h>
#include <encoder.h>

void Encoder::init()
{
    pinMode(_a_pin,INPUT);
    pinMode(_b_pin,INPUT);
}

void Encoder::update()
{
    if(digitalRead(_b_pin) == HIGH)
        _count++;
    else
        _count--;
}

void Encoder::clear()
{
    _count = 0;
}

uint8_t Encoder::getIntPin()
{
    return _a_pin;
}

int32_t Encoder::getCount()
{
    return _count;
}