//
// Created by JustinWCola on 2024/3/18.
//

#ifndef RGB_H
#define RGB_H

#include <Arduino.h>

typedef enum eLedColor : uint8_t{
    Red = 0x01,
    Green = 0x10,
    Yellow = 0x11,
};

class Led{
public:
    Led(uint8_t rPin,uint8_t gPin):_rPin(rPin),_gPin(gPin){}

    void init();
    void setColor(eLedColor color);
private:
    uint8_t _rPin;  //红灯引脚
    uint8_t _gPin;  //绿灯引脚
};


#endif //RGB_H
