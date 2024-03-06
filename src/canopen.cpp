//
// Created by JustinWCola on 2024/3/6.
//
#include <canopen.h>

CANopen myCAN;

uint8_t CANopen::can_msg_buffer[8]={0};
uint8_t CANopen::can_receive_buffer[8]={0};

void CANopen::begin()
{
    if (!CAN.begin(CanBitRate::BR_1000k))
    {
        Serial1.println("CAN init failed.");
        for (;;) {}
    }
    Serial1.println("CAN init ok.");
}

uint8_t CANopen::read(uint8_t id, uint16_t index, uint8_t sub_index, uint8_t *data)
{
    composeMsg(SDO_REQUEST_READ,index,sub_index);

    CanMsg const tx_msg(CanStandardId(id+SDO_COMMAND_ID_BASE), 4, can_msg_buffer);
    CAN.write(tx_msg);

    CanMsg const rx_msg = CAN.read();

    sendCanBuffer(id+SDO_COMMAND_ID_BASE,4);
    if (receiveCanMsg()==SDO_RESPONSE_READ_8BIT) {
        *data = can_receive_buffer[4]; // first data byte
        return SUCCESS;
    } else {
        return FAILURE; // requested data not received
    }
}

void CANopen::composeMsg(uint8_t type_byte, uint16_t index, uint8_t sub_index)
{
    can_msg_buffer[0] = type_byte;
    uint8_t *ptr = (uint8_t*)&index;
    can_msg_buffer[1] = ptr[0];
    can_msg_buffer[2] = ptr[1];
    can_msg_buffer[3] = sub_index;
}

uint8_t CANopen::sendCanBuffer(uint16_t id, uint8_t length)
{
    CanMsg const msg(CanStandardId(id), length, can_msg_buffer);
    return CAN.write(msg);
}

uint8_t CANopen::receiveCanMsg() {
    // wait for message
    uint32_t startTime = millis();
    while(!CAN.available()){
        if ((millis()-startTime)>CAN_RECEIVE_TIMEOUT_MS) {
            return FAILURE; // timed out
        }
    }
    uint8_t length;
    uint32_t can_id;
    CanMsg const msg = CAN.read();
    can_id = msg.getStandardId();
    while (!CAN.available())
    {
        length = msg.data_length;
        memcpy(can_receive_buffer,msg.data,length);

        Serial1.print(F("Id: "));
        Serial1.println(can_id,HEX);
        Serial1.print(F("data: "));
        for(int i = 0; i<length; i++)
        {
            Serial1.print(can_receive_buffer[i],HEX);
            Serial1.print(" ");
        }

        // check the type bit, which kind of response it is
        switch (can_receive_buffer[0]) {
            // requested readings for 8,16,32 bit data
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
                for (uint8_t i=0; i<length; i++) {
                    Serial1.print(can_receive_buffer[i],HEX);
                }
                break;
            case 0:
                //PDO message
                Serial1.println(F("\nPDO Msg received"));
                Serial1.print(F("Message is: "));
                for (uint8_t i=0; i<length; i++) {
                    Serial1.print(can_receive_buffer[i],HEX);
                }
                break; // maybe another msg is waiting
            default:
                break;
        } // switch type_bit
    } // while msg received
}

