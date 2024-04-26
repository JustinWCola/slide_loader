//
// Created by JustinWCola on 2024/2/26.
//

#ifndef ENCODER_H
#define ENCODER_H

class Encoder
{
public:
    Encoder() = default;
    Encoder(uint8_t a_pin, uint8_t b_pin):
    _a_pin(a_pin), _b_pin(b_pin){}

    void init();
    void update();
    void clear();
    uint8_t getIntPin();
    int32_t getCount();
private:
    uint8_t _a_pin;     //A相引脚
    uint8_t _b_pin;     //B相引脚
    int32_t _count = 0; //脉冲数
};

#endif //ENCODER_H
