//
// Created by JustinWCola on 2024/3/6.
//
#include <canopen.h>

uint8_t CANopen::send_msg_buffer[8]={0};
uint8_t CANopen::recv_msg_buffer[8]={0};

bool CANopen::begin(const CanBitRate can_bitrate)
{
    if (!CAN.begin(can_bitrate))
    {
        Serial.println("CAN init failed");
        for (;;) {}
    }
    Serial.println("CAN init ok");
    return true;
}

bool CANopen::read(uint8_t id, uint16_t index, uint8_t sub_index, uint32_t *data)
{
    formMsg(SDO_REQUEST_READ, index, sub_index);
    sendMsg(id + SDO_COMMAND_ID_BASE, 4);

    uint8_t *ptr = (uint8_t*)data;
    uint8_t len = recvMsg();

    for (uint8_t i=0; i<4; i++)
    {
        if (i<len)
            ptr[i] = recv_msg_buffer[4+i]; // fill data bytes
        else
            ptr[i] = 0x00; // fill the other bytes with 0
    }
    return true;
}

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

bool CANopen::formMsg(uint8_t type_byte, uint16_t index, uint8_t sub_index)
{
    send_msg_buffer[0] = type_byte;
    uint8_t *ptr = (uint8_t*)&index;
    send_msg_buffer[1] = ptr[0];
    send_msg_buffer[2] = ptr[1];
    send_msg_buffer[3] = sub_index;
    return true;
}

bool CANopen::sendMsg(uint16_t id, uint8_t length)
{
    CanMsg const msg(CanStandardId(id), length, send_msg_buffer);
    CAN.write(msg);
    return true;
}

uint8_t CANopen::recvMsg() {
    // wait for message
    uint32_t startTime = millis();
    while(!CAN.available())
        if ((millis()-startTime)>CAN_RECEIVE_TIMEOUT_MS)
            return 0; // time out
    uint8_t len;
    uint8_t id;
    CanMsg const msg = CAN.read();
    can_id = msg.getStandardId();
    id = can_id - SDO_REPLY_ID_BASE;
    while (!CAN.available())
    {
        len = msg.data_length;
        memcpy(recv_msg_buffer, msg.data, len);

//        Serial.print(F("Id: "));
//        Serial.println(can_id,HEX);
//        Serial.print(F("data: "));
//        for(int i = 0; i<len; i++)
//        {
//            Serial.print(recv_msg_buffer[i], HEX);
//            Serial.print(" ");
//        }

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
            case SDO_ERROR_CODE: // fall through error
                return SDO_ERROR_CODE;
//                Serial.println(F("\nERROR"));
//                Serial.print(F("Error Message is: "));
//                for (uint8_t i=0; i<len; i++)
//                    Serial.print(recv_msg_buffer[i], HEX);
            case 0:
                //PDO message
//                Serial.println(F("\nPDO Msg received"));
//                Serial.print(F("Message is: "));
//                for (uint8_t i=0; i<len; i++)
//                    Serial.print(recv_msg_buffer[i], HEX);
                break; // maybe another msg is waiting
            default:
                break;
        } // switch type_bit
    } // while msg received
    return 0;
}

bool CANopen::startOperational(uint8_t id)
{
    //进入操作状态指令
    send_msg_buffer[0] = 0x01; //NMT Msg
    send_msg_buffer[1] = id;
    sendMsg(0x000,2);
    while (!recvMsg())
        Serial.println("starting operational");
    return true;
}

bool CANopen::resetNode(uint8_t id)
{
    //rest application on the node
    send_msg_buffer[0] = 0x81; //NMT Msg
    send_msg_buffer[1] = id;
    sendMsg(0x0000,2);
    while (!recvMsg())
        Serial.println("resetting node");
    return true;
}

bool CANopen::sendSyncMsg(uint8_t id)
{
    //synchronous CANopen device
    send_msg_buffer[0] = 0x00; //NMT Msg
    send_msg_buffer[1] = id;
    sendMsg(0x0000,2);
    while (!recvMsg())
        Serial.println("sending sync msg");
    return true;
}
