//
// Created by JustinWCola on 2024/3/12.
//

#include <delivery.h>

/**
 * 初始化传送机构
 */
void Delivery::init()
{
    _axis_x.init();
    _axis_z.init();
}

/**
 * 设置传送机构绝对位置
 * @param x X轴绝对位置
 * @param z Z轴绝对位置
 */
void Delivery::setAbsPoint(float x, float z)
{
    _x_tar_pos = x;
    _z_tar_pos = z;

    Serial.print("setting abs point:");
    Serial.print(x);
    Serial.print(",");
    Serial.println(z);
}

/**
 * 设置传送机构相对位置
 * @param x X轴相对位置
 * @param z Z轴相对位置
 */
void Delivery::setRevPoint(float x, float z)
{
    _x_tar_pos += x;
    _z_tar_pos += z;

    Serial.print("setting rev point:");
    Serial.print(x);
    Serial.print(",");
    Serial.println(z);
}

/**
 * 设置单位转换
 * @param x X轴单位转换率
 * @param z Z轴单位转换率
 */
void Delivery::setUnitConvert(float x, float z)
{
    _x_to_mm = x;
    _z_to_mm = z;
}

/**
 * 获取传送机构绝对位置以及到达状态
 */
void Delivery::getAbsPoint()
{
    //获取绝对位置并进行单位转换
    _x_now_pos = (float)_axis_x.getAbsPosition() * _x_to_mm;
    _z_now_pos = (float)_axis_z.getAbsPosition() * _z_to_mm;

//    Serial.print("getting abs point:");
//    Serial.print(_pos_x);
//    Serial.print(",");
//    Serial.println(_pos_z);

    //获取到达状态
    if(_axis_x.getReach() && _axis_z.getReach())
        if(_reach_time > 5)
            _is_reach = true;
        else
            _reach_time++;
    else
    {
        _is_reach = false;
        _reach_time = 0;
    }
}

/**
 * 传送机构更新函数，需要在传送机构任务循环中调用
 */
void Delivery::update()
{
    _axis_x.setAbsPosition((int32_t)(LIMIT(_x_tar_pos, 310, 0) / _x_to_mm));
    _axis_z.setAbsPosition((int32_t)(LIMIT(_z_tar_pos, 136, 0) / _z_to_mm));

    getAbsPoint();
}

/**
 * 发送传送机构位置与到达报文，需要在串口任务循环中调用，注意分时
 */
void Delivery::send()
{
    static uint32_t send_time = 0;
    static uint8_t tx_data[10];

    if(send_time % 2 == 0)
    {
        tx_data[0] = 0xA1;
        tx_data[1] = 0xC2;
        tx_data[2] = _is_reach;
        Serial.write(tx_data,3);
    }
    else
    {
        tx_data[0] = 0xA1;
        tx_data[1] = 0xC1;
        memcpy(tx_data+2,&_x_now_pos,4);
        memcpy(tx_data+6,&_z_now_pos,4);
        Serial.write(tx_data,10);
    }
    send_time++;
}
