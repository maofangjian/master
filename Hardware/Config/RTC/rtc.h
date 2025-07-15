#ifndef _RTC_H_
#define _RTC_H_

#include "gd32f30x.h"
#include <time.h>
#include <stdint.h>
#include <math.h>

void RTC_init(void);
void App_Set_Rtc_Time(uint32_t time);
void App_Get_Rtc_Time(uint8_t *day);
int Rtc_To_Realtime(time_t time, uint8_t *Day);
uint32_t App_GetRtcCount(void);
void Rtc_TimerInit(void);
#endif

