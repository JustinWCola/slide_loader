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

// 编码器
Encoder motor_encoder(2,3);
// PID参数
Pid motor_pid(1,0,0.1);
// 限位开关
/**
 * 初始化按键引脚
 */
// void Key::init()
// {
//     pinMode(_pin, INPUT);
// }
// pinMode(_pin=0, INPUT)
Key sw(0);
// 定义电机

// pinMode(_cw_pin,OUTPUT);
// pinMode(_ccw_pin,OUTPUT);
// pinMode(_pwm_pin,OUTPUT);
Motor motor(8,1,9,&motor_encoder,&motor_pid,&sw);

Key key[4]{14,15,16,17};

static bool isKeyDetected;
static uint8_t keyStatus[4];

Led led[4]{{18,19},{13,12},{11,10},{7,6}};

// void position_Send(float pos, uint8_t identify);
void taskSerial(void *param);
// void taskSerialSend(void *param);
void taskDelivery(void *param);
void taskLoader(void *param);
void taskKey(void *param);
void keyUpdate();

void encoderUpdate()
{
    motor_encoder.update();
}


void setup()
{
    delay(5000);
    //初始化LED
    for(auto &led: led)
        led.init();
    //初始化按键
    for(auto &key: key)
        key.init();
    //初始化串口, Serial: USB
    Serial.begin(115200);
    Serial.print("Serial init finished");
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
    
    delay(3000);
    //初始化CAN通信, D4(CANTX0) D5(CANRX0)
    CANOPEN.begin(CanBitRate::BR_1000k);
    
    Serial.print("Hello world!");
    //初始化伺服电机
    delay(3000);
    axis_x.init(led);
    delay(3000);
    axis_z.init(led);
    
    // LED灯复位
    led[0].setColor(Off);

    // 当前位置清零
    motor.clear();

    Serial.print("Servo init finished\n");

    xTaskCreate(taskSerial, "Serial", 1024, nullptr, 2, nullptr);
    // xTaskCreate(taskSerialSend, "SerialSend", 1024, nullptr, 2, nullptr);
    xTaskCreate(taskDelivery, "Delivery", 256, nullptr, 2, nullptr);
    xTaskCreate(taskLoader, "Loader", 128, nullptr, 1, nullptr);
    xTaskCreate(taskKey, "Key", 128, nullptr, 2, nullptr);
    Serial.print("Start working\n");

    vTaskStartScheduler();
}

// void position_Send(float pos, uint8_t identify)
// {
//     uint8_t tx_data[8];
//     float x_pos = pos;
//     tx_data[0] = 0xAA;
//     tx_data[1] = identify;
//     memcpy(tx_data + 2, &x_pos, 4);
//     tx_data[6] = crc8Check(tx_data,6);
//     tx_data[7] = 0xFF;
//     Serial.flush();
//     Serial.write(tx_data,8);
// }

// void taskSerialSend(void *param)
// {
//     static uint32_t send_time = 0;
//     uint8_t rx_data[8] = {0};
//     float x = 0, y = 0, z = 0;
//     while(1)
//     {
//         // AA C0 00 XX XX XX CRC FF
//         static bool busy_now, busy_last;
//         busy_now = !(axis_x.getReach() && axis_z.getReach() && motor.getReach());
//         // 打印x,y,z的busy状态
//         // Serial.print("x:");
//         // Serial.print(axis_x.getReach());
//         // Serial.print("z:");
//         // Serial.print(axis_z.getReach());
//         // Serial.print("y:");
//         // Serial.print(motor.getReach());
//         // Serial.print("\n");
//         if(!busy_now && busy_last)
//         {
//             uint8_t tx_data[8];
//             tx_data[0] = 0xAA;
//             tx_data[1] = 0xC0;
//             tx_data[2] = 0x00;
//             tx_data[3] = 0x00;
//             tx_data[4] = 0x00;
//             tx_data[5] = 0x00;
//             tx_data[6] = crc8Check(tx_data,6);
//             tx_data[7] = 0xFF;
//             Serial.flush();
//             Serial.write(tx_data,8);
//         }
//         else if(isKeyDetected)
//         {
//             uint8_t tx_data[8];
//             tx_data[0] = 0xAA;
//             tx_data[1] = 0xC4;
//             memcpy(tx_data + 2, &keyStatus, 4);
//             tx_data[6] = crc8Check(tx_data,6);
//             tx_data[7] = 0xFF;
//             Serial.flush();
//             Serial.write(tx_data,8);
//             isKeyDetected = false;
//         }
    
//         busy_last = busy_now;

//         send_time++;
//         Serial.print("x:");
//         Serial.print(axis_x.getAbsPos());

//         Serial.print("z:");
//         Serial.print(axis_z.getAbsPos());

//         Serial.print("y:");
//         Serial.print(motor.getPos());

//         Serial.print("sw:");
//         Serial.print(motor.get_sw_Pos());

//         Serial.print("\n");

//         position_Send(axis_x.getAbsPos(),0xC1);
//         position_Send(axis_z.getAbsPos(),0xC2);
//         position_Send(motor.getPos(),0xC3);
//         position_Send(motor.get_sw_Pos(),0xC5);

//         vTaskDelay(5/portTICK_PERIOD_MS);
//     }
// }

// 串口通讯任务
void taskSerial(void *param)
{
    static uint32_t send_time = 0;
    uint8_t rx_data[8] = {0};
    float x = 0, y = 0, z = 0;
    while(1)
    {
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
                        // Serial.print("setting point:");
                        // Serial.print(pos);
                        // Serial.print("\n");
                        case 0xB2:
                            memcpy(&z, rx_data + 2, 4);
                            axis_z.setAbsPos(z);
                            break;
                        // y递送指令
                        case 0xB3:
                        // rx_data + 2从第二个位置开始读取4个
                        // 从rx_data+2的地址开始的4个字节的数据转移到y中（赋值）
                            memcpy(&y, rx_data + 2, 4);
                            
                        // _is_reach = false;  //到达标志位只在接收到新指令后清零
                        // _target_now = target;
                        // _is_cmd = true;
                            motor.setTarget(y);
                            break;
                        //xB4->灯控制
                        // red = b"\x01"
                        // green = b"\x10"
                        // yellow = b"\x11"
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

        static bool busy_now, busy_last;
        // 打印x,y,z的busy状态
        busy_now = !(axis_x.getReach() && axis_z.getReach() && motor.getReach());
        if(!busy_now && busy_last)
        {
            uint8_t tx_data[8];
            tx_data[0] = 0xAA;
            tx_data[1] = 0xC0;
            tx_data[2] = 0x00;
            tx_data[3] = 0x00;
            tx_data[4] = 0x00;
            tx_data[5] = 0x00;
            tx_data[6] = crc8Check(tx_data,6);
            tx_data[7] = 0xFF;
            Serial.flush();
            Serial.write(tx_data,8);
        }
        else if(isKeyDetected)
        {
            uint8_t tx_data[8];
            tx_data[0] = 0xAA;
            tx_data[1] = 0xC4;
            memcpy(tx_data + 2, &keyStatus, 4);
            tx_data[6] = crc8Check(tx_data,6);
            tx_data[7] = 0xFF;
            Serial.flush();
            Serial.write(tx_data,8);
            isKeyDetected = false;
        }
        else if(send_time % 50 == 0)
        {
            uint8_t tx_data[8];
            float x_pos = axis_x.getAbsPos();
            tx_data[0] = 0xAA;
            tx_data[1] = 0xC1;
            memcpy(tx_data + 2, &x_pos, 4);
            tx_data[6] = crc8Check(tx_data,6);
            tx_data[7] = 0xFF;
            Serial.flush();
            Serial.write(tx_data,8);
        }
        else if(send_time % 50 == 10)
        {
            uint8_t tx_data[8];
            float z_pos = axis_z.getAbsPos();
            tx_data[0] = 0xAA;
            tx_data[1] = 0xC2;
            memcpy(tx_data + 2, &z_pos, 4);
            tx_data[6] = crc8Check(tx_data,6);
            tx_data[7] = 0xFF;
            Serial.flush();
            Serial.write(tx_data,8);
        }        
        else if(send_time % 50 == 30)
        {
            uint8_t tx_data[8];
            float y_pos = motor.getPos();
            tx_data[0] = 0xAA;
            if(motor.get_sw_Pos()==0){tx_data[1] = 0xC5;}
            else{tx_data[1] = 0xC6;}
            memcpy(tx_data + 2, &y_pos, 4);
            tx_data[6] = crc8Check(tx_data,6);
            tx_data[7] = 0xFF;
            Serial.flush();
            Serial.write(tx_data,8);
        }
        busy_last = busy_now;
        send_time++;

        vTaskDelay(5/portTICK_PERIOD_MS);
    }
}

// 传送任务
void taskDelivery(void *param)
{
    while(1)
    {
        axis_x.update();
        axis_z.update();
        axis_x.updateStatus();
        axis_z.updateStatus();
        vTaskDelay(5/portTICK_PERIOD_MS);
    }
}
// 加载任务
void taskLoader(void *param)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    static uint32_t send_time = 0;
    while(1)
    {
        motor.update();
        motor.updateStatus();
        // 5ms
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5));
        send_time++;
    }
}

// 摁键交互任务
void taskKey(void *param)
{
    while(1)
    {
        keyUpdate();
        vTaskDelay(5/portTICK_PERIOD_MS);
    }
}

// 判断摁键状态,延时防抖动
void keyUpdate()
{
    static uint8_t keyPressed[4];
    static uint8_t lastKeyPressed[4];    
    // None = 0x00,
    // Pressed = 0x01,
    // Released = 0x02,
    for(int i=0;i<4;i++)
    {
        lastKeyPressed[i] = keyPressed[i];
        if(key[i].getKey()==HIGH)
        {
            delay(10);
            if(key[i].getKey()==HIGH)
                keyPressed[i] = HIGH;
        }
        if(key[i].getKey()==LOW)
        {
            delay(10);
            if(key[i].getKey()==LOW)
                keyPressed[i]= LOW;
        }
        if(keyPressed[i] != lastKeyPressed[i])
        {
            // 检测到摁键的键位变化
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

// 循环
void loop()
{

}
