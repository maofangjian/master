/*!
    \file  gd32f30x_it.c
    \brief interrupt service routines
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-06-30, V1.0.0, demo for GD32F30x
*/

#include "gd32f30x_it.h"
#include "systick.h"
#include "FreeRTOS.h"
#include "task.h"
#include "i2s.h"
/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1){
    }
}


void DMA0_Channel4_IRQHandler(void)
{
    if(dma_interrupt_flag_get(DMA0,DMA_CH4,DMA_FLAG_FTF) != RESET)
    {
        dma_interrupt_flag_clear(DMA0,DMA_CH4,DMA_FLAG_FTF);
        dma_channel_disable(DMA0,DMA_CH4);
        dma_complete_flag = 0;
    }
}
/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void SVC_Handler(void)
//{
//}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void PendSV_Handler(void)
//{
//}

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
uint32_t i = 0;
extern void xPortSysTickHandler(void);

//systick中断服务函数
void SysTick_Handler(void)
{	
    if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)//系统已经运行
    {
        i++;
        xPortSysTickHandler();	
    }
}
