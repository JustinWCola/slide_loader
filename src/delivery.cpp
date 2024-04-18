//
// Created by JustinWCola on 2024/3/12.
//

#include <delivery.h>

void Delivery::init()
{
    _axis_x.init();
    _axis_z.init();
}

bool Delivery::setAbsPoint(float x, float z)
{
    _x_tar_pos = x;
    _z_tar_pos = z;

    Serial.print("setting abs point:");
    Serial.print(x);
    Serial.print(",");
    Serial.println(z);

    return true;
}

bool Delivery::setRevPoint(float x, float z)
{
    _x_tar_pos += x;
    _z_tar_pos += z;

    Serial.print("setting rev point:");
    Serial.print(x);
    Serial.print(",");
    Serial.println(z);

    return true;
}

bool Delivery::setUnitConvert(float x, float z)
{
    _x_to_mm = x;
    _z_to_mm = z;
    return true;
}

bool Delivery::getAbsPoint()
{
    _x_now_pos = (float)_axis_x.getAbsPosition() * _x_to_mm;
    _z_now_pos = (float)_axis_z.getAbsPosition() * _z_to_mm;

//    Serial.print("getting abs point:");
//    Serial.print(_pos_x);
//    Serial.print(",");
//    Serial.println(_pos_z);

    _is_reach = _axis_x.getReach() && _axis_z.getReach();
}

bool Delivery::update()
{
    _axis_x.setAbsPosition((int32_t)(LIMIT(_x_tar_pos, 310, 0) / _x_to_mm));
    _axis_z.setAbsPosition((int32_t)(LIMIT(_z_tar_pos, 136, 0) / _z_to_mm));

    getAbsPoint();

    uint8_t tx_data[10];
    tx_data[0] = 0xA1;
    tx_data[1] = 0xC2;
    tx_data[2] = _is_reach;
    Serial.write(tx_data,3);

    tx_data[0] = 0xA1;
    tx_data[1] = 0xC1;
    memcpy(tx_data+2,&_x_now_pos,4);
    memcpy(tx_data+6,&_z_now_pos,4);
    Serial.write(tx_data,10);
}