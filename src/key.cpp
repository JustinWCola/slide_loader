//
// Created by JustinWCola on 2024/3/18.
//

#include <key.h>

void Key::init()
{
    pinMode(_pin, INPUT);
}

PinStatus Key::getKey()
{
    return digitalRead(_pin);
}