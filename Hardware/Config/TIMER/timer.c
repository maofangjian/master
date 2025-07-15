#include "timer.h"
//#include "led.h"
#include <stdio.h>
#include "relay_control.h"
#include "global.h"
/*��̬����----------------------------------------------------------------*/
static SYSTIMERMSG	Sys_Timer_Msg[TIMERMSGNUM];

//ms��ʱ����
void Delay_ms(uint16_t mSec)
{
	while (mSec--)
	{
		Delay_us(1000);
	}
}

//us��ʱ����
void Delay_us(uint16_t count)
{
	__IO uint32_t i = 0;
	__IO uint8_t j = 0;

	for(i=0; i<count; i++) {
	
		for(j=0; j<8; j++);
	}
}
/**
  * @brief  ��ʱ����ʼ��
  *			����10ms�Ļ�׼��ʱ��
  * @param  None
  * @retval None
  */
void YA_TimerInit(void)
{
    // 使能定时器时钟
    rcu_periph_clock_enable(RCU_TIMER1);
    timer_deinit(TIMER1); // 复位定时器

    // 配置定时器参数
    timer_parameter_struct timer_initpara;
    timer_initpara.prescaler = 14399;        // 预分频值
    timer_initpara.alignedmode = TIMER_COUNTER_EDGE; // 边沿对齐模式
    timer_initpara.counterdirection = TIMER_COUNTER_UP; // 向上计数
    timer_initpara.period = 99;              // 自动重装载值
    timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
    timer_init(TIMER1, &timer_initpara);

    // 立即加载预分频器值
    timer_prescaler_config(TIMER1,14399,TIMER_PSC_RELOAD_NOW);

    // 清除中断标志（防止首次误触发）
    timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);

    // 使能更新中断
    timer_interrupt_enable(TIMER1, TIMER_INT_UP);

    // 配置NVIC中断优先级
    nvic_irq_enable(TIMER1_IRQn, 1, 1); // 优先级组1，子优先级1

    // 启动定时器
    timer_enable(TIMER1);
		
}

void timer7_init(uint16_t period) 
{
	timer_parameter_struct timer_initpara;
	rcu_periph_clock_enable(RCU_TIMER7);
	
	timer_deinit(TIMER7);
	timer_initpara.prescaler         = 12000-1;	//Ԥ��Ƶ0.1ms
	timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection  = TIMER_COUNTER_UP;
	timer_initpara.period            = period;	
	timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init(TIMER7, &timer_initpara);

	timer_auto_reload_shadow_enable(TIMER7);
	timer_interrupt_flag_clear(TIMER7, TIMER_INT_UP);
	timer_interrupt_enable(TIMER7, TIMER_INT_UP);
	
	timer_flag_clear(TIMER7,TIMER_FLAG_UP);
	timer_interrupt_enable(TIMER7, TIMER_INT_FLAG_UP);
	
	nvic_irq_enable(TIMER7_UP_IRQn, 2, 1);
	timer_enable(TIMER7);
}
/**
  * @brief  10ms��ʱ���жϴ�������
  * @param  None
  * @retval None
  */
void TIMER1_IRQHandler(void)
{
     if (timer_interrupt_flag_get(TIMER1, TIMER_INT_FLAG_UP) == SET){
		timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);
		for (uint16_t fuseIndex = 0; fuseIndex < PORTMAX; fuseIndex++)
		{
			if(gpio_input_bit_get(Fuse_Array[fuseIndex].port, Fuse_Array[fuseIndex].pin) == RESET)  //���͵�ƽ
			{
				Fuse_Det[fuseIndex].low_time++;
				Fuse_Det[fuseIndex].total_time++;
			}
			else
			{
				Fuse_Det[fuseIndex].high_time++;
				Fuse_Det[fuseIndex].total_time++;
			}
		}
    }
}




