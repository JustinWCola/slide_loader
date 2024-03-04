//
// Created by JustinWCola on 2024/2/29.
//

#ifndef SERVO_H
#define SERVO_H

//控制模式 Ctrl Mode
#define CiA402    0x00  //CiA402模式
#define NiM_POS   0x01  //NiMotion位置模式
#define NiM_VEL   0x02  //NiMotion速度模式
#define NiM_TRQ   0x03  //NiMotion转矩模式
#define NiM_OPN   0x04  //NiMotion开环模式

//运动模式 Move Mode
#define PP  0x01    //轮廓位置模式
#define VM  0x02    //速度模式
#define PV  0x03    //轮廓速度模式
#define PT  0x04    //轮廓转矩模式
#define HM  0x06    //原点回归模式
#define IP  0x07    //插补模式
#define CSP 0x08    //循环同步位置模式
#define CSV 0x09    //循环同步速度模式
#define CST 0x0A    //循环同步转矩模式

//运行模式 Run Mode
#define ABS_POS     0x0F    //绝对位置模式,非立刻更新
#define ABS_POS_RT  0x2F    //绝对位置模式,立刻更新
#define REV_VEL     0x4F    //相对位置模式,非立刻更新
#define REV_VEL_RT  0x6F    //相对位置模式,立刻更新

void SERVO_SetCtrlMode(uint8_t id, uint32_t ctrl_mode);
void SERVO_SetMoveMode(uint8_t id, uint32_t move_mode);
void SERVO_SetPos(uint8_t id, uint32_t pos);
void SERVO_SetVel(uint8_t id, uint32_t vel);
void SERVO_SetDcc(uint8_t id, uint32_t dcc);
void SERVO_SetAcc(uint8_t id, uint32_t acc);
void SERVO_Enable(uint8_t id);
void SERVO_SetRunMode(uint8_t id, uint32_t run_mode);



#endif //SERVO_H
