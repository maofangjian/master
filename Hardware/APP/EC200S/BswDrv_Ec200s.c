/*
 * @Author: chenyuan
 * @Date: 2025-04-17 14:15:02
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-04-18 15:47:14
 * @FilePath: \13.FreeRTOS实验\Hardware\APP\EC200S\BswDrv_Ec200s.c
 * @Description: 
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#include "BswDrv_Ec200s.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "gpio.h"
#include "usart.h"
#include "fifo.h"
#include "BswAbstr_Ec200s.h"
void BswDrv_Ec200sPowerOn(void)
{
	PWR_4G_ON;
	Delay_ms(1000);
}

void BswDrv_Ec200sPowerOff(void)
{
	PWR_4G_OFF;
	Delay_ms(1000);
}

void BswDrv_Ec200sGetCell(void)
{
	Abstr_Ec200sQcellSendCmd((void *)"AT+QCELL?\r", (void *)"OK", 10000, 0);
}

void BswDrv_Ec200sUartSend(uint8_t *data, uint32_t len)
{
	Bsp_UartSendData(M_4G_UART, (void *)data, len);
}

int32_t BswDrv_Ec200s_GetCharFromFifo(uint8_t *ch)
{
	return App_UartGetOneData(M_4G_UART, ch);
}

void BswDrv_Ec200s_FifoFlush(void)
{
	App_UartFlushFifo(M_4G_UART);
}










