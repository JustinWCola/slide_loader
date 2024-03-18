#include <Arduino.h>
#include <encoder.h>
#include <motor.h>
#include <canopen.h>
#include <delivery.h>
#include <led.h>
#include <key.h>
#include <queue>

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

void keyDetect()
{
    if(key[0].getKey() == RISING)
        isVIP = true;
    for(int i=1;i<4;i++)
    {
        if(key[i].getKey()== RISING)
        {
            id_queue.push(i);
            led[i].setColor(Red);
        }
    }
}

void takeSlide()
{
    MOTOR_Update(Y_DISTANCE);//执行机构伸出
    delivery.setRevPoint(0, Z_LIFT);//向上
    MOTOR_Update(0);//执行机构收回
}

void giveSlide()
{
    MOTOR_Update(Y_DISTANCE);//执行机构伸出
    delivery.setRevPoint(0,-Z_LIFT);//向下
    MOTOR_Update(0);//执行机构收回
}

void startLoader(uint8_t id)
{
    led[id].setColor(Yellow);//开始装载，亮黄灯
    delivery.setAbsPoint(X_GAP * id + 0, 0);
    for(int i=0;i<24;i++)
    {
        takeSlide();//从装载仓取出
        delivery.setAbsPoint(20000,0);//运送到载物台
        giveSlide();//放入载物台
        while(!Serial.find("1"))
            Serial.println("wait for scanning");//等待扫描
        takeSlide();//从载物台取出
        delivery.setAbsPoint(X_GAP * id + 0, Z_GAP * (i-1));//运送回下一层
        giveSlide();//放入装载仓
        delivery.setRevPoint(0,Z_GAP-Z_LIFT);//到下一个位置
        Serial.print("complete scanning:");
        Serial.print(id);
        Serial.print(",");
        Serial.println(i);
    }
    led[id].setColor(Green);//完成装载，亮绿灯
}

void selectLoader()
{
    if(isVIP)
        startLoader(0);
    if(!id_queue.empty())
    {
        startLoader(id_queue.front());
        id_queue.pop();
    }
}

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

