//
// Created by JustinWCola on 2024/3/6.
//
#include <canopen.h>

uint8_t CANopen::send_msg_buffer[8]={0};
uint8_t CANopen::recv_msg_buffer[8]={0};

/**
 * 开启CANopen通信链路
 * @param can_bitrate 波特率
 * @return 开启成功
 */
bool CANopen::begin(const CanBitRate can_bitrate)
{
    while (!CAN.begin(can_bitrate))
    {
        Serial.println("CAN init failed");
        return false;
    }
    Serial.println("CAN init ok");

    sendNmt(0,ResetCommunication);
    sendNmt(0,ResetNode);
    delay(1000);

    return true;
}

/**
 * SDO方式读取CANopen节点数据
 * @param id 节点ID
 * @param index 索引
 * @param sub_index 子索引
 * @param data 数据指针，指向存储读到数据的变量
 * @return 读取成功
 */
bool CANopen::read(uint8_t id, uint16_t index, uint8_t sub_index, uint32_t *data)
{
    formMsg(SDO_REQUEST_READ, index, sub_index);
    sendMsg(id + SDO_COMMAND_ID_BASE, 4);

    uint8_t *ptr = (uint8_t*)data;
    uint8_t len = recvMsg();
    if(len == 0)
        return false;

    for (uint8_t i=0; i<4; i++)
    {
        if (i<len)
            ptr[i] = recv_msg_buffer[4+i]; // fill data bytes
        else
            ptr[i] = 0x00; // fill the other bytes with 0
    }
    return true;
}

/**
 * SDO方式写入CANopen节点数据
 * @param id 节点ID
 * @param index 索引
 * @param sub_index 子索引
 * @param data 数据
 * @return 写入成功
 */
bool CANopen::write(uint8_t id, uint16_t index, uint8_t sub_index, uint8_t data)
{
    formMsg(SDO_REQUEST_WRITE_8BIT,index,sub_index);
    send_msg_buffer[4] = data;
    sendMsg(id + SDO_COMMAND_ID_BASE, 4 + 1);
    if(recvMsg() == SDO_RESPONSE_WRITE \
        && recv_msg_buffer[1]==(index&0xFF) \
        && recv_msg_buffer[2]==((index&0xFF00)>>8) \
        && recv_msg_buffer[3]==sub_index)
        return true;
    else
        return false;
}

bool CANopen::write(uint8_t id, uint16_t index, uint8_t sub_index, uint16_t data)
{
    formMsg(SDO_REQUEST_WRITE_16BIT,index,sub_index);
    send_msg_buffer[4] = (uint8_t)data;
    send_msg_buffer[5] = (uint8_t)(data>>8);
    sendMsg(id + SDO_COMMAND_ID_BASE, 4 + 2);
    if(recvMsg() == SDO_RESPONSE_WRITE \
        && recv_msg_buffer[1]==(index&0xFF) \
        && recv_msg_buffer[2]==((index&0xFF00)>>8) \
        && recv_msg_buffer[3]==sub_index)
        return true;
    else
        return false;
}

bool CANopen::write(uint8_t id, uint16_t index, uint8_t sub_index, uint32_t data)
{
    formMsg(SDO_REQUEST_WRITE_32BIT,index,sub_index);
    send_msg_buffer[4] = (uint8_t)data;
    send_msg_buffer[5] = (uint8_t)(data>>8);
    send_msg_buffer[6] = (uint8_t)(data>>16);
    send_msg_buffer[7] = (uint8_t)(data>>24);
    sendMsg(id + SDO_COMMAND_ID_BASE, 4 + 4);
    if(recvMsg() == SDO_RESPONSE_WRITE \
        && recv_msg_buffer[1]==(index&0xFF) \
        && recv_msg_buffer[2]==((index&0xFF00)>>8) \
        && recv_msg_buffer[3]==sub_index)
        return true;
    else
        return false;
}

void CANopen::send607060(uint8_t id)
{
    formMsg(0x60,0,0);
    sendMsg(id + SDO_COMMAND_ID_BASE, 8);
    formMsg(0x70,0,0);
    sendMsg(id + SDO_COMMAND_ID_BASE, 8);
    formMsg(0x60,0,0);
    sendMsg(id + SDO_COMMAND_ID_BASE, 8);
}

/**
 * 发送NMT命令
 * @param id 节点ID
 * @param msg 命令字
 * @return 发送成功
 */
bool CANopen::sendNmt(uint8_t id, eNmtMsg msg)
{
    send_msg_buffer[0] = msg;
    send_msg_buffer[1] = id;
    sendMsg(0x0000,2);
    if (!recvMsg())
        return false;
    else
        return true;
}

/**
 * 组成CANopen消息
 * @param type_byte 请求命令符
 * @param index 索引
 * @param sub_index 子索引
 */
void CANopen::formMsg(uint8_t type_byte, uint16_t index, uint8_t sub_index)
{
    send_msg_buffer[0] = type_byte;
    uint8_t *ptr = (uint8_t*)&index;
    send_msg_buffer[1] = ptr[0];
    send_msg_buffer[2] = ptr[1];
    send_msg_buffer[3] = sub_index;
}

/**
 * 发送CANopen消息
 * @param id COB-ID
 * @param length 长度
 */
void CANopen::sendMsg(uint16_t id, uint8_t length)
{
    CanMsg const msg(CanStandardId(id), length, send_msg_buffer);
    CAN.write(msg);
}

/**
 * 接收CANopen消息
 * @return 应答长度/命令符
 */
uint8_t CANopen::recvMsg()
{
    uint8_t len, id;
    uint32_t startTime = millis();
    while(!CAN.available())
        if ((millis()-startTime)>CAN_RECEIVE_TIMEOUT_MS)
            return 0; // time out

    CanMsg const msg = CAN.read();
    id = msg.getStandardId() - SDO_REPLY_ID_BASE;
    len = msg.data_length;
    memcpy(recv_msg_buffer, msg.data, len);

    // Serial.print(F("Id: "));
    // Serial.println(id,HEX);
    // Serial.print(F("data: "));
    // for(int i = 0; i<len; i++)
    // {
    //     Serial.print(recv_msg_buffer[i], HEX);
    //     Serial.print(" ");
    // }

    // check the type bit, which kind of response it is
    switch (recv_msg_buffer[0])
    {
        // requested readings for 8,16,32-bit data
        case SDO_RESPONSE_READ_8BIT:
            return 1;
        case SDO_RESPONSE_READ_16BIT:
            return 2;
        case SDO_RESPONSE_READ_32BIT:
            return 4;
            // confirmation of successful write
        case SDO_RESPONSE_WRITE:
            return SDO_RESPONSE_WRITE;
        case SDO_ERROR: // fall through error
            return SDO_ERROR;
                // Serial.println(F("\nERROR"));
                // Serial.print(F("Error Message is: "));
                // for (uint8_t i=0; i<len; i++)
                //     Serial.print(recv_msg_buffer[i], HEX);
        // case 0:
        //     // PDO message
        //     Serial.println(F("\nPDO Msg received"));
        //     Serial.print(F("Message is: "));
        //     for (uint8_t i=0; i<len; i++)
        //         Serial.print(recv_msg_buffer[i], HEX);
        //     break; // maybe another msg is waiting
        default:
            break;
    } // switch type_bit
    return 0;
}
