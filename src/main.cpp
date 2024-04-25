#include <Arduino.h>
#include <encoder.h>
#include <motor.h>
#include <canopen.h>
#include <delivery.h>
#include <led.h>
#include <key.h>
#include <Arduino_FreeRTOS.h>

CANopen CANOPEN;
Delivery delivery(CANOPEN);

Encoder motor_encoder(2,3);
Pid motor_pid(1,0,0.1);
//此处使用指针实例化Motor类的Encoder和Pid成员，因为中断服务函数只能
Motor motor(8,1,9,&motor_encoder,&motor_pid);

Key sw(0);
Key key[4]{14,15,16,17};
Led led[4]{{18,19},{13,12},{11,10},{7,6}};

void TaskSerial(void *param);
void TaskDelivery(void *param);
void TaskLoader(void *param);
void TaskKey(void *param);
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
    //电机位置归零
    while (sw.getKey() != HIGH)
        motor.setPower(-5);
    motor.clear();
    //初始化CAN通信, D4(CANTX0) D5(CANRX0)
    // CANOPEN.begin(CanBitRate::BR_1000k);
    // //初始化伺服电机
    // delivery.init();

    Serial.println("ready");
    delay(2000);

    xTaskCreate(TaskSerial, "Serial", 1024, nullptr, 2, nullptr);
    // xTaskCreate(TaskDelivery, "Delivery", 128, nullptr, 2, nullptr);
    xTaskCreate(TaskLoader, "Loader", 128, nullptr, 1, nullptr);
    // xTaskCreate(TaskKey, "Key", 128, nullptr, 2, nullptr);

    vTaskStartScheduler();
}

void TaskSerial(void *param)
{
    while(1)
    {
        /*
         * byte 0 start 0xA1
         * byte 1 cmd id(delivery/loader/led)
         *
         * delivery abs: 0xB1
         * byte 2-5 delivery x
         * byte 6-9 delivery z
         *
         * delivery rev: 0xB2
         * byte 2-5 delivery x
         * byte 6-9 delivery z
         *
         * loader: 0xB3
         * byte 2-5 loader distance
         *
         * led: 0xB4
         * byte 2 led0
         * byte 3 led1
         * byte 4 led2
         * byte 5 led3
         */
        uint8_t rx_data[8] = {0};
        float x,y,z;
        if(Serial.available() > 0)
        {
            if(Serial.read() == 0xA1)
            {
                switch (Serial.read())
                {
                    case 0xB1:
                        Serial.readBytes(rx_data, 8);
                        memcpy(&x, rx_data, 4);
                        memcpy(&z, rx_data + 4, 4);

                        delivery.setAbsPoint(x, z);
                        break;
                    case 0xB2:
                        Serial.readBytes(rx_data, 8);
                        memcpy(&x, rx_data, 4);
                        memcpy(&z, rx_data + 4, 4);

                        delivery.setRevPoint(x, z);
                        break;
                    case 0xB3:
                        Serial.readBytes(rx_data,4);
                        memcpy(&y, rx_data, 4);

                        motor.setTarget(y);
                        break;
                    case 0xB4:
                        Serial.readBytes(rx_data,4);

                        for (int i = 0; i < 4; i++)
                            if (rx_data[i] != 0)
                                led[i].setColor((eLedColor)rx_data[i]);
                        break;
                    case 0xB5:
                        Serial.readBytes(rx_data,12);
                        memcpy(&x,rx_data, 4);
                        memcpy(&y,rx_data + 4, 4);
                        memcpy(&z,rx_data + 8, 4);
                        
                        delivery.setUnitConvert(x,z);
                        motor.setUnitConvert(y);
                        break;
                }
            }
        }
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
}

void TaskDelivery(void *param)
{
    while(1)
    {
        delivery.update();
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void TaskLoader(void *param)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    while(1)
    {
        motor.update();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5));
    }
}

void TaskKey(void *param)
{
    while(1)
    {
        uint8_t keyPressed[4];
        uint8_t lastKeyPressed[4];
        uint8_t keyStatus[4];
        bool isKeyDetected = false;
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

        if(isKeyDetected)
        {
            uint8_t tx_data[6];
            tx_data[0] = 0xA1;
            tx_data[1] = 0xC4;
            memcpy(tx_data+2,keyStatus,4);
            Serial.write(tx_data,6);
        }

        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void loop()
{

}

