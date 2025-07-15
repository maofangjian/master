/*
 * @Author: chenyuan
 * @Date: 2025-03-17 14:15:29
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-07-09 10:55:15
 * @FilePath: \FreeRtos实验0703\13.FreeRTOS实验\Hardware\Config\SYSTEM\system.c
 * @Description: 
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#include "system.h"
#include "gpio.h"
#include "spi.h"
#include "timer.h"
#include "global.h"
#include "watchdog.h"
#include "adc.h"
#include "led.h"
#include "relay_control.h"
#include "rtc.h"
#include "usart.h"
#include "systick.h"
#include "start.h"
#include "i2s.h"

void system_reset_reason(void)
{
    if (rcu_flag_get(RCU_FLAG_EPRST) == SET)
    {
        printf("external reset pin reset\r\n");
    }
    else if (rcu_flag_get(RCU_FLAG_PORRST) == SET)
    {
        printf("power on reset\r\n");
    }
    else if (rcu_flag_get(RCU_FLAG_SWRST) == SET)
    {
        printf("software reset\r\n");
    }
    else if (rcu_flag_get(RCU_FLAG_FWDGTRST) == SET)
    {
        printf("independ watchdog reseet\r\n");
    }
    else if (rcu_flag_get(RCU_FLAG_WWDGTRST) == SET)
    {
        printf("window watchdog reset\r\n");
    }
    else if (rcu_flag_get(RCU_FLAG_LPRST) == SET)
    {
        printf("low power reset\r\n");
    }
    rcu_all_reset_flag_clear();  
    printf("\r\n\r\n\r\n/*****************************************************************************\r\n");
}


//系统初始化
void system_init(void)
{
	/*设置系统中断优先级分组4*/
    systick_config();
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    YA_uart_init();
    system_reset_reason();
    Rtc_TimerInit();
    YA_GpioInit();
    YA_SpiInit(SPI_FLASH);
    YA_SpiInit(SPI_NFC);
    i2s_dma_init();
    YA_TimerInit();
    YA_Adc_Init();
    YA_Watchdog_Init();
    // App_Led_Ctrl(0,LED_ON);
}


