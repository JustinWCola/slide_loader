//
// Created by JustinWCola on 2024/3/12.
//

#include <delivery.h>
#include <crc8.h>

/**
 * 初始化传送机构
 */
void Delivery::init()
{
    _axis_x.init();
    // _axis_x.setZero();
    // _axis_x.init();
    delay(1000);
    _axis_z.init();
    // _axis_z.setZero();
    // _axis_z.init();
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

    // Serial.print("setting abs point:");
    // Serial.print(x);
    // Serial.print(",");
    // Serial.println(z);
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

    // Serial.print("setting rev point:");
    // Serial.print(x);
    // Serial.print(",");
    // Serial.println(z);
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
 * 获取传送机构绝对位置
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
}

/**
 * 传送机构更新函数，需要在传送机构任务循环中调用
 */
void Delivery::update()
{
    _axis_x.setAbsPosition((int32_t)(LIMIT(_x_tar_pos, 310, 0) / _x_to_mm));
    _axis_z.setAbsPosition((int32_t)(LIMIT(_z_tar_pos, 136, 0) / _z_to_mm));

    // getAbsPoint();
}

/**
 * 更新传送机构状态
 */
void Delivery::updateStatus()
{
    //获取到达状态
    if(_axis_x.getReach() && _axis_z.getReach())
        //计时到达时间，消抖10*20=200ms
        if(_reach_time > 10)
            _is_reach = true;
        else
            _reach_time++;
    else
    {
        //清空计时
        _is_reach = false;  //只在这一个地方清零
        _reach_time = 0;
    }
}

/**
 * 获取传送机构到达状态
 * @return 是否达到
 */
bool Delivery::getReach()
{
    return _is_reach;
}

/**
 * 发送传送机构位置与到达报文，需要在串口任务循环中调用，注意分时
 */
void Delivery::send()
{
    uint8_t tx_data[10] = {0};
    static bool last_reach;

    //检测到达标志位的上升沿，只在到达后发送一次消息
    if(_is_reach && !last_reach)
    {
        tx_data[0] = 0xAA;
        tx_data[1] = 0xC2;
        tx_data[2] = _is_reach;
        tx_data[3] = crc8Check(tx_data,3);
        tx_data[4] = 0xFF;
        Serial.write(tx_data,5);
    }
    // else
    // {
    //     tx_data[0] = 0xA1;
    //     tx_data[1] = 0xC1;
    //     memcpy(tx_data+2,&_x_now_pos,4);
    //     memcpy(tx_data+6,&_z_now_pos,4);
    //     Serial.write(tx_data,10);
    // }
    last_reach = _is_reach;
}
