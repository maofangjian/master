#ifndef _LED_H_
#define _LED_H_
#include "gd32f30x.h"

typedef enum{
    LED_OFF = 0, //LED灯关闭
    LED_ON,  //常亮
    LED_FLASH, //闪烁
}Led_Def;

typedef struct 
{
    uint32_t port;
    uint16_t pin;
}Led_Port;

extern Led_Def ledstatus;
void App_Ledturnoff(void);
void App_Qrledtoggle(void);
void App_Led_Ctrl(uint8_t door,Led_Def flag);
#endif


