//
// Created by JustinWCola on 2024/2/26.
//
#include <Arduino.h>
#include <encoder.h>

/**
 * 初始化编码器引脚
 */
void Encoder::init()
{
    pinMode(_a_pin,INPUT);
    pinMode(_b_pin,INPUT);
}

/**
 * 编码器更新函数，需要在中断中调用
 */
void Encoder::update()
{
    if(digitalRead(_b_pin) == HIGH)
        _count++;
    else
        _count--;
}

/**
 * 清除编码器信息
 */
void Encoder::clear()
{
    _count = 0;
}

/**
 * 获取中断引脚位号
 * @return 中断引脚位号
 */
uint8_t Encoder::getIntPin()
{
    return _a_pin;
}

/**
 * 获取脉冲数
 * @return 脉冲数
 */
int32_t Encoder::getCount()
{
    return _count;
}