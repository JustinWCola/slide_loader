//
// Created by JustinWCola on 2024/3/6.
//
#include <canopen.h>

CANopen myCAN;

uint8_t CANopen::send_msg_buffer[8]={0};
uint8_t CANopen::recv_msg_buffer[8]={0};

uint8_t CANopen::begin(CanBitRate const can_bitrate)
{
    if (!CAN.begin(can_bitrate))
    {
        Serial1.println("CAN init failed.");
        for (;;) {}
    }
    Serial1.println("CAN init ok.");
    return SUCCESS;
}

uint8_t CANopen::read(uint8_t id, uint16_t index, uint8_t sub_index, uint32_t *data)
{
    formMsg(SDO_REQUEST_READ, index, sub_index);
    sendMsg(id + SDO_COMMAND_ID_BASE, 4);

    uint8_t *ptr = (uint8_t*)&data;
    uint8_t len = 0;

    switch (recvMsg())
    {
        case SDO_RESPONSE_READ_8BIT:
            len = 1;
            break;
        case SDO_RESPONSE_READ_16BIT:
            len = 2;
            break;
        case SDO_RESPONSE_READ_32BIT:
            len = 4;
            break;
        default:
            return FAILURE;
    }
    for (uint8_t i=0; i<4; i++) {
        if (i<len) {
            ptr[i] = recv_msg_buffer[4+i]; // fill data bytes
        } else {
            ptr[i] = 0x00; // fill the other bytes with 0
        }
    }
    return SUCCESS;
}

uint8_t CANopen::write(uint8_t id, uint16_t index, uint8_t sub_index, uint32_t *data, uint8_t len)
{
    uint8_t type_byte = 0;
    switch (len)
    {
        case 1:
            type_byte = SDO_REQUEST_WRITE_8BIT;
            break;
        case 2:
            type_byte = SDO_REQUEST_WRITE_16BIT;
            break;
        case 4:
            type_byte = SDO_REQUEST_WRITE_32BIT;
            break;
    }
    formMsg(type_byte,index,sub_index);
    memcpy(send_msg_buffer+4,data,len);
    sendMsg(id + SDO_COMMAND_ID_BASE, 4 + len);
    if(recvMsg() == SDO_RESPONSE_WRITE && recv_msg_buffer[1]==(index&0xFF) \
        && recv_msg_buffer[2]==((index&0xFF00)>>8) \
        && recv_msg_buffer[3]==sub_index)
        return SUCCESS;
    else
        return FAILURE;
}


uint8_t CANopen::formMsg(uint8_t type_byte, uint16_t index, uint8_t sub_index)
{
    send_msg_buffer[0] = type_byte;
    uint8_t *ptr = (uint8_t*)&index;
    send_msg_buffer[1] = ptr[0];
    send_msg_buffer[2] = ptr[1];
    send_msg_buffer[3] = sub_index;
    return SUCCESS;
}

uint8_t CANopen::sendMsg(uint16_t id, uint8_t length)
{
    CanMsg const msg(CanStandardId(id), length, send_msg_buffer);
    return CAN.write(msg);
}

uint8_t CANopen::recvMsg() {
    // wait for message
    uint32_t startTime = millis();
    while(!CAN.available())
        if ((millis()-startTime)>CAN_RECEIVE_TIMEOUT_MS)
            return FAILURE; // time out
    uint8_t len;
    uint32_t can_id;
    CanMsg const msg = CAN.read();
    can_id = msg.getStandardId();
    while (!CAN.available())
    {
        len = msg.data_length;
        memcpy(recv_msg_buffer, msg.data, len);

        Serial1.print(F("Id: "));
        Serial1.println(can_id,HEX);
        Serial1.print(F("data: "));
        for(int i = 0; i<len; i++)
        {
            Serial1.print(recv_msg_buffer[i], HEX);
            Serial1.print(" ");
        }

        // check the type bit, which kind of response it is
        switch (recv_msg_buffer[0]) {
            // requested readings for 8,16,32-bit data
            case SDO_RESPONSE_READ_8BIT:
                return SDO_RESPONSE_READ_8BIT;
            case SDO_RESPONSE_READ_16BIT:
                return SDO_RESPONSE_READ_16BIT;
            case SDO_RESPONSE_READ_32BIT:
                return SDO_RESPONSE_READ_32BIT;
                // confirmation of successful write
            case SDO_RESPONSE_WRITE:
                return SDO_RESPONSE_WRITE;
            case SDO_ERROR_CODE: // fall through error
            case SDO_ERROR_CODE+DEFAULT_NODE_ID:
                Serial1.println(F("\nERROR"));
                Serial1.print(F("Error Message is: "));
                for (uint8_t i=0; i<len; i++)
                    Serial1.print(recv_msg_buffer[i], HEX);
                break;
            case 0:
                //PDO message
                Serial1.println(F("\nPDO Msg received"));
                Serial1.print(F("Message is: "));
                for (uint8_t i=0; i<len; i++)
                    Serial1.print(recv_msg_buffer[i], HEX);
                break; // maybe another msg is waiting
            default:
                break;
        } // switch type_bit
    } // while msg received
    return SUCCESS;
}

uint8_t CANopen::startOperational(uint8_t id)
{
    //switch between pre-operational mode to operational
    send_msg_buffer[0] = 0x01; //NMT Msg
    send_msg_buffer[1] = id;
    sendMsg(0x0000,2);
    while (recvMsg()!=SUCCESS)
        Serial1.println("starting operational.");
    return SUCCESS;
}

uint8_t CANopen::resetNode(uint8_t id)
{
    //rest application on the node
    send_msg_buffer[0] = 0x81; //NMT Msg
    send_msg_buffer[1] = id;
    sendMsg(0x0000,2);
    while (recvMsg()!=SUCCESS)
        Serial1.println("resetting node.");
    return SUCCESS;
}

uint8_t CANopen::sendSyncMsg(uint8_t id)
{
    //synchronous CANopen device
    send_msg_buffer[0] = 0x00; //NMT Msg
    send_msg_buffer[1] = id;
    sendMsg(0x0000,2);
    while (recvMsg()!=SUCCESS)
        Serial1.println("sending sync msg.");
    return SUCCESS;
}
