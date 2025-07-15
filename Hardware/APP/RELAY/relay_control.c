/*
 * @Author: chenyuan
 * @Date: 2025-03-17 14:15:29
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-06-25 20:44:56
 * @FilePath: \FreeRtos实验0621\13.FreeRTOS实验\Hardware\APP\RELAY\relay_control.c
 * @Description: 
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#include "relay_control.h"
#include "global.h"
#include "timer.h"
#include "gpio.h"
#include "meter.h"
#include "charge.h"
uint16_t relay_open = 0x00; //bit3 :端口4 bit2：端口3 ...
uint16_t relay_close = 0x00; //bit3 :端口4 bit2：端口3 ...
uint8_t timerflag = 0;
uint8_t relay_flag = 0; //0 关闭继电器  1 开启继电器
uint32_t Insert_Check_Cnt[PORTMAX];
Relay_Port Relay_Array[PORTMAX] = {
    {RELAY_1_PORT,RELAY_1_PIN},
    {RELAY_2_PORT,RELAY_2_PIN},
    {RELAY_3_PORT,RELAY_3_PIN},
    {RELAY_4_PORT,RELAY_4_PIN},
    {RELAY_5_PORT,RELAY_5_PIN},
    {RELAY_6_PORT,RELAY_6_PIN},
    {RELAY_7_PORT,RELAY_7_PIN},
    {RELAY_8_PORT,RELAY_8_PIN},
    {RELAY_9_PORT,RELAY_9_PIN},
    {RELAY_10_PORT,RELAY_10_PIN},
    {RELAY_11_PORT,RELAY_11_PIN},
    {RELAY_12_PORT,RELAY_12_PIN},
};

Fuse_Port Fuse_Array[PORTMAX] = {
    {FUSE_1_PORT,FUSE_1_PIN},
    {FUSE_2_PORT,FUSE_2_PIN},
    {FUSE_3_PORT,FUSE_3_PIN},
    {FUSE_4_PORT,FUSE_4_PIN},
    {FUSE_5_PORT,FUSE_5_PIN},
    {FUSE_6_PORT,FUSE_6_PIN},
    {FUSE_7_PORT,FUSE_7_PIN},
    {FUSE_8_PORT,FUSE_8_PIN},
    {FUSE_9_PORT,FUSE_9_PIN},
    {FUSE_10_PORT,FUSE_10_PIN},
    {FUSE_11_PORT,FUSE_11_PIN},
    {FUSE_12_PORT,FUSE_12_PIN},

};

Fuse_Def Fuse_Det[PORTMAX];  //保险丝检测
void App_Relay_ON(void);
void App_Relay_OFF(void);

//定时器中断函数
void TIMER7_UP_IRQHandler(void)
{
    if(SET == timer_interrupt_flag_get(TIMER7,TIMER_INT_FLAG_UP)){ 
		timer_interrupt_flag_clear(TIMER7,TIMER_INT_FLAG_UP);
        
		timer_interrupt_disable(TIMER7, TIMER_INT_FLAG_UP);
        nvic_irq_disable(TIMER7_UP_IRQn);
		timer_flag_clear(TIMER7,TIMER_FLAG_UP);
        timer_disable(TIMER7);
        if(relay_flag == 0)
        {
            App_Relay_OFF();//控制继电器-关
        }
        else if(relay_flag == 1)
        {
            App_Relay_ON();//控制继电器-开
        }
        timerflag = 0;
    }
}
//定时器延时函数
void  App_Relay_Delay_Start(uint16_t peroid)
{
    if(timerflag == 0)
    {
        timerflag = 1;
        timer7_init(peroid);
    }
}
#if 0

//过零检测中断处理函数
void EXTI5_9_IRQHandler(void)
{
	 if (RESET != exti_interrupt_flag_get(EXTI_5)) 
	 {  
		exti_interrupt_flag_clear(EXTI_5);
        if(gpio_input_bit_get(GPIOC,GPIO_PIN_5) == 0)//电压过零点中断
        {
            if(relay_open != 0)//开继电器
            {
                relay_flag = 1;
                App_Relay_Delay_Start(37);	
            }
            else if(relay_close != 0)
            {
                relay_flag = 0;
                App_Relay_Delay_Start(42);
            }
            INSERT_DET_OFF;
            Delay_ms(1);
            if(READ_DET1 == 1)
            {
                Insert_Check_Cnt[0]++;
            }
            else
            {
                Insert_Check_Cnt[0] = 0;
            }
            if(READ_DET2 == 1)
            {
                Insert_Check_Cnt[1]++;
            }
            else
            {
                Insert_Check_Cnt[1] = 0;
            }
            if(READ_DET3 == 1)
            {
                Insert_Check_Cnt[2]++;
            }
            else
            {
                Insert_Check_Cnt[2] = 0;
            }
            if(READ_DET4 == 1)
            {
                Insert_Check_Cnt[3]++;
            }
            else
            {
                Insert_Check_Cnt[3] = 0;
            }
            INSERT_DET_ON;
        }
        else
        {
            INSERT_DET_OFF;
            Delay_ms(1);
            if(READ_DET1 == 0)
            {
                Insert_Check_Cnt[0]++;
            }
            else
            {
                Insert_Check_Cnt[0] = 0;
            }
            if(READ_DET2 == 0)
            {
                Insert_Check_Cnt[1]++;
            }
            else
            {
                Insert_Check_Cnt[1] = 0;
            }
            if(READ_DET3 == 0)
            {
                Insert_Check_Cnt[2]++;
            }
            else
            {
                Insert_Check_Cnt[2] = 0;
            }
            if(READ_DET4 == 0)
            {
                Insert_Check_Cnt[3]++;
            }
            else
            {
                Insert_Check_Cnt[3] = 0;
            }
            for(uint8_t i = 0; i < PORTMAX; i++)
            {
                if(Insert_Check_Cnt[i] >= 100)
                {
                    Insert_flag[i] = 1;
                }
                else
                {
                    Insert_flag[i] = 0;
                }
            }
        }
        INSERT_DET_ON;
	 }
}
#endif

void App_Relay_Control(uint8_t port,uint8_t flag)
{
    LOG("relay :%d flag :%d\r\n",port,flag);
    if(port == 0)  //全开或者全关
    {
        if(flag == ON)
        {
            relay_open = 0x0FFF;
        }
        else
        {
            relay_close = 0x0FFF;
        }
    }
    else
    {
        if(flag == ON)
        {
            memset(&Fuse_Det[port-1],0,sizeof(Fuse_Def));
            relay_open |= 0x01 << (port-1);
        }
        else
        {
            relay_close |= 0x01 << (port-1);
        }
    }
}

//控制继电器开
void App_Relay_ON(void)
{
    uint8_t i = 0;
    while (relay_open)
    {
        if(relay_open >> i&0x01)
        {
            gpio_bit_set(Relay_Array[i].port, Relay_Array[i].pin);
            relay_open &= ~(1<<i);
        }
        i++;
    }
    
}

//控制继电器关
void App_Relay_OFF(void)
{
    uint8_t i = 0;
    while (relay_close)
    {
        if(relay_close >> i&0x01)
        {
            gpio_bit_reset(Relay_Array[i].port, Relay_Array[i].pin);
            relay_close &= ~(1<<i);
        }
        i++;
    }
    
}

void App_Relay_Control_All(uint8_t flag)
{

    App_Relay_Control(0,flag);

}

