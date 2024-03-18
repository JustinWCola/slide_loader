//
// Created by JustinWCola on 2024/3/18.
//

#ifndef KEY_H
#define KEY_H

#include <Arduino.h>

class Key{
public:
    Key(uint8_t pin):_pin(pin){}

    void init();
    PinStatus getKey();

private:
    uint8_t _pin;
};

#endif //KEY_H
