//
// Created by JustinWCola on 2024/3/18.
//

#ifndef KEY_H
#define KEY_H

#include <Arduino.h>

typedef enum eKeyStatus: uint8_t
{
    None = 0x00,
    Pressed = 0x01,
    Released = 0x02,
};

class Key{
public:
    Key() = default;
    Key(uint8_t pin):_pin(pin){}

    void init();
    PinStatus getKey();

private:
    uint8_t _pin;   //引脚
};

#endif //KEY_H
