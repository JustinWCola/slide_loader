//
// Created by JustinWCola on 2024/2/24.
//

#ifndef CAN_H
#define CAN_H

#include "Arduino_CAN.h"

void CAN_Init();
CanMsg CAN_Read();
void CAN_Send(uint8_t *msg_data);

#endif //CAN_H
