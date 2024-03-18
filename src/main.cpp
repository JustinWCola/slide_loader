#include <Arduino.h>
#include <encoder.h>
#include <motor.h>
#include <canopen.h>
#include <servo.h>
#include <delivery.h>
#include <led.h>
#include <key.h>

CANopen CANOPEN;
Delivery delivery(CANOPEN);
Led led[4]{{18,19},{12,11},{7,6},{5,4}};
Key key[4]{14,15,16,17};

void setup()
{
    //初始化GPIO
    for(auto &led: led)
        led.init();
    for(auto &key: key)
        key.init();
    //初始化串口, Serial: USB
    Serial.begin(115200);
    //初始化CAN通信, CAN Transceiver: D13(CANRX0) D10(CANTX0)
    CANOPEN.begin(CanBitRate::BR_1000k);
    //初始化伺服电机
    delivery.init();
    //初始化电机编码器, D2(A相) D3(B相)
    ENCODER_Init();
    //初始化电机PWM, D8(CW) D9(PWM)
    MOTOR_Init();
}

void loop()
{
    // 一、设置模式，设置各个位置点

    /* 二、运行模式
     * 1.检测待扫描装载仓（1，2，3号）
     * 2.开始顺序扫描该装载仓，保存各装载仓扫描进度
     * 3.扫描过程中若检测到紧急装载仓（4号）优先扫描
     * 4.检测是否为空，连续两次为空视为扫描完成
     * 5.发送扫描完成信号
     * */
//    MOTOR_SetPower(0);
//    MOTOR_Update(1050);
//    CANOPEN.recvMsg();
//    led[0].setColor(Yellow);
    Serial.println(key[0].getKey());
    delivery.setAbsPoint(0,0);
//    while (1)
//    {
//        delivery.getAbsPoint();
//    }
//    Serial.println(encoder_count);
}

