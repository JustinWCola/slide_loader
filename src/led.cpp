//
// Created by JustinWCola on 2024/3/18.
//
#include <led.h>

void Led::init()
{
    pinMode(_rPin,PinMode::OUTPUT);
    pinMode(_gPin,PinMode::OUTPUT);
}

void Led::setColor(eLedColor color)
{
    digitalWrite(_rPin,(color&0x01));
    digitalWrite(_gPin,(color&0x10));
}