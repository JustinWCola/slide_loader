#include <Arduino.h>
#include <encoder.h>
#include <can.h>

void setup() {
    //初始化GPIO

    //初始化串口，Serial1: D0(RX) D1(TX)
    Serial1.begin(115200);
    //初始化CAN通信，CAN Transceiver: D13(CANRX0) D10(CANTX0)
//    CAN_Init();
    //初始化电机PWM

    //初始化电机编码器
    ENCODER_Init();
    //各轴位置归零

}

void loop() {
    // 一、设置模式，设置各个位置点

    /* 二、运行模式
     * 1.检测待扫描装载仓（1，2，3号）
     * 2.开始顺序扫描该装载仓，保存各装载仓扫描进度
     * 3.扫描过程中若检测到紧急装载仓（4号）优先扫描
     * 4.检测是否为空，连续两次为空视为扫描完成
     * 5.发送扫描完成信号
     * */
    Serial1.println(encoder_count);
}

