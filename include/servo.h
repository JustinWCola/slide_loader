//
// Created by JustinWCola on 2024/2/29.
//

#ifndef SERVO_H
#define SERVO_H

#define I_CONTROL_WORD    0x6040  //控制字
#define I_STATUS_WORD     0x6041  //状态字
#define I_DEVICE_TYPE     0x1000  //设备类型

#define I_CTRL_PARAM      0x2002  //控制参数
#define SI_CTRL_MODE      0x01    //控制模式

typedef enum eCtrlMode : uint16_t
{
    CiA402 = 0,     //CiA402模式
    NiMPos = 1,     //NiMotion位置模式
    NiMVel = 2,     //NiMotion速度模式
    NiMTrq = 3,     //NiMotion转矩模式
    NiMOpn = 4,     //NiMotion开环模式
}; //控制模式

#define I_MOTION_MODE   0x6060  //运动模式

typedef enum eMotionMode : uint8_t
{
    PP = 0x01,      //轮廓位置模式
    VM = 0x02,      //速度模式
    PV = 0x03,      //轮廓速度模式
    PT = 0x04,      //轮廓转矩模式
    HM = 0x06,      //原点回归模式
    IP = 0x07,      //插补模式
    CSP = 0x08,     //循环同步位置模式
    CSV = 0x09,     //循环同步速度模式
    CST = 0x0A,     //循环同步转矩模式
};   //运动模式

#define I_NOW_POSITION      0x6064  //当前位置
#define I_TARGET_POSITION   0x607A  //目标位置
#define I_PROFILE_VELOCITY  0x6081  //轮廓速度
#define I_END_VELOCITY      0x6082  //终点速度
#define I_PROFILE_ACCELERATION  0x6083  //轮廓加速度
#define I_PROFILE_DECELERATION  0x6084  //轮廓减速度

typedef enum eTriggerMode : uint16_t
{
    AbsPos = 0x0F,      //绝对位置模式,非立刻更新
    AbsPosRt = 0x2F,    //绝对位置模式,立刻更新
    RevPos = 0x4F,      //相对位置模式,非立刻更新
    RevPosRt = 0x6F,    //相对位置模式,立刻更新
};  //触发模式

#define I_ZERO_MODE         0x6098  //回零模式

typedef enum eZeroMode : uint8_t
{
    NegativeSwitch = 17,  //负方向限位开关
    PositiveSwitch = 18,  //正方向限位开关
    PositiveStuck = 37,   //正方向堵转
    NegativeStuck = 38,   //反方向堵转
};

#define I_ZERO_VELOCITY     0x6099  //回零速度
#define SI_HIGH_VELOCITY    0X01    //查找开关（高速）速度
#define SI_LOW_VELOCITY     0X02    //查找零点（低速）速度

#define I_ZERO_ACCELERATION 0x609A  //回零加速度

#define I_STUCK_CHECK   0x2007  //堵转检测
#define SI_STUCK_TORQUE 0X13    //堵转转矩检测
#define SI_STUCK_TIME   0X15    //堵转时间检测

#define I_POSITION_CONTROL  0X2005  //位置控制参数
#define SI_ZERO_TIME_LIMIT  0X1C    //回零时间限制


class Servo
{
public:
    Servo(CANopen& can, uint8_t id, float guide):_can(can),_id(id),_guide(guide){}

    void init();

    void setAbsPos(float pos);
    void setRevPos(float pos);
    void setUnitConvert(float point);
    void update();
    void updateStatus();
    bool getReach();
    void send();

private:
    bool setCtrlMode(eCtrlMode ctrl_mode);
    bool setMotionMode(eMotionMode motion_mode);
    void setMotorReady();
    void disableMotor();
    void enableMotor();

    bool setZero();
    bool setAbsPosition(int32_t pos, uint32_t vel);
    bool setAbsPosition(int32_t pos);
    bool setRevPosition(int32_t pos, uint32_t vel);
    bool setRevPosition(int32_t pos);
    int32_t getAbsPosition();

private:
    uint8_t _id;
    CANopen _can;

    float _target_now = 0;
    float _input_now = 0;
    float _input_last = 0;

    float _guide;
    float _pulse = 10000.0f;
    float _pulse_to_mm = (_guide/_pulse);//编码器位置 -> 实际位置(mm)

    bool _is_reach = false;
    uint8_t _reach_time = 0;
};

#endif //SERVO_H
