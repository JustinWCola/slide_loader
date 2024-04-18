//
// Created by JustinWCola on 2024/2/26.
//

#ifndef ENCODER_H
#define ENCODER_H

class Encoder
{
public:
    Encoder(uint8_t a_pin, uint8_t b_pin):
    _a_pin(a_pin), _b_pin(b_pin){}

    void init();
    void update();
    void clear();
    uint8_t getIntPin();
    int32_t getCount();
private:
    uint8_t _a_pin;
    uint8_t _b_pin;
    int32_t _count = 0;
};

#endif //ENCODER_H
