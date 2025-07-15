/*** 
 * @Author: 
 * @Date: 2025-03-17 15:04:48
 * @LastEditors: 
 * @LastEditTime: 2025-03-17 15:06:15
 * @FilePath: \13.FreeRTOS实验\Hardware\APP\CHECK\Device_check.h
 * @Description: 
 * @
 * @Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#ifndef _DEVICE_CHECK_H_
#define _DEVICE_CHECK_H_
#include "gd32f30x.h"
#include "charge.h"
#include "global.h"
#include "gpio.h"


//保险丝检测
uint8_t App_OpenOrder_Check(uint8_t port);
void App_Fuse_Deal_Task(void);
void App_ChgCtrlProcFuseStatus(void);
int App_Fuse_Check(uint8_t port,uint8_t check_all);
#endif

