/*!
    \file    systick.c
    \brief   the systick configuration file

    \version 2017-02-10, V1.0.0, firmware for GD32F30x
    \version 2018-10-10, V1.1.0, firmware for GD32F30x
    \version 2018-12-25, V2.0.0, firmware for GD32F30x
    \version 2020-09-30, V2.1.0, firmware for GD32F30x 
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
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
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / 10000U)){//--1ms
        /* capture error */
        while (1){
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

/*!
    \brief      delay a time in milliseconds
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
void delay_us(uint32_t count)
{
    delay = count;

    while(0U != delay){
    }
}

/* 描述：us级延时函数
 * 参数nus：需要延时的us数
 * 返回值：无*/		    								   
void delay_us_x(uint32_t nus)
{		
	uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;		/* 滴答定时器的重装载值 */
    ticks = nus * 120; 						/* 需要的节拍数 */
    told = SysTick->VAL;        			/* 刚进入时的计数器值 */

    while(1)
    {
        tnow = SysTick->VAL;

        if(tnow != told)
        {
            if(tnow < told)tcnt += told - tnow;
            else tcnt += reload - tnow + told;
            if(tcnt >= ticks)break;			 /* 时间超过/等于要延迟的时间,则退出. */
			told = tnow;
        }
    }  
}


/*!
    \brief      delay decrement
    \param[in]  none
    \param[out] none
    \retval     none
*/
void delay_decrement(void)
{
    if (0U != delay){
        delay--;
    }
}

//void delay_us(unsigned int nus)
//{
	// unsigned int temp;
	// SysTick->LOAD = 6*nus;
	 //SysTick->VAL=0X00;                      //清空计数器 自动重装LOAD
	// SysTick->CTRL=0X01;                     //使能，减到零是无动作，采用外部时钟源
	// do
	// {
	//  temp=SysTick->CTRL;                    //读取当前倒计数值
	// }while((temp&0x01)&&(!(temp&(1<<16)))); //等待时间到达
	 
	// SysTick->CTRL=0x00;                     //关闭计数器
	// SysTick->VAL =0X00;                     //清空计数器
//}


