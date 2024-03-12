//
// Created by JustinWCola on 2024/3/12.
//

#include <delivery.h>

void Delivery::init()
{
    _axis_x.init();
    _axis_z.init();
}

bool Delivery::setPoint(int32_t x, int32_t z)
{
    _axis_x.setPoint(x);
    _axis_z.setPoint(z);

    Serial1.print("setting point:");
    Serial1.print(x);
    Serial1.print(",");
    Serial1.println(z);

    return true;
}

