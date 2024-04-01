#include <Arduino.h>
#include <encoder.h>
#include <motor.h>
#include <canopen.h>
#include <delivery.h>
#include <led.h>
#include <key.h>
#include <queue>
#include <Arduino_FreeRTOS.h>

#define X_GAP 1000  //X轴间距
#define Z_GAP 100   //Z轴间距

#define Z_LIFT 50   //Z轴升降距离
#define Y_DISTANCE 1000    //Y轴距离

CANopen CANOPEN;
Delivery delivery(CANOPEN);
Led led[4]{{18,19},{12,11},{7,6},{5,4}};
Key key[4]{14,15,16,17};
std::queue<uint8_t> id_queue;
bool isVIP = false;
float y_pos = 0;

//void keyDetect()
//{
//    if(key[0].getKey() == RISING)
//        isVIP = true;
//    for(int i=1;i<4;i++)
//    {
//        if(key[i].getKey()== RISING)
//        {
//            id_queue.push(i);
//            led[i].setColor(Red);
//        }
//    }
//}
//
//void takeSlide()
//{
//    MOTOR_Update(Y_DISTANCE);//执行机构伸出
//    delivery.setRevPoint(0, Z_LIFT);//向上
//    MOTOR_Update(0);//执行机构收回
//}
//
//void giveSlide()
//{
//    MOTOR_Update(Y_DISTANCE);//执行机构伸出
//    delivery.setRevPoint(0,-Z_LIFT);//向下
//    MOTOR_Update(0);//执行机构收回
//}
//
//void startLoader(uint8_t id)
//{
//    led[id].setColor(Yellow);//开始装载，亮黄灯
//    delivery.setAbsPoint(X_GAP * id + 0, 0);
//    for(int i=0;i<24;i++)
//    {
//        takeSlide();//从装载仓取出
//        delivery.setAbsPoint(20000,0);//运送到载物台
//        giveSlide();//放入载物台
//        while(!Serial.find("1"))
//            Serial.println("wait for scanning");//等待扫描
//        takeSlide();//从载物台取出
//        delivery.setAbsPoint(X_GAP * id + 0, Z_GAP * (i-1));//运送回下一层
//        giveSlide();//放入装载仓
//        delivery.setRevPoint(0,Z_GAP-Z_LIFT);//到下一个位置
//        Serial.print("complete scanning:");
//        Serial.print(id);
//        Serial.print(",");
//        Serial.println(i);
//    }
//    led[id].setColor(Green);//完成装载，亮绿灯
//}
//
//void selectLoader()
//{
//    if(isVIP)
//        startLoader(0);
//    if(!id_queue.empty())
//    {
//        startLoader(id_queue.front());
//        id_queue.pop();
//    }
//}

void TaskSerial(void *param);
void TaskDelivery(void *param);
void TaskLoader(void *param);
void TaskKey(void *param);

void setup()
{
    //初始化LED
    for(auto &led: led)
        led.init();
    //初始化按键
    for(auto &key: key)
        key.init();
    //初始化串口, Serial: USB
    Serial.begin(115200);
    //初始化CAN通信, CAN Transceiver: D13(CANRX0) D10(CANTX0)
//    CANOPEN.begin(CanBitRate::BR_1000k);
//    //初始化伺服电机
//    delivery.init();
    //初始化电机编码器, D2(A相) D3(B相)
//    ENCODER_Init();
    //初始化电机PWM, D8(CW) D9(PWM)
//    MOTOR_Init();
//    led[0].setColor(Yellow);

    xTaskCreate(TaskSerial, "Serial", 128, nullptr, 1, nullptr);
//    xTaskCreate(TaskDelivery, "Delivery", 128, nullptr, 1, nullptr);
//    xTaskCreate(TaskLoader, "Loader", 128, nullptr, 1, nullptr);
    xTaskCreate(TaskKey, "Key", 128, nullptr, 1, nullptr);

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
                        memcpy(&x,rx_data, 4);
                        memcpy(&z,rx_data + 4, 4);

                        delivery.setAbsPoint(x, z);
                        break;
                    case 0xB2:
                        Serial.readBytes(rx_data, 8);
                        memcpy(&x,rx_data, 4);
                        memcpy(&z,rx_data + 4, 4);

                        delivery.setRevPoint(x, z);
                        break;
                    case 0xB3:
                        Serial.readBytes(rx_data,4);
                        memcpy(&y, rx_data, 4);

                        MOTOR_SetTarget(y);
                        break;
                    case 0xB4:
                        Serial.readBytes(rx_data,4);

                        for (int i = 0; i < 4; i++)
                            if (rx_data[i] != 0)
                                led[i].setColor((eLedColor)rx_data[i]);
                        break;
                }
            }
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void TaskDelivery(void *param)
{
    while(1)
    {
        if(!delivery.getReach())
            delivery.getAbsPoint();

        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void TaskLoader(void *param)
{
    while(1)
    {
        if(MOTOR_Update())
        {
            uint8_t tx_data[3];
            tx_data[0] = 0xA1;
            tx_data[1] = 0xC3;
            tx_data[2] = 0x01;
            Serial.write(tx_data,3);
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void TaskKey(void *param)
{
    while(1)
    {
        uint8_t keyPressed[4];
        uint8_t lastKeyPressed[4];
        bool isKeyPressed = false;
        for(int i=0;i<4;i++)
        {
            lastKeyPressed[i] = keyPressed[i];
            keyPressed[i] = key[i].getKey();
            if(keyPressed[i] != lastKeyPressed[i])
                isKeyPressed = true;
        }

        if(isKeyPressed)
        {
            uint8_t tx_data[6];
            tx_data[0] = 0xA1;
            tx_data[1] = 0xC4;
            memcpy(tx_data+2,keyPressed,4);
            Serial.write(tx_data,6);
        }

        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}


void loop()
{
    // 一、设置模式，设置各个位置点

    /* 二、运行模式
        - 检测装载仓
          - 若检测到紧急装载仓（0号）则标记
          - 其余则依次放入队列（1，2，3号）
        - 扫描装载仓
          - 若紧急装载仓（0号）被标记则优先扫描
          - 其余按照队列顺序扫描
          - 扫描中亮黄灯，确认显微镜扫描完成，打印扫描进度
          - 扫描完成亮绿灯，退出队列
          - 连续未扫描到2片，提前结束
     * */
//    MOTOR_SetPower(0);
//    MOTOR_Update(1050);
//    CANOPEN.recvMsg();
//    led[0].setColor(Yellow);
//    Serial.println(key[0].getKey());
//    delivery.setAbsPoint(0,0);
//    keyDetect();
//    selectLoader();
//    while (1)
//    {
//        delivery.getAbsPoint();
//    }
//    Serial.println(encoder_count);
}

