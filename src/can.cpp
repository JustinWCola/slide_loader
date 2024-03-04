//
// Created by JustinWCola on 2024/2/24.
//
#include <can.h>

static uint32_t const CAN_ID = 0x20;

void CAN_Init()
{
    if (!CAN.begin(CanBitRate::BR_1000k))
    {
        Serial1.println("CAN.begin(...) failed.");
        for (;;) {}
    }
}

CanMsg CAN_Read()
{
    if (CAN.available())
    {
        CanMsg const msg = CAN.read();
        Serial1.println(msg);
        return msg;
    }
}

static uint32_t msg_cnt = 0;

void CAN_Send(uint8_t msg_data[])
{
    /* Assemble a CAN message with the format of
   * 0xCA 0xFE 0x00 0x00 [4 byte message counter]
   */
//    uint8_t const msg_data[] = {0xCA,0xFE,0,0,0,0,0,0};
    memcpy((void *)(msg_data + 4), &msg_cnt, sizeof(msg_cnt));
    CanMsg const msg(CanStandardId(CAN_ID), sizeof(msg_data), msg_data);

    /* Transmit the CAN message, capture and display an
     * error core in case of failure.
     */
    if (int const rc = CAN.write(msg); rc < 0)
    {
        Serial1.print("CAN.write(...) failed with error code ");
        Serial1.println(rc);
        for (;;) { }
    }

    /* Increase the message counter. */
    msg_cnt++;

    /* Only send one message per second. */
    delay(1000);
}