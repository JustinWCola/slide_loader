#include <Arduino.h>
#include <encoder.h>
#include <motor.h>
#include <canopen.h>
#include <servo.h>
#include <led.h>
#include <key.h>
#include <crc8.h>
#include <Arduino_FreeRTOS.h>

CANopen CANOPEN;
Servo axis_x(CANOPEN,1,5);
Servo axis_z(CANOPEN,2,2);

Encoder motor_encoder(2,3);
Pid motor_pid(1,0,0.1);
Key sw(0);
Motor motor(8,1,9,&motor_encoder,&motor_pid,&sw);

Key key[4]{14,15,16,17};
static bool isKeyDetected;
static uint8_t keyStatus[4];
Led led[4]{{18,19},{13,12},{11,10},{7,6}};

void taskSerial(void *param);
void taskDelivery(void *param);
void taskLoader(void *param);
void taskKey(void *param);
void keyUpdate();
void keySend();
void encoderUpdate()
{
    motor_encoder.update();
}


void setup()
{
//    //初始化LED
//    for(auto &led: led)
//        led.init();
//    //初始化按键
//    for(auto &key: key)
//        key.init();
    //初始化串口, Serial: USB
    Serial.begin(115200);
    delay(1000);
    //初始化限位开关, D0
    sw.init();
    //初始化电机编码器, D2(A相) D3(B相)
    motor_encoder.init();
    attachInterrupt(digitalPinToInterrupt(motor_encoder.getIntPin()),encoderUpdate,RISING);
    //初始化电机PWM, D8(CW) D1(CCW) D9(PWM)
    motor.init();
    //电机归零
    motor.setZeroInit();
    // motor.setPower(1);
    // delay(100);
    // motor.setPower(0);
    delay(3000);
    //初始化CAN通信, D4(CANTX0) D5(CANRX0)
    // CANOPEN.begin(CanBitRate::BR_1000k);
    // //初始化伺服电机
    // axis_x.init();
    // axis_z.init();

    xTaskCreate(taskSerial, "Serial", 1024, nullptr, 2, nullptr);
    // xTaskCreate(taskDelivery, "Delivery", 128, nullptr, 2, nullptr);
    xTaskCreate(taskLoader, "Loader", 256, nullptr, 1, nullptr);
    // xTaskCreate(TaskKey, "Key", 128, nullptr, 2, nullptr);

    vTaskStartScheduler();
}

void taskSerial(void *param)
{
    while(1)
    {
        static uint32_t send_time = 0;
        uint8_t rx_data[8] = {0};
        float x = 0, y = 0, z = 0;
        //协议见README
        if(Serial.available() > 0)
        {
            // 1帧头 + 1命令符 + 4数据 + 1校验 + 1帧尾
            if(Serial.peek() == 0xAA)
            {
                Serial.readBytes(rx_data,8);
                if(rx_data[6] == crc8Check(rx_data,6) && rx_data[7] == 0xFF)
                {
                    switch (rx_data[1])
                    {
                        case 0xB1:
                            memcpy(&x, rx_data + 2, 4);
                            axis_x.setAbsPos(x);
                            break;
                        case 0xB2:
                            memcpy(&z, rx_data + 2, 4);
                            axis_z.setAbsPos(z);
                            break;
                        case 0xB3:
                            memcpy(&y, rx_data + 2, 4);
                            motor.setTarget(y);
                            break;
                        case 0xB4:
                            for (int i = 2; i < 6; i++)
                                if (rx_data[i] != 0)
                                    led[i-2].setColor((eLedColor)rx_data[i]);
                            break;
                        case 0xD1:
                            memcpy(&x,rx_data + 2, 4);
                            axis_x.setUnitConvert(x);
                        case 0xD2:
                            memcpy(&z,rx_data + 2, 4);
                            axis_z.setUnitConvert(z);
                            break;
                        case 0xD3:
                            memcpy(&x,rx_data + 2, 4);
                            motor.setUnitConvert(y);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        Serial.flush();

        // static bool busy_now, busy_last;
        // busy_now = !(axis_x.getReach() && axis_z.getReach() && motor.getReach());
        // if(!busy_now && busy_last)
        // {
        //     uint8_t tx_data[10];
        //     tx_data[0] = 0xAA;
        //     tx_data[1] = 0xC1;
        //     tx_data[2] = 0x00;
        //     tx_data[3] = crc8Check(tx_data,3);
        //     tx_data[4] = 0xFF;
        //     Serial.write(tx_data,5);
        // }
        // busy_last = busy_now;


        // keySend();
        // send_time++;
        vTaskDelay(20/portTICK_PERIOD_MS);
    }
}

void taskDelivery(void *param)
{
    while(1)
    {
        if(motor.getReach())
        {
            axis_x.update();
            axis_z.update();
        }
        axis_x.updateStatus();
        axis_z.updateStatus();
        vTaskDelay(20/portTICK_PERIOD_MS);
    }
}

void taskLoader(void *param)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    while(1)
    {
        // if(delivery.getReach())
            motor.update();
        motor.updateStatus();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));
    }
}

void taskKey(void *param)
{
    while(1)
    {
        keyUpdate();
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
}

void keyUpdate()
{
    uint8_t keyPressed[4];
    uint8_t lastKeyPressed[4];
    isKeyDetected = false;
    for(int i=0;i<4;i++)
    {
        lastKeyPressed[i] = keyPressed[i];
        keyPressed[i] = key[i].getKey();
        if(keyPressed[i] != lastKeyPressed[i])
        {
            isKeyDetected = true;
            if(lastKeyPressed[i] == LOW)
                keyStatus[i] = Pressed;
            else
                keyStatus[i] = Released;
        }
        else
            keyStatus[i] = None;
    }
}

void keySend()
{
    if(isKeyDetected)
    {
        uint8_t tx_data[6];
        tx_data[0] = 0xA1;
        tx_data[1] = 0xC4;
        memcpy(tx_data+2,keyStatus,4);
        Serial.write(tx_data,6);
    }
}

void loop()
{

}

