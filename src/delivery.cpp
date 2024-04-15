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
    _axis_x.setAbsPosition((int32_t)(x * _x_to_mm));
    _axis_z.setAbsPosition((int32_t)(z * _z_to_mm));

    Serial.print("setting abs point:");
    Serial.print(x);
    Serial.print(",");
    Serial.println(z);

    return true;
}

bool Delivery::setRevPoint(float x, float z)
{
    _axis_x.setRevPosition((int32_t)(x * _x_to_mm));
    _axis_z.setRevPosition((int32_t)(z * _z_to_mm));

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
    _pos_x = (float)_axis_x.getAbsPosition() / _x_to_mm;
    _pos_z = (float)_axis_z.getAbsPosition() / _z_to_mm;

    uint8_t tx_data[10];
    tx_data[0] = 0xA1;
    tx_data[1] = 0xC1;
    memcpy(tx_data+2,&_pos_x,4);
    memcpy(tx_data+6,&_pos_z,4);
    Serial.write(tx_data,10);
//    Serial.print("getting abs point:");
//    Serial.print(_pos_x);
//    Serial.print(",");
//    Serial.println(_pos_z);

    return true;
}


bool Delivery::getReach()
{
    if(_axis_x.getReach() && _axis_z.getReach())
    {
        uint8_t tx_data[3];
        tx_data[0] = 0xA1;
        tx_data[1] = 0xC2;
        tx_data[2] = 0x01;
        Serial.write(tx_data,10);

        return true;
    }
    else
        return false;
}