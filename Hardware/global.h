/*** 
 * @Author: chenyuan
 * @Date: 2025-03-17 14:15:29
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-07-08 11:26:12
 * @FilePath: \FreeRtos实验0703\13.FreeRTOS实验\Hardware\global.h
 * @Description: 
 * @
 * @Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#ifndef _GLOBAL_H_
#define _GLOBAL_H_
#include "gd32f30x.h"
#include "watchdog.h"
#include <string.h>
#include "stdio.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define SOFT_VERSION     "V1.0.4-S1"


#define ABS(X) ((X) >= 0 ? (X) : (-X))
#define LOG             printf
#define EC200S_DEBUG    printf
#define SYSTEM_DEBUG(fmt,...)
//#define EC200S_DEBUG(fmt,...)	printf("[%s] %s(%u):"fmt,BswDrv_GetCurrentTime(),__func__,__LINE__,##__VA_ARGS__)
#define  PORTMAX        12 //端口数
#define MQTT_TEST       2

#if (MQTT_TEST == 1)
//https://iot-tool.obs-website.cn-north-4.myhuaweicloud.com/
#define MQTT_SERVER_IP       "fc1d27ad9c.st1.iotda-device.cn-east-3.myhuaweicloud.com"
#define MQTT_SERVER_PORT     1883
#define MQTT_USER_COUNT      "67eab3b26b2d836c3fd0ae5c_charge"
#define MQTT_USER_PASSWORD   "30e98e0439484f0ddebf5309c0f573501cb6cf19bde27f2dd46d066659e6f9a5"  
#define MQTT_CLIENT_ID       "67eab3b26b2d836c3fd0ae5c_charge_0_1_2025042409"
#else if(MQTT_TEST == 2)
#define MQTT_SERVER_IP       "mqtt.lechongbao.cn"
#define MQTT_SERVER_PORT     1883
#define MQTT_USER_COUNT      "lechongbao"
#define MQTT_USER_PASSWORD   "pile666888"  
#define MQTT_CLIENT_ID       "67eab3b26b2d836c3fd0ae5c_charge_0_1_2025042109"
#endif

#define  CURRENT_OVER   16000  //过流保护值
#define  VOLTAGE_OVER   2600
#define  ON             1
#define  OFF            0
#define  ORDER_LEN      20 //订单长度
#define  SEGMENT_NUM_MAX 10		    //功率段分段总数
#define  MODULE_IMEI_LEN  15
#define  CHARGE_MAX_TIME  (12*60)

#define PORT_MAX_POWER       1500 //单口最大功率
#define TOTAL_MAX_POWER      5500 //整桩最大功率
#define CHARGE_FULL_POWER1   20 //充满判断功率 2w
#define CHARGE_FULL_POWER2   300 //充满判断功率 30w

#define FEED_WDG        Feed_Watchdog

#define TaskInit_Stack_SIZE								(64)
#define APP_MAIN_TASK_DELAY_MS                    	    (20)
#define APP_MAIN_TASK_PRIORITY						    (tskIDLE_PRIORITY+15)
#define APP_MAIN_TASK_STACK_SIZE					    (TaskInit_Stack_SIZE*20)

#define APP_CHARGE_TASK_PRIORITY						(tskIDLE_PRIORITY+10)
#define APP_CHARGE_TASK_STACK_SIZE					    (TaskInit_Stack_SIZE*20)

#define APP_TEMP_TASK_PRIORITY						    (tskIDLE_PRIORITY+10)
#define APP_TEMP_TASK_STACK_SIZE					    (TaskInit_Stack_SIZE*2)

#define Abs_Ec200s_TASK_PRIORITY            			(tskIDLE_PRIORITY+13)
#define Abs_Ec200s_TASK_STACK_SIZE                      (TaskInit_Stack_SIZE*10)

#define Srv_Ec200s_TASK_PRIORITY            			(tskIDLE_PRIORITY+14)
#define Srv_Ec200s_TASK_STACK_SIZE          			(TaskInit_Stack_SIZE*25)

#define APP_UPGRADE_TASK_PRIORITY            			(tskIDLE_PRIORITY+10)
#define APP_UPGRADE_TASK_STACK_SIZE          			(TaskInit_Stack_SIZE*10)

#define APP_MCARD_TASK_PRIORITY            			    (tskIDLE_PRIORITY+12)
#define APP_MCARD_TASK_STACK_SIZE          			    (TaskInit_Stack_SIZE*10)

#define APP_VOICE_TASK_PRIORITY            			    (tskIDLE_PRIORITY+10)
#define APP_VOICE_TASK_STACK_SIZE          			    (TaskInit_Stack_SIZE*7)

#define APP_DIS_TASK_PRIORITY            			    (tskIDLE_PRIORITY+12)
#define APP_DIS_TASK_STACK_SIZE          			    (TaskInit_Stack_SIZE*3)
#endif

