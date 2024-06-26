//
// Created by JustinWCola on 2024/3/6.
//

#ifndef CANOPEN_H
#define CANOPEN_H

#include <Arduino_CAN.h>

#define CAN_RECEIVE_TIMEOUT_MS 100  //CAN接收超时时间

// 服务数据对象(SDO)
// ------------------------------------
#define SDO_COMMAND_ID_BASE 0x600
#define SDO_REPLY_ID_BASE 0x580

// 命令符(Data 0)
//       8Bit  16Bit  24Bit  32Bit
// write 0x2F  0x2B   0x27   0x23
// read  0x4F  0x4B   0x47   0x43

#define SDO_REQUEST_READ 0x40
#define SDO_REQUEST_WRITE_8BIT 0x2F
#define SDO_REQUEST_WRITE_16BIT 0x2B
#define SDO_REQUEST_WRITE_32BIT 0x23

#define SDO_RESPONSE_WRITE 0x60
#define SDO_RESPONSE_READ_8BIT 0x4F
#define SDO_RESPONSE_READ_16BIT 0x4B
#define SDO_RESPONSE_READ_32BIT 0x43

#define SDO_ERROR 0x80

typedef enum eNmtMsg : uint8_t
{
    EnterOperational = 0x01,
    StopNode = 0x02,
    EnterPreOperational = 0x80,
    ResetNode = 0x81,
    ResetCommunication = 0x82,
};

class CANopen
{
public:
    CANopen(){};

    bool begin(const CanBitRate can_bitrate);
    bool read(uint8_t id, uint16_t index, uint8_t sub_index, uint32_t *data);
    bool write(uint8_t id, uint16_t index, uint8_t sub_index, uint8_t data);
    bool write(uint8_t id, uint16_t index, uint8_t sub_index, uint16_t data);
    bool write(uint8_t id, uint16_t index, uint8_t sub_index, uint32_t data);
    bool sendNmt(uint8_t id, eNmtMsg msg);
    void send607060(uint8_t id);

private:
    void formMsg(uint8_t type_byte, uint16_t index, uint8_t sub_index);
    void sendMsg(uint16_t id, uint8_t length);
    uint8_t recvMsg();

    static uint8_t send_msg_buffer[8];
    static uint8_t recv_msg_buffer[8];
};

#endif //CANOPEN_H
