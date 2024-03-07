#include <Arduino.h>
#include <encoder.h>
#include <motor.h>
#include <canopen.h>
#include <servo.h>

void setup()
{
    //初始化GPIO

    //初始化串口, Serial1: D0(RX) D1(TX)
    Serial1.begin(115200);
    //初始化CAN通信, CAN Transceiver: D13(CANRX0) D10(CANTX0)
//    myCAN.begin(CanBitRate::BR_1000k);
    myServo.init();
    //初始化电机编码器, D2(A相) D3(B相)
    ENCODER_Init();
    //初始化电机PWM，D7(CW) D8(CCW) D9(PWM)
    MOTOR_Init();
//    myCAN.read(1,0x1000,0,0);//你是谁
//    myCAN.write(1,0x2002,0x01,(uint16_t)0);//设置为CiA402模式
//    myCAN.write(1,0x6060,0x00,(uint8_t)1);//设置为轮廓位置模式
//    myCAN.write(1,0x607A,0x00,(uint32_t)10000);//设置目标位置为10000;
//    myCAN.write(1,0x6040,0x00,(uint16_t)0x06);//电机准备
//    myCAN.write(1,0x6040,0x00,(uint16_t)0x07);//电机失能
//    myCAN.write(1,0x6040,0x00,(uint16_t)0x0F);//电机使能
//    myCAN.write(1,0x6040,0x00,(uint16_t)0x1F);//触发电机运行
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
//        myCAN.recvMsg();
    myServo.setPoint(0,0);
    while (1){}
//    Serial1.println(encoder_count);
}

