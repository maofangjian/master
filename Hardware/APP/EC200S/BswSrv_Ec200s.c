/*
 * @Author: chenyuan
 * @Date: 2025-04-08 17:19:29
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-04-11 18:43:38
 * @FilePath: \13.FreeRTOS实验\Hardware\APP\EC200S\BswSrv_Ec200s.c
 * @Description: 
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
// ------------------------------------------------------------------------
// @Project Includes
#include "global.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "BswSrv_Ec200s.h"
#include "fifo.h"
#include "sys.h"
#include "usart.h"
#include "BswAbstr_Ec200s.h"
#include "netmessage.h"
#include "cJSON.h"
uint8_t BswComSrvTempBuffer[1024] = {0}; //任务解析数据缓存



uint8_t BswSrv_Ec200sAnalyseGprsData(uint8_t* data)
{
	int8_t *src = NULL;
	uint32_t dataLen = 0;
    uint32_t client_id = 0;
    uint32_t msg_id = 0;
    uint8_t  topic[64] = {0};
    int  ret = 0;
    uint8_t *args = NULL;
    uint8_t  *gprsString = pvPortMalloc(1024);
    uint8_t  validDataBuff[850];
    memset(gprsString, 0, 1024);    //4G模块串口传输数据量比较大
    memset(validDataBuff, 0, 850);
    uint32_t time = App_GetRtcCount();
	if(data == NULL) 
    {
        ret = -1;
        goto FREE;
    }

	src = (int8_t *)strstr((void *)data,"+QMTRECV");				//查找GPRS数据的头信息
	if(src == NULL)
	{
        ret = -1;
		goto FREE;
	}
    sscanf((void *)src, "+QMTRECV: %d,%d,%[^,],%d,%[^,]", &client_id, &msg_id, topic, &dataLen, gprsString);
    BswSrv_hexStrToChar(&gprsString[1],validDataBuff);
    // LOG("data buff = %s.\n", validDataBuff);
    const char *json_start = strstr((char*)validDataBuff,"{\"device_id");
    if (json_start == NULL)
    {
        goto FREE;
    }
    uint16_t json_len = strlen(json_start);
    char *json_data = (char*)malloc(json_len+1);
    if (json_data == NULL)
    {
        goto FREE;
    }
    strncpy(json_data, json_start, json_len);  
	json_data[json_len] = '\0'; // 添加null终止符  
    cJSON *root = cJSON_Parse(json_data);

    cJSON *device_id = cJSON_GetObjectItem(root,"device_id");
    if (cJSON_IsString(device_id))
    {
        LOG("device_id :%s\r\n",device_id->valuestring);
    }

    cJSON *cmd = cJSON_GetObjectItem(root,"cmd");
    if (cJSON_IsNumber(cmd))
    {
        LOG("cmd :%d\r\n",cmd->valueint);
    }

    cJSON *timestamp = cJSON_GetObjectItem(root,"timestamp");
    if (cJSON_IsNumber(timestamp))
    {
        LOG("timestamp :%d\r\n",timestamp->valueint);
    }
    if((timestamp->valueint - time >= 30) && (timestamp->valueint > time))
    {
        LOG("now timestamp :%d,set :%d\r\n",time,timestamp->valueint);
        App_Set_Rtc_Time(timestamp->valueint);
    }
    cJSON *Data = cJSON_GetObjectItem(root,"args");
    args = cJSON_PrintUnformatted(Data);
    LOG("args :%s\r\n",args);
    App_NetMessagePro(args,cmd->valueint,sizeof(args));
    cJSON_Delete(root); 
    free(json_data); 
    free(args);
FREE:
    if (gprsString != NULL)
    {
        vPortFree(gprsString);
    }
	return ret;
}

void BswSrv_Ec200sUrcHandle(uint8_t* data)
{
    uint8_t *pos = NULL;
    uint32_t client_idx;
    uint32_t errcode;
    uint32_t pingResult;
    
    if (strstr((void *)data, "+QMTRECV") != NULL)
    {
        BswSrv_Ec200sAnalyseGprsData(data);
        return;
    }
    else if (strstr((void *)data,"+QMTSTAT") != NULL)    ////当MQTT链层状态变化时，客户端会关闭MQTT连接并上报此URC
    {
        pos = (uint8_t *)strstr((void *)data, "+QMTSTAT");
        sscanf((void *)pos, "+QMTSTAT: %d,%d", &client_idx, &errcode);
        LOG("recv +QMTSTAT, client_idx=%d, errcode=%d.\r\n", client_idx, errcode);

		if (AbstrEc200sInfo.ec200sState.Bits.ec200sHttpState == 0)
		{
			AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState = 0;
		}
    
    }
    else if (strstr((void *)data, "+QMTPING") != NULL)
    {
        pos = (uint8_t *)strstr((void *)data,"+QMTPING");
        sscanf((void *)pos, "+QMTPING: %d,%d", &client_idx, &pingResult);
        LOG("recv +QMTPING, client_idx=%d, pingResult=%d.\r\n", client_idx, pingResult);
        //当MQTT链层状态变化时，客户端会关闭MQTT连接并上报此URC
        if (pingResult == 1)
        {
            AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState = 0;
        }
    }
//    else if (strstr((void *)data, "+QTTS: 0") != NULL)
//    {
//        AbstrEc200sInfo.TTS_Status = 0;
//    }
//    else if (strstr((void *)data, "+QWTTS: 0") != NULL)
//    {
//        AbstrEc200sInfo.TTS_Status = 0;
//    }
    
}

void Srv_Ec200s_Task(void *pvParameters)
{
	uint8_t  receveData = 0;
	uint32_t gprsIndex = 0;
	uint8_t  gprsHeadFlag = 0;
    uint8_t  gprsFlag = 0;
	uint16_t gprsDataLen = 0;
	uint32_t gprsFlagIndex = 0;

	
    //App_NetProtoInit();
	while (1)
	{
		vTaskDelay(50);
		while(FIFO_S_Get(&Ec200_Fifo,&receveData) == TRUE)	//获取GPS或者GPRS数据
		{         
			if(gprsIndex == 0)
			{
				if(receveData == '+')
				{
					BswComSrvTempBuffer[gprsIndex] = receveData;
					gprsIndex++;
				}
			}
			else
			{
				BswComSrvTempBuffer[gprsIndex] = receveData;
				gprsIndex++;

				if(receveData == ':')
				{
					/*  +QMTRECV: <client_idx>,<msgid>,<topic>[,<payload_len>],<payload>
                        +QMTRECV: 0,1,"cs/cz21a/414674111535363137393950",40,"1234567890123456789012345678901234567890"
                    */
					if((strncmp((void *)BswComSrvTempBuffer,"+QMTRECV",strlen("+QMTRECV")) == 0)
                        || (strncmp((void *)BswComSrvTempBuffer,"+QMTSTAT",strlen("+QMTSTAT")) == 0)
                        || (strncmp((void *)BswComSrvTempBuffer,"+QMTPING",strlen("+QMTPING")) == 0))
//                        || (strncmp((void *)BswComSrvTempBuffer,"+QTTS",strlen("+QTTS")) == 0)
//                        || (strncmp((void *)BswComSrvTempBuffer,"+QWTTS",strlen("+QWTTS")) == 0))		//获取GPRS数据头
					{
                        gprsHeadFlag = 1;
						gprsFlag = 0;
					}
					else
					{
						gprsFlag = 0;
						gprsIndex = 0;
					}
				}

                if(gprsHeadFlag == 1)
                {
                    if(receveData == '\n')                      //获取GPRS头部的所有信息
					{   
                        gprsFlagIndex = gprsIndex;					//标识记录GPRS数据头部索引号
                        gprsHeadFlag = 0;
                        gprsFlag = 1;
                    }
                }
                
                
				if(gprsFlag == 1)
				{
					if(gprsIndex == (gprsDataLen + gprsFlagIndex))	//接收到完整的GPRS数据
					{
						gprsFlag = 0;
						gprsFlagIndex = 0;
						BswComSrvTempBuffer[gprsIndex] = 0;
						//EC200S_DEBUG("recive GPRS data : %s\r\n", BswComSrvTempBuffer);
                        //解析GPRS数据
                        BswSrv_Ec200sUrcHandle((void *)BswComSrvTempBuffer);
                        
						gprsIndex = 0;
						memset(BswComSrvTempBuffer, 0, sizeof(BswComSrvTempBuffer));
					}
				}
				
				if(gprsIndex >= sizeof(BswComSrvTempBuffer))
				{
					EC200S_DEBUG("recive data error\r\n");
					gprsIndex = 0;
				}
			}
		}
	}

}





