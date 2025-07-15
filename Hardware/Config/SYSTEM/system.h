#ifndef _SYSTEM_H_
#define _SYSTEM_H_
#include "gd32f30x.h"
enum {
    FIRST_START_CHARGING = 1,               //第一次启动
    NET_RECOVER,                            //网络重连
    POWER_RECOVER,                          //重新上电
};
void system_init(void);
#endif
