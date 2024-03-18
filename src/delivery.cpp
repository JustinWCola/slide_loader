//
// Created by JustinWCola on 2024/3/12.
//

#include <delivery.h>

void Delivery::init()
{
    _axis_x.init();
    _axis_z.init();
}

bool Delivery::setAbsPoint(int32_t x, int32_t z)
{
    _axis_x.setAbsPosition(x);
    _axis_z.setAbsPosition(z);

    Serial.print("setting abs point:");
    Serial.print(x);
    Serial.print(",");
    Serial.println(z);

    return true;
}

bool Delivery::setRevPoint(int32_t x, int32_t z)
{
    _axis_x.setRevPosition(x);
    _axis_z.setRevPosition(z);

    Serial.print("setting rev point:");
    Serial.print(x);
    Serial.print(",");
    Serial.println(z);

    return true;
}

bool Delivery::getAbsPoint()
{
    _pos_x = _axis_x.getAbsPosition();
    _pos_z = _axis_z.getAbsPosition();

    Serial.print("getting abs point:");
    Serial.print(_pos_x);
    Serial.print(",");
    Serial.println(_pos_z);

    return true;
}
