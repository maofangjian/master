/*
 * @Author: chenyuan
 * @Date: 2025-07-03 15:04:33
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-07-09 11:22:40
 * @FilePath: \FreeRtos实验0703\13.FreeRTOS实验\Hardware\main.c
 * @Description: 
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
/*
 * @Author: chenyuan
 * @Date: 2025-05-22 13:51:32
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-06-26 19:16:50
 * @FilePath: \FreeRtos实验0621\13.FreeRTOS实验\Hardware\main.c
 * @Description: 
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#include "gd32f30x.h"
#include "led.h"
#include "systick.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "usart.h"
#include "timer.h"
#include "global.h"
#include "system.h"
#include "start.h"
#include "meter.h"
#include "gpio.h"
#include "order.h"
#include "flash.h"
#include "adc.h"
#include "timer.h"
#include "rtc.h"
#include "charge.h"
#include "relay_control.h"
#include "BswAbstr_Ec200s.h"
#include "BswSrv_Ec200s.h"
#include "BswDrv_Ec200s.h"
#include "gd32f30x_it.h"
#include "netmessage.h"
#include "mcard.h"
#include "i2s.h"
#include "App_CkbProto.h"
#include "Device_check.h"

TaskHandle_t Ec200sAbstrTask_Handle;
TaskHandle_t SrcEc200sAbstrTask_Handle;
void App_Main_Task(void *pvParameters)
{
    uint32_t time;
    uint32_t tick = 0;
    uint8_t login_cnt = 0;
    static uint8_t location_flag = 0; //定位信息发送标志位
    App_Audio_Data_Init();
    System_Start_Config();
    App_Bl0939_Init();
    App_OrderInit();
    taskENTER_CRITICAL(); //进入临界区
    xTaskCreate((TaskFunction_t)Abstr_Ec200sTask, "AbstrEc200sTask", Abs_Ec200s_TASK_STACK_SIZE, NULL, Abs_Ec200s_TASK_PRIORITY, &Ec200sAbstrTask_Handle);
    xTaskCreate((TaskFunction_t)Srv_Ec200s_Task, "SrvEc200sTask", Srv_Ec200s_TASK_STACK_SIZE, NULL, Srv_Ec200s_TASK_PRIORITY, &SrcEc200sAbstrTask_Handle);
    xTaskCreate((TaskFunction_t)App_ChargeTask, "App_ChargeTask", APP_CHARGE_TASK_STACK_SIZE, NULL, APP_CHARGE_TASK_PRIORITY, NULL);
    xTaskCreate((TaskFunction_t)App_audio_task, "App_voiceTask", APP_VOICE_TASK_STACK_SIZE, NULL, APP_VOICE_TASK_PRIORITY, NULL);

    xTaskCreate((TaskFunction_t)BswSrv_McardTask, "App_McardTask", APP_MCARD_TASK_STACK_SIZE, NULL, APP_MCARD_TASK_PRIORITY, NULL);
    xTaskCreate((TaskFunction_t)App_CkbProtoTask, "App_DisTask", APP_DIS_TASK_STACK_SIZE, NULL, APP_DIS_TASK_PRIORITY, NULL);
    //xTaskCreate((TaskFunction_t)App_TempTask, "App_TempTask", APP_TEMP_TASK_STACK_SIZE, NULL, APP_TEMP_TASK_PRIORITY, NULL);
    taskEXIT_CRITICAL(); //退出临界区
    while (1)
    {
        vTaskDelay(50);
        
        FEED_WDG();
        if(App_GetRtcCount() - time != 0)
        {
            tick++;
            time = App_GetRtcCount();
            App_Qrledtoggle();
            LOG("time :%d Heap size %x\r\n",time,xPortGetFreeHeapSize());
            App_CkbConnectStatuProc();
            if(SystemInfo.connect_server_flag)  //连接服务器
            {
                if(SystemInfo.onlineflag == 0) //还未登录
                {
                    ledstatus = LED_FLASH;
                    if(time%30 == 0)
                    {
                        login_cnt++;
                        App_StartUpMessage();
                        
                        
                    }
                    else if(login_cnt > 5)
                    {
                        //NVIC_SystemReset(); //五次连接不上重启
                    }

                }
                else    //设备登录成功
                {
                    login_cnt = 0;
                    ledstatus = LED_ON;
                    if(location_cnt == 0)
                    {
                        Abstr_Ec200sQcell();
                    }
                    if(location_flag == 0 && location_cnt >= 1)
                    {
                        location_flag = 1;
                        App_LocationMessage();
                    }
                    if(tick % 60 == 0 && upgrade_flag == 0)
                    {
                        App_HeartBeatSendMessage();
                    }
                    App_SendChargeStartNotify();
                    
                    if(tick % 15 == 0)
                    {  
                        App_SendOrderToBackstage();    //发送结束订单到后台
                    }
                    if((tick+1) % (30*60) == 0)
                    {
                        App_System_Order_Save(); //30分钟存储一次数据
                    }
                    if(tick % 10 == 0)
                    {
                        App_ChgCtrlMoneyCount();
                    }
                }
            }  
            App_ChgCtrlProcFuseStatus(); //充电过程中保险丝自检  
        } 
        // App_Fuse_Deal_Task();  //12点保险丝自检
        if (SystemInfo.connect_server_flag == TRUE)
        {
            App_McardProcess();
        } 
    }

    

}


int main(void)
{
	nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x08003000);
	/* start scheduler */
    system_init(); //系统配置初始化
    LOG("System start\r\n");
    xTaskCreate((TaskFunction_t)App_Main_Task, "App_MainTask", APP_MAIN_TASK_STACK_SIZE, NULL, APP_MAIN_TASK_PRIORITY, NULL);
    vTaskStartScheduler();
    while(1){
    }
}


