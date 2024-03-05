//
// Created by JustinWCola on 2024/2/29.
//
#include <can.h>
#include <servo.h>

void SERVO_Test()
{
    int32_t pos = 5000;

    uint8_t msg_data[8];
    msg_data[0] = pos;
    msg_data[1] = (pos >> 8);
    msg_data[2] = (pos >> 16);
    msg_data[3] = (pos >> 24);

    CanMsg const msg(CanStandardId(0x301), sizeof(msg_data), msg_data);
    CAN.write(msg);
}

void SERVO_Send(uint8_t id, uint8_t cmd, uint8_t index, uint8_t sub_index, uint32_t data)
{
    uint8_t msg_data[8];
    msg_data[0] = cmd;
    msg_data[1] = index;
    msg_data[2] = (index >> 8);
    msg_data[3] = sub_index;
    memcpy(msg_data + 4, &data, 4);

    CanMsg const msg(CanStandardId(0x600+id), sizeof(msg_data), msg_data);
    CAN.write(msg);
}

void SERVO_SetCtrlMode(uint8_t id, uint32_t ctrl_mode)
{
    SERVO_Send(id, 0x2B, 0x2002, 0x01, ctrl_mode);
}

void SERVO_SetMoveMode(uint8_t id, uint32_t move_mode)
{
    SERVO_Send(id, 0x2F, 0x6060, 0x01, move_mode);
}

void SERVO_SetPos(uint8_t id, uint32_t pos)
{
    SERVO_Send(id, 0x23, 0x607A, 0x00, pos);
}

void SERVO_SetVel(uint8_t id, uint32_t vel)
{
    SERVO_Send(id, 0x23, 0x6081, 0x00, vel);
}

void SERVO_SetDcc(uint8_t id, uint32_t dcc)
{
    SERVO_Send(id, 0x23, 0x6083, 0x00, dcc);
}

void SERVO_SetAcc(uint8_t id, uint32_t acc)
{
    SERVO_Send(id, 0x23, 0x6084, 0x00, acc);
}

void SERVO_Enable(uint8_t id)
{
    SERVO_Send(id, 0x2B, 0x6040, 0x00, 0X06);
    SERVO_Send(id, 0x2B, 0x6040, 0x00, 0X07);
    SERVO_Send(id, 0x2B, 0x6040, 0x00, 0X0F);
}

void SERVO_SetRunMode(uint8_t id, uint32_t run_mode)
{
    SERVO_Send(id, 0x2B, 0x6040, 0x00, run_mode);
}