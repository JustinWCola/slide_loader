//
// Created by JustinWCola on 2024/3/18.
//
#include <led.h>

/**
 * 初始化LED灯
 */
void Led::init()
{
    pinMode(_rPin,PinMode::OUTPUT);
    pinMode(_gPin,PinMode::OUTPUT);
}

/**
 * 设置LED灯的颜色
 * @param color 颜色
 */
void Led::setColor(eLedColor color)
{
    // 巧妙！红色和绿色，3/4实现了 无/红/绿/黄
    digitalWrite(_rPin,(color&0x01));
    digitalWrite(_gPin,(color&0x10));
}