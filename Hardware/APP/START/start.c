/*
 * @Author: chenyuan
 * @Date: 2025-04-08 17:19:29
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-07-08 15:25:36
 * @FilePath: \FreeRtos实验0703\13.FreeRTOS实验\Hardware\APP\START\start.c
 * @Description: 
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#include "start.h"
#include "flash.h"
#include "global.h"
#include "sys.h"
#include "led.h"
#include "charge.h"
#include "cJSON.h"
System_Config SystemInfo;

//系统启动初始化
void System_Start_Config(void)
{
    memset(&SystemInfo,0,sizeof(SystemInfo));
    memcpy(SystemInfo.serverurl,MQTT_SERVER_IP,sizeof(MQTT_SERVER_IP));
    memcpy(SystemInfo.mqttcount,MQTT_USER_COUNT,sizeof(MQTT_USER_COUNT));
    memcpy(SystemInfo.mqttpassword,MQTT_USER_PASSWORD,sizeof(MQTT_USER_PASSWORD));
    SystemInfo.serverport = MQTT_SERVER_PORT;
    //Bsp_GetId(SystemInfo.device_id);
    //memcpy(SystemInfo.device_id,"00000000006",sizeof("00000000006"));
    LOG("Soft version :%s\r\n",SOFT_VERSION);
    LOG("serurl:%s  serverport:%d mqttcount:%s mqttpassword:%s \r\n",SystemInfo.serverurl,SystemInfo.serverport,SystemInfo.mqttcount,SystemInfo.mqttpassword);
    cJSON_InitHooks(NULL);
}

void App_System_Order_Save(void)
{
    uint8_t i = 0;
    for(i = 0; i < PORTMAX; i++)
    {
        SystemOrderInfo[i].crc = 0;
        CRC16((void *)&SystemOrderInfo[i],sizeof(SystemPortInfo_t) - 2,&(SystemOrderInfo[i].crc));
        
    } 
    App_Write_Flash((void*)&(SystemOrderInfo[0]),SYS_PORT_ADDR,sizeof(SystemPortInfo_t)*PORTMAX);
    App_Write_Flash((void*)&(SystemOrderInfo[0]),SYS_PORT_BACK_ADDR,sizeof(SystemPortInfo_t)*PORTMAX);
    App_Read_Flash((void*)&(SystemOrderInfo[0]),SYS_PORT_ADDR,sizeof(SystemPortInfo_t)*PORTMAX);
}

void App_System_Order_Load(void)
{
    uint8_t i = 0;
    uint16_t crc_check = 0;
    memset((void*)&(SystemOrderInfo[0]),0,sizeof(SystemPortInfo_t)*PORTMAX);
    App_Read_Flash((void*)&(SystemOrderInfo[0]),SYS_PORT_ADDR,sizeof(SystemPortInfo_t)*PORTMAX);
    for(i = 0; i < PORTMAX; i++)
    {
        crc_check = 0;
        CRC16((void *)&SystemOrderInfo[i],sizeof(SystemPortInfo_t) - 2,&crc_check);
        if(crc_check != SystemOrderInfo[i].crc)
        {
            crc_check = 0;
            memset((void*)&(SystemOrderInfo[i]),0,sizeof(SystemPortInfo_t));
            App_Read_Flash((void*)&(SystemOrderInfo[i]),(SYS_PORT_BACK_ADDR+i*sizeof(SystemPortInfo_t)),sizeof(SystemPortInfo_t)); 
            CRC16((void *)&SystemOrderInfo[i],sizeof(SystemPortInfo_t) - 2,&crc_check);
            if(crc_check != SystemOrderInfo[i].crc)
            {
                memset(&SystemOrderInfo[i],0,sizeof(SystemPortInfo_t));
                LOG("clear SystemPortInfo_t %d\r\n",i);
            }
        }
    }
    App_System_Order_Save();
    App_Restore_Charge_State();

}

