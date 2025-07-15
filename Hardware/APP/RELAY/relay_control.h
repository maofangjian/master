#ifndef _RELAY_CONTROL_H_
#define _RELAY_CONTROL_H_
#include "gd32f30x.h"

typedef struct{
    uint32_t port;
    uint16_t pin;
}Relay_Port;

typedef struct{
    uint32_t port;
    uint16_t pin;
}Fuse_Port;

typedef struct
{
    uint32_t high_time; //高电平时间
    uint32_t low_time; //低电平时间
    uint32_t total_time; //总时间
}Fuse_Def;

extern Relay_Port Relay_Array[];
extern uint8_t timerflag;
extern uint8_t relay_flag; //0 关闭继电器  1 开启继电器
extern uint16_t relay_open; //bit3 :端口4 bit2：端口3 ...
extern uint16_t relay_close; //bit3 :端口4 bit2：端口3 ...
extern uint32_t Insert_Check_Cnt[];
extern Fuse_Port Fuse_Array[];
extern Fuse_Def Fuse_Det[];
void  App_Relay_Delay_Start(uint16_t peroid);
void App_Relay_Control_All(uint8_t flag);
void App_Relay_Control(uint8_t port,uint8_t flag);
#endif

