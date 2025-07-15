/*
 * @Author: chenyuan
 * @Date: 2025-07-03 15:04:33
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-07-08 18:33:08
 * @FilePath: \FreeRtos实验0703\13.FreeRTOS实验\Hardware\systick.c
 * @Description: 
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
/*!
    \file  systick.c
    \brief the systick configuration file
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-06-23, V1.0.0, demo for GD32F30x
*/

#include "gd32f30x.h"
#include "systick.h"

volatile static uint32_t delay;

/*!
    \brief      configure systick
    \param[in]  none
    \param[out] none
    \retval     none
*/
void systick_config(void)
{
    systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);  //选择外部时钟  HCLK
    if (SysTick_Config(SystemCoreClock / 1000U)){
        /* capture error */
        while (1){
        }
    }
    /* configure the systick handler priority */
    //NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

/*!
    \brief      delay a time in milliseconds
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
void delay_1ms(uint32_t count)
{
    delay = count;
}

/*!
    \brief      delay decrement
    \param[in]  none
    \param[out] none
    \retval     none
*/
void delay_decrement(void)
{
}
