//
// Created by JustinWCola on 2024/3/18.
//

#include <key.h>

/**
 * 初始化按键引脚
 */
void Key::init()
{
    pinMode(_pin, INPUT);
}

/**
 * 获取按键键值
 * @return 键值
 */
PinStatus Key::getKey()
{
    return digitalRead(_pin);
}