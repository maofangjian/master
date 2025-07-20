// ------------------------------------------------------------------------
// @Project Includes

#include "BswAbstr_Ec200s.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "start.h"
#include "led.h"
#include "flash.h"
#include "global.h"
#include "fifo.h"
#include "BswDrv_Ec200s.h"
#include "BswSrv_Ec200s.h"
#include "netmessage.h"
#include "mcard.h"
#define SOCKET_ID (0)

uint8_t Location_status = 0;
uint8_t apn_info[64];
ABS_EC200S_INFO AbstrEc200sInfo = {0};									//相关数据信息
volatile ABS_EC200S_INIT_STATE AbstrEc200sGprsInitState = EC200S_RESET; //初始化状态
uint8_t AbstrEc200sBuffer[1024] = {0};							//Ec200s数据缓冲
uint8_t flash_data[1024];
UPGRADE_FW_HEAD_INFO UgradeHeadInfo = {0};								//EC200SFTP升级任务表和进度表
SemaphoreHandle_t AbstrEc200sMutex = NULL;
MQTT_INFO_t MqttInfo_t = {0};

SCELL location[4];
uint32_t	location_cnt = 0;
Upgrade_Info upgrade_info;
uint8_t upgrade_flag = 0;

static int32_t Abstr_Ec200sCheckReset(int8_t ok, uint8_t retry);
static int32_t Abstr_Ec200sCheckATV(int8_t ok, uint8_t retry);
static int32_t Abstr_Ec200sCheckATE(int8_t ok, uint8_t retry);
static int32_t Abstr_Ec200sCheckATI(int8_t ok, uint8_t retry);
#if defined(USE_TEST_ANTENNA)
static int32_t Abstr_Ec200sCheckATS0(int8_t ok, uint8_t retry);
#endif
static int32_t Abstr_Ec200sCheckCPIN(int8_t ok, uint8_t retry);
static int32_t Abstr_Ec200sCheckCCID(int8_t ok, uint8_t retry);
static int32_t Abstr_Ec200sCheckGsn(int8_t ok, uint8_t retry);

static int32_t Abstr_Ec200sCheckCGREG(int8_t ok, uint8_t retry);
static int32_t Abstr_Ec200sCheckCSQ(int8_t ok, uint8_t retry);
static int32_t Abstr_Ec200sCheckCOPS(int8_t ok, uint8_t retry);
static int32_t Abstr_Ec200sCheckQNWINFO(int8_t ok, uint8_t retry);
static int32_t Abstr_Ec200sQICSGP(int8_t ok, uint8_t retry);
static int32_t Abstr_Ec200sQIDEACT(int8_t ok, uint8_t retry);
static int32_t Abstr_Ec200sQIACT(int8_t ok, uint8_t retry);
static int32_t Abstr_Ec200sQMTCFGVersion(int8_t ok, uint8_t retry);
static int32_t Abstr_Ec200sQMTCFGDataformat(int8_t ok, uint8_t retry);
static int32_t Abstr_Ec200sQMTCFGViewmode(int8_t ok, uint8_t retry);
static int32_t Abstr_Ec200sQMTRecvmode(int8_t ok, uint8_t retry);
#ifdef EC200S_USE_TCP
static int32_t Abstr_Ec200sTcpConnect(int8_t ok, uint8_t retry);
#endif
static int32_t Abstr_Ec200sMqttCreat(int8_t ok, uint8_t retry);
static void GetTtsResult(int8_t *pStr);


const ABS_EC200S_INIT_T AbstrEc200sGprsInitTable[] =
{
		{NULL, (void *)"OK", 3000, 10, Abstr_Ec200sCheckReset},
        {(void *)"ATV1\r", (void *)"OK", 5000, 10, Abstr_Ec200sCheckATV },         //设置返回格式有OK
		{(void *)"ATE0\r", (void *)"OK", 5000, 10, Abstr_Ec200sCheckATE},		   //关闭回显
		{(void *)"ATI\r", (void *)"OK", 1000, 10, Abstr_Ec200sCheckATI},		   //显示产品ID信息
		{(void *)"AT+CPIN?\r", (void *)"READY", 1000, 100, Abstr_Ec200sCheckCPIN}, //指示用户是否需要密码
		{(void *)"AT+QCCID\r", (void *)"OK", 1000, 100, Abstr_Ec200sCheckCCID},	   //查询设备的SIM卡号

		{(void *)"AT+GSN\r", (void *)"OK", 1000, 10, Abstr_Ec200sCheckGsn},

#if defined(USE_TEST_ANTENNA)
		{(void *)"ATS0=1\r", (void *)"OK", 500, 10, Abstr_Ec200sCheckATS0},         //打开模块自动接听功能
        {(void *)"AT&W\r", (void *)"OK", 500, 10, Abstr_Ec200sCheckATS0},           //保存模块自动接听功能
#else
		{(void *)"AT+CGREG?\r", (void *)"OK", 3000, 1000, Abstr_Ec200sCheckCGREG},  //检查GSM网络注册状态
		{(void *)"AT+CSQ\r", (void *)"OK", 1000, 150, Abstr_Ec200sCheckCSQ},	    //查看信号强度
        {(void *)"AT+COPS?\r",(void *)"OK",1000,50,Abstr_Ec200sCheckCOPS},			//获取网络状态
        {(void *)"AT+QNWINFO\r",(void *)"OK",1000,50,Abstr_Ec200sCheckQNWINFO},		//获取网络状态
        {(void *)apn_info, (void *)"OK", 1000, 20, Abstr_Ec200sQICSGP },//设置APN
        {(void *)"AT+QIDEACT=1\r", (void *)"OK", 4000, 100, Abstr_Ec200sQIDEACT },  //禁用PDP上下文,关闭移动场景
        {(void *)"AT+QIACT=1\r", (void *)"OK", 50000, 10, Abstr_Ec200sQIACT },       //激活PDP上下文，打开移动场景   
        {(void *)"AT+QIACT?\r", (void *)".", 5000, 10, Abstr_Ec200sQIACT},         //获取IP
        
        {(void *)"AT+QMTCFG=\"version\",0,4\r", (void *)"OK", 3000, 10, Abstr_Ec200sQMTCFGVersion },  //配置MQTT协议版本
        {(void *)"AT+QMTCFG=\"dataformat\",0,1,1\r", (void *)"OK", 3000, 10, Abstr_Ec200sQMTCFGDataformat },  //配置MQTT数据格式，0字符串 1十六进制
        {(void *)"AT+QMTCFG=\"view/mode\",0,0\r", (void *)"OK", 3000, 10, Abstr_Ec200sQMTCFGViewmode },  //配置透传模式下MQTT数据的回显模式,0不回显
        {(void *)"AT+QMTCFG=\"recv/mode\",0,0,1\r", (void *)"OK", 3000, 10, Abstr_Ec200sQMTRecvmode },  //配置MQTT接收模式,URC上报,带长度
        
		{NULL, NULL, 1000, 0, Abstr_Ec200sMqttCreat},
#endif
};

void GetCcidSn(int8_t *pStr)
{
	int32_t i;
	int32_t flag = 0;

	for (i = 0; i < strlen((void *)pStr); i++)
	{
		switch (flag)
		{
		    case 0:
    			if ((0x0a == pStr[i]) || (0x0d == pStr[i]))
    			{
    				flag = 1;
    			}
			break;
		    case 1:
			    if ((0x30 <= pStr[i]) && (pStr[i] <= 0x39))
    			{
    				memcpy((void *)AbstrEc200sInfo.ec200sIccid, &pStr[i], ICCID_LEN);
    				AbstrEc200sInfo.ec200sIccid[ICCID_LEN] = 0;
    				memcpy((void *)SystemInfo.sim_id,(void *)AbstrEc200sInfo.ec200sIccid, ICCID_LEN);
					if (strstr(AbstrEc200sInfo.ec200sIccid,"898604") != NULL)
					{
						memcpy(apn_info,"AT+QICSGP=1,1,\"CMNET\",\"\",\"\",1\r",sizeof("AT+QICSGP=1,1,\"CMNET\",\"\",\"\",1\r"));
					}
					else
					{
						memcpy(apn_info,"AT+QICSGP=1,1,\"CTNET\",\"\",\"\",1\r",sizeof("AT+QICSGP=1,1,\"CMNET\",\"\",\"\",1\r"));
					}
					
    				EC200S_DEBUG("SIM ICCID: %s.\r\n", AbstrEc200sInfo.ec200sIccid);
    				return;
    			}
			break;
		}
	}
}



void GetCnum(int8_t *pStr)
{
	EC200S_DEBUG("CNUM=%s\r\n",pStr);
}
/*  */

void GetModuleImei(int8_t *pStr)
{
	int32_t i;
	int32_t flag = 0;

	for (i = 0; i < strlen((void *)pStr); i++)
	{
		switch (flag)
		{
		    case 0:
    			if ((0x0a == pStr[i]) || (0x0d == pStr[i]))/*  */
    			{
    				flag = 1;
    			}
			break;
		    case 1:
			    if ((0x30 <= pStr[i]) && (pStr[i] <= 0x39))
    			{
    				memset((void *)AbstrEc200sInfo.ec200sImei,0,MODULE_IMEI_LEN+1);
    				memcpy((void *)AbstrEc200sInfo.ec200sImei, &pStr[i], MODULE_IMEI_LEN);
    				memcpy(SystemInfo.device_id,AbstrEc200sInfo.ec200sImei,sizeof(AbstrEc200sInfo.ec200sImei));
    				LOG("device id  :%s\r\n",SystemInfo.device_id);
    				EC200S_DEBUG("Module imei: %s.\r\n", AbstrEc200sInfo.ec200sImei);
					Abstr_Ec200sMqttInit();
    				return;
    			}
			break;
		}
	}
}


void GetEdition(int8_t *pStr)
{
    //Quectel\r\nEC200S\r\nRevision: EC200SCNAAR01A05M16\r\nOK
    int8_t *p = (int8_t *)strstr((void *)pStr, "Revision: ");
	
	if (p)
	{
        memset(SystemInfo.ec200_version, 0, sizeof(SystemInfo.ec200_version));
		sscanf((void *)p, "Revision: %[^\r\n]", SystemInfo.ec200_version);
		EC200S_DEBUG("EC200S Revision : %s.\n", SystemInfo.ec200_version);

	}
}

int32_t GetCREG(int8_t *pStr)
{
	//+CGREG: 0,2
    //+CGREG: 2,1,"3747","A23C2",100
	int8_t *p = (int8_t *)strstr((void *)pStr, "+CGREG: ");
	int32_t state = 0;
	int32_t netstate = 0;
	if (p)
	{
		sscanf((void *)p, "+CGREG: %d,%d", (int *)&state, (int *)&netstate);
		EC200S_DEBUG("CGREG : state = %d,netstate = %d\r\n", state, netstate);

		if ((netstate == 1) || (netstate == 3) || (netstate == 5))
		{
			return OK;
		}
	}
	return FAIL;
}

void GetCsq(int8_t *pStr)
{
	//+CSQ: 31,0
	int32_t rssi = 0, ber = 0;

	int8_t *p = (int8_t *)strstr((void *)pStr, "+CSQ:");
	if (p)
	{
		sscanf((void *)p, "+CSQ:%d,%d", (int *)&rssi, (int *)&ber);
		AbstrEc200sInfo.ec200sRssi = rssi;
		SystemInfo.csq = rssi;
		EC200S_DEBUG("csq=%d.\r\n", rssi);
	}
}

void GetCell(int8_t *pStr)
{
/*
AT+QCELL? //Get the information of serving cells and neighbour cells
+QCELL: "servingcell","LTE",460,00,550b,d6b5c0,123,36
+QCELL: "neighbourcell intra","LTE",460,00,550b,5e05e2a,20,12
+QCELL: "neighbourcell inter","LTE",460,00,550b,5c4ef29,121,25
+QCELL: "neighbourcell","GSM",460,00,550b,d89,35,46
+QCELL: "neighbourcell","GSM",460,00,550b,34b8,10,45
OK	
*/
	EC200S_DEBUG("GetCell = %s\r\n", pStr);
	int8_t *p1 = (int8_t *)strstr((void *)pStr, "+QCELL: \"servingcell\",\"LTE\"");
	if (p1)
	{
		EC200S_DEBUG("GetCell p1 = %s\r\n", p1);
		sscanf((void *)p1, "+QCELL: \"servingcell\",\"LTE\",%d,%d,%x,%x", 
			(uint32_t *)&location[0].mcc, (uint32_t *)&location[0].mnc, (uint32_t *)&location[0].tac, (uint32_t *)&location[0].cell_id);
		location[0].signal = SystemInfo.csq;
		
		EC200S_DEBUG("p1 mcc=%d,mnc=%d,lac=%x,cellid=%x\r\n",
		location[0].mcc, location[0].mnc, location[0].tac, location[0].cell_id);
	}
	
	int8_t *p2 = (int8_t *)strstr((void *)pStr, "+QCELL: \"neighbourcell inter\",\"LTE\"");
	if (p2)
	{
		EC200S_DEBUG("GetCell p2 = %s\r\n", p2);
		sscanf((void *)p2, "+QCELL: \"neighbourcell inter\",\"LTE\",%d,%d,%x,%x", 
			(uint32_t *)&location[1].mcc, (uint32_t *)&location[1].mnc, (uint32_t *)&location[1].tac, (uint32_t *)&location[1].cell_id);
		location[1].signal = SystemInfo.csq;
		
		EC200S_DEBUG("p2 mcc=%d,mnc=%d,lac=%x,cellid=%x\r\n",
		location[1].mcc, location[1].mnc, location[1].tac, location[1].cell_id);
	}
	
	p2 += 10;
	int8_t *p3 = (int8_t *)strstr((void *)p2, "+QCELL: \"neighbourcell inter\",\"LTE\"");
	if (p3)
	{
		EC200S_DEBUG("GetCell p3 = %s\r\n", p3);
		sscanf((void *)p3, "+QCELL: \"neighbourcell inter\",\"LTE\",%d,%d,%x,%x", 
			(uint32_t *)&location[2].mcc, (uint32_t *)&location[2].mnc, (uint32_t *)&location[2].tac, (uint32_t *)&location[2].cell_id);
		location[2].signal = SystemInfo.csq;
		
		EC200S_DEBUG("p3 mcc=%d,mnc=%d,lac=%x,cellid=%x\r\n",
		location[2].mcc, location[2].mnc, location[2].tac, location[2].cell_id);
	}
	
	p3 += 10;
	int8_t *p4 = (int8_t *)strstr((void *)p3, "+QCELL: \"neighbourcell inter\",\"LTE\"");
	if (p4)
	{
		EC200S_DEBUG("GetCell p4 = %s\r\n", p4);
		sscanf((void *)p4, "+QCELL: \"neighbourcell inter\",\"LTE\",%d,%d,%x,%x", 
			(uint32_t *)&location[3].mcc, (uint32_t *)&location[3].mnc, (uint32_t *)&location[3].tac, (uint32_t *)&location[3].cell_id);
		location[3].signal = SystemInfo.csq;
		
		EC200S_DEBUG("p4 mcc=%d,mnc=%d,lac=%x,cellid=%x\r\n",
		location[3].mcc, location[3].mnc, location[3].tac, location[3].cell_id);
	}
	
	location_cnt++;
}

//对相应的AT指令，GPRS模块回复的ACK进行判断
int32_t Abstr_Ec200sCheckResQcell(int8_t *cmd, int8_t *res, uint16_t tmo)
{
	int8_t *ret = NULL;
	uint8_t c;
	int32_t cnt = 0;
	int32_t retv = FAIL;
	
    memset(AbstrEc200sBuffer, 0, sizeof(AbstrEc200sBuffer));
	for (uint16_t time = 0; time < tmo; time += 100)
	{ //等待时间
		vTaskDelay(100);
		FEED_WDG();
        cnt = 0;
        c = 0;
		while (BswDrv_Ec200s_GetCharFromFifo(&c) == TRUE)
		{
			//存储接收到的数据
			if (c)
			{
				// 如果缓冲区有大量的数据，tmo 实际没有起到效果
				if ((sizeof(AbstrEc200sBuffer) - 1) > cnt)
				{
					AbstrEc200sBuffer[cnt++] = c;
				}
				else
				{
					EC200S_DEBUG("cnt=%d,error.\r\n", cnt);
					break;
				}
			}
		}

		AbstrEc200sBuffer[cnt] = 0;
		ret = (int8_t *)strstr((void *)AbstrEc200sBuffer, (void *)"ERROR");
		if (ret)
		{
			//EC200S_DEBUG("cmd=%s,ACK:%s\r\n", cmd, AbstrEc200sBuffer);       todo by liutao
			retv = FAIL;
			break;
		}
		ret = (int8_t *)strstr((void *)AbstrEc200sBuffer, (void *)res); //判断是否收到相关的回复
		if (ret)
		{
			retv = OK;
			if (NULL != cmd)
			{
				if (strstr((void *)cmd, "AT+QCELL?")) //基站定位信息
				{
					EC200S_DEBUG("<EC200S->CPU>---->ACK=%s", AbstrEc200sBuffer);
					GetCell((void *)AbstrEc200sBuffer);
				}
			}
			break;
		}
	}

	if (NULL != cmd)
    {
        //EC200S_DEBUG("<EC200S->CPU>cmd=%s----->ACK=%s", cmd, AbstrEc200sBuffer);
    }
    else
    {
        //EC200S_DEBUG("<EC200S->CPU>---->ACK=%s", AbstrEc200sBuffer);
    }
	printf("\r\n");
	return retv;
}


int32_t _Abstr_Ec200sQcellSendCmd(int8_t *cmd, int8_t *ack, uint16_t waittime, int32_t flag)
{
	uint8_t res = 1;

    BswDrv_Ec200s_FifoFlush();
	BswDrv_Ec200sUartSend((void *)cmd, strlen((void *)cmd)); //发送相应的AT指令

	if ((ack == NULL) || (waittime == 0))
	{
		return OK;
	}

	if (OK == Abstr_Ec200sCheckResQcell(cmd, ack, waittime))
	{
		res = OK; /*check success, retrun 0*/
	}
	else
	{
		res = (1 == flag) ? 0 : 1;
	}
	return res;
}

//flag = 1是发>   对AT指令进行发送
int32_t Abstr_Ec200sQcellSendCmd(int8_t *cmd, int8_t *ack, uint16_t waittime, int32_t flag)
{
    xSemaphoreTakeRecursive(AbstrEc200sMutex, portMAX_DELAY);
	int32_t res = 1;

	res = _Abstr_Ec200sQcellSendCmd(cmd, ack, waittime, flag);
	xSemaphoreGiveRecursive(AbstrEc200sMutex);
	return res;
}

int8_t Abstr_Ec200sQcell(void)
{
	if(location_cnt > 10)
	{
		return OK;
	}
	if(AbstrEc200sInfo.ec200sState.Bits.ec200sSendDataState == 1)
	{
		EC200S_DEBUG("Ec200s is send data.\r\n");
		return OK;
	}
	Abstr_Ec200sQcellSendCmd((void *)"AT+QCELL = 1\r", (void *)"OK", 10000, 0);
	Delay_ms(1000);
	Abstr_Ec200sQcellSendCmd((void *)"AT+QCELL?\r", (void *)"OK", 10000, 0);
	return OK;
}


int32_t GetSocketState(int8_t *pStr)
{
	//+QISTATE: 0,"TCP","106.55.155.243",10020,7855,2,1,0,1,"uart1"
	char *p = (char *)strstr((void *)pStr, "+QISTATE:");
	EC200S_DEBUG("GetSocketState:%s\r\n",pStr);
	int socketState = 0;
	if(p)
	{
		char *temp = strtok(p,",");
		for(uint8_t i = 0;i < 5;i++)
		{
			temp = strtok(NULL,",");
		}
		if(temp)
		{
			socketState = atoi(temp);
			EC200S_DEBUG("socketState=%d\r\n",socketState);
			if(socketState == 2)
			{
				AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState = 1;
				return OK;
			}
			else
			{
				AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState = 0;
			}
		}
		else
		{
			EC200S_DEBUG("no socketState,reconnect\r\n");
			AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState = 0;
		}
	}
	else
	{
		AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState = 0;
	}
	return FAIL;
}

static void GetTtsResult(int8_t *pStr)
{ 
	int8_t *p;
	
	p = (int8_t *)strstr((void *)pStr, "ERROR");
	if (p) 
    {
//		AbstrEc200sInfo.TTS_Status = 0;
		EC200S_DEBUG("TTS play err.\r\n");
	}
} 


//对相应的AT指令，GPRS模块回复的ACK进行判断
int32_t Abstr_Ec200sCheckRes(int8_t *cmd, int8_t *res, uint16_t tmo)
{
	int8_t *ret = NULL;
	uint8_t c;
	int32_t cnt = 0;
	int32_t retv = FAIL;
    memset(AbstrEc200sBuffer, 0, sizeof(AbstrEc200sBuffer));
	for (uint16_t time = 0; time < tmo; time += 10)
	{ //等待时间
		vTaskDelay(50);
		FEED_WDG();
        cnt = 0;
        c = 0;
		while (BswDrv_Ec200s_GetCharFromFifo(&c) == TRUE)
		{
			//存储接收到的数据
			if (c)
			{
				//printf("%c\r\n",c);
				// 如果缓冲区有大量的数据，tmo 实际没有起到效果
				if ((sizeof(AbstrEc200sBuffer) - 1) > cnt)
				{
					AbstrEc200sBuffer[cnt++] = c;
				}
				else
				{
					EC200S_DEBUG("cnt=%d,error.\r\n", cnt);
					break;
				}
			}
		}
		AbstrEc200sBuffer[cnt] = 0;
		ret = (int8_t *)strstr((void *)AbstrEc200sBuffer, (void *)"ERROR");
		//EC200S_DEBUG("cmd=%s,ACK:%s\r\n", cmd, AbstrEc200sBuffer);
		if (ret)
		{
			//EC200S_DEBUG("cmd=%s,ACK:%s\r\n", cmd, AbstrEc200sBuffer);       todo by liutao
			retv = FAIL;
			break;
		}
		ret = (int8_t *)strstr((char *)AbstrEc200sBuffer, (char *)res); //判断是否收到相关的回复
		if (ret)
		{
			retv = OK;
			if (NULL != cmd)
			{
				if (strstr((void *)cmd, "AT+CGREG"))
				{
					retv = GetCREG((void *)AbstrEc200sBuffer);
				}
				else if (strstr((void *)cmd, "AT+CSQ")) //判断命令是否是获取GPRS的信号值
				{
					GetCsq((void *)AbstrEc200sBuffer);
				}
				else if (strstr((void *)cmd, "AT+QCCID")) //判断命令是否是获取ICCID
				{
					GetCcidSn((void *)AbstrEc200sBuffer);
				}
				/*else if (strstr((void *)cmd, "AT+CNUM")) //判断命令是否是获取电话号码
				{
					GetCnum((void *)AbstrEc200sBuffer);
				}*/
				else if (strstr((void *)cmd, "AT+GSN")) //判断命令是否是获取电话号码
				{
					GetModuleImei((void *)AbstrEc200sBuffer);
				}
				else if (strstr((void *)cmd, "ATI")) //判断命令是否是获取版本信息
				{
					GetEdition((void *)AbstrEc200sBuffer);
				}
				else if (strstr((void *)cmd, "AT+QISTATE"))
				{
					retv = GetSocketState((void *)AbstrEc200sBuffer);
				}
                else if (strstr((void *)cmd, "AT+QTTS"))
                {
                    GetTtsResult((void *)AbstrEc200sBuffer);
                }
                else if (strstr((void *)cmd, "AT+QWTTS"))
                {
                    GetTtsResult((void *)AbstrEc200sBuffer);
                }
//				else if (strstr((void *)cmd, "AT+QCELL?")) //基站定位信息
//				{
//					EC200S_DEBUG("<EC200S->CPU>---->ACK=%s", AbstrEc200sBuffer);
//					GetCell((void *)AbstrEc200sBuffer);
//				}
			}
			break;
		}
	}

	if (NULL != cmd)
    {
        //EC200S_DEBUG("<EC200S->CPU>cmd=%s----->ACK=%s", cmd, AbstrEc200sBuffer);
    }
    else
    {
        //EC200S_DEBUG("<EC200S->CPU>---->ACK=%s", AbstrEc200sBuffer);
    }
	printf("\r\n");
	return retv;
}

int32_t _Abstr_Ec200sSendCmd(int8_t *cmd, int8_t *ack, uint16_t waittime, int32_t flag)
{
	uint8_t res = 1;

    BswDrv_Ec200s_FifoFlush();
	BswDrv_Ec200sUartSend((void *)cmd, strlen((void *)cmd)); //发送相应的AT指令

	if ((ack == NULL) || (waittime == 0))
	{
		return OK;
	}
	if (OK == Abstr_Ec200sCheckRes(cmd, ack, waittime))
	{
		res = OK; /*check success, retrun 0*/
	}
	else
	{
		res = (1 == flag) ? 0 : 1;
	}
	return res;
}

//flag = 1是发>   对AT指令进行发送
int32_t Abstr_Ec200sSendCmd(int8_t *cmd, int8_t *ack, uint16_t waittime, int32_t flag)
{
    xSemaphoreTakeRecursive(AbstrEc200sMutex, portMAX_DELAY);
	int32_t res = 1;
	LOG("cmd :%s\r\n",cmd);
	res = _Abstr_Ec200sSendCmd(cmd, ack, waittime, flag);
	xSemaphoreGiveRecursive(AbstrEc200sMutex);
	return res;
}


int32_t Abstr_CheckModeCmd(int8_t ok, uint8_t retry, uint16_t delay)
{
	if (ok == 0)
	{
		AbstrEc200sGprsInitState++;
		return OK;
	}
	else if (retry > 10)
	{
		AbstrEc200sGprsInitState = EC200S_RESET;
	}
	return FAIL;
}

static int32_t Abstr_Ec200sSendRepeatCmd(int8_t *cmd, int8_t *ack, uint16_t delay)
{
	uint8_t i = 0;
	uint8_t sendCmdFlag = 0;

	for (i = 0; i < 10; i++)
	{
		if (!Abstr_Ec200sSendCmd(cmd, ack, delay, 0))
		{
			sendCmdFlag = 1;
			break;
		}
		vTaskDelay(400);
	}
	if (sendCmdFlag == 0)
	{
		return FAIL;
	}
	return OK;
}

static int32_t Abstr_Ec200sCheckReset(int8_t ok, uint8_t retry)
{
	int8_t i;

	LOG("reset Ec200s module\r\n");
	AbstrEc200sInfo.ec200sState.Bits.ec200sPowerState = 0;
	BswDrv_Ec200sPowerOff();
	LOG("Drv_Ec200sPowerOff\r\n");
	BswDrv_Ec200sPowerOn();
	LOG("Drv_Ec200sPowerOn\r\n");
	
//	while(1) vTaskDelay(1000);

	for (i = 0; i < 5; i++)
	{
		if (Abstr_Ec200sSendCmd((void *)"AT\r\n", (void *)"OK", AbstrEc200sGprsInitTable[EC200S_RESET].wait, 0) == 0)
		{
			LOG("4G reset ok \r\n");
			AbstrEc200sGprsInitState++;
			AbstrEc200sInfo.ec200sState.Bits.ec200sPowerState = 1;
			return OK;
		}
		LOG("4G reset fail.\r\n");
		vTaskDelay(1000);
	}
	return FAIL;
}

static int32_t Abstr_Ec200sCheckATV(int8_t ok, uint8_t retry)
{
	EC200S_DEBUG("in.\r\n");
	return Abstr_CheckModeCmd(ok, retry, 0);
}
static int32_t Abstr_Ec200sCheckATE(int8_t ok, uint8_t retry)
{
	EC200S_DEBUG("in.\r\n");
	return Abstr_CheckModeCmd(ok, retry, 0);
}
static int32_t Abstr_Ec200sCheckATI(int8_t ok, uint8_t retry)
{
	EC200S_DEBUG("in.\r\n");
	return Abstr_CheckModeCmd(ok, retry, 0);
}
#if defined(USE_TEST_ANTENNA)
static int32_t Abstr_Ec200sCheckATS0(int8_t ok, uint8_t retry)
{
	EC200S_DEBUG("in.\r\n");
	return Abstr_CheckModeCmd(ok, retry, 0);
}
#endif
static int32_t Abstr_Ec200sCheckCPIN(int8_t ok, uint8_t retry)
{
	EC200S_DEBUG("in.\r\n");
	return Abstr_CheckModeCmd(ok, retry, 0);
}

static int32_t Abstr_Ec200sCheckGsn(int8_t ok, uint8_t retry)
{
	EC200S_DEBUG("in.\r\n");
	return Abstr_CheckModeCmd(ok, retry, 0);
}


static int32_t Abstr_Ec200sCheckCCID(int8_t ok, uint8_t retry)
{
	EC200S_DEBUG("in.\r\n");
	return Abstr_CheckModeCmd(ok, retry, 0);
}
static int32_t Abstr_Ec200sCheckCGREG(int8_t ok, uint8_t retry)
{
	EC200S_DEBUG("in.\r\n");
#if 1
	return Abstr_CheckModeCmd(ok, retry, 0);
#else
	AbstrEc200sGprsInitState = EC200S_RESET;
	return FAIL;
#endif
}
static int32_t Abstr_Ec200sCheckCSQ(int8_t ok, uint8_t retry)
{
	EC200S_DEBUG("in.\r\n");
	return Abstr_CheckModeCmd(ok, retry, 0);
}
static int32_t Abstr_Ec200sCheckCOPS(int8_t ok, uint8_t retry)
{
	EC200S_DEBUG("in.\n");
    return Abstr_CheckModeCmd(ok, retry, 0);
}
static int32_t Abstr_Ec200sCheckQNWINFO(int8_t ok, uint8_t retry)
{
	EC200S_DEBUG("in.\n");
    return Abstr_CheckModeCmd(ok, retry, 0);
}
static int32_t Abstr_Ec200sQICSGP(int8_t ok, uint8_t retry)
{
	EC200S_DEBUG("in.\n");
    return Abstr_CheckModeCmd(ok, retry, 0);
}
static int32_t Abstr_Ec200sQIACT(int8_t ok, uint8_t retry)
{
	EC200S_DEBUG("in.\n");
    return Abstr_CheckModeCmd(ok, retry, 0);
}
static int32_t Abstr_Ec200sQIDEACT(int8_t ok, uint8_t retry)
{
	EC200S_DEBUG("in.\n");
    return Abstr_CheckModeCmd(ok, retry, 0);
}
static int32_t Abstr_Ec200sQMTCFGVersion(int8_t ok, uint8_t retry)
{
    EC200S_DEBUG("in.\n");
    return Abstr_CheckModeCmd(ok, retry, 0);
}
static int32_t Abstr_Ec200sQMTCFGDataformat(int8_t ok, uint8_t retry)
{
    EC200S_DEBUG("in.\n");
    return Abstr_CheckModeCmd(ok, retry, 0);
}
static int32_t Abstr_Ec200sQMTCFGViewmode(int8_t ok, uint8_t retry)
{
    EC200S_DEBUG("in.\n");
    return Abstr_CheckModeCmd(ok, retry, 0);
}
static int32_t Abstr_Ec200sQMTRecvmode(int8_t ok, uint8_t retry)
{
    EC200S_DEBUG("in.\n");
    return Abstr_CheckModeCmd(ok, retry, 0);
}

//返回 0成功 通过GPRS模块发送数据
int32_t Abstr_Ec200sSendData(int8_t *data, int32_t len, int8_t *ack, uint16_t waittime)
{
	int32_t res = 1;

	BswDrv_Ec200sUartSend((void *)data, len);
	if ((ack == NULL) || (waittime == 0))
	{
		return OK;
	}

	if (OK == Abstr_Ec200sCheckRes(NULL, ack, waittime))
	{
		res = 0;
	}
	return res;
}


 

int BswAbstr_Ec200sTtsPlay(uint8_t *text)
{ 		
	int8_t 	temp[256] = {0};      //指令进行的缓存
	
	if((sizeof(text) > 220) || (AbstrEc200sGprsInitState <= EC200S_CREG))
		return FAIL;
 
	memset(temp, 0, sizeof(temp)); 

    //AT+QTTS=2,“欢迎使用搜电，祝你生活愉快”    //开始播放以GBK编码的文本
    sprintf((void *)temp, "AT+QTTS=2,\"%s\"\r", text); 
    Abstr_Ec200sSendCmd((void *)temp, (void *)"OK", 2000, 0);
 
    return OK;
}

void Abstr_Ec200sMqttInit(void)
{
    MqttInfo_t.client_idx = 0;
#if(MQTT_TEST == 1)
	//uint8_t topic[] = "a1BZ0s5Db5u/chenyuantest/user/mqtttopic_test";
	memcpy(MqttInfo_t.topic_pub_req, "ds/imel/user", strlen("ds/imel/user"));
    memcpy(MqttInfo_t.topic_sub_req, "ds/imel/user", strlen("ds/imel/user"));
	memcpy(MqttInfo_t.topic_pub_res, "ds/imel/user", strlen("ds/imel/user"));
	memcpy(MqttInfo_t.topic_sub_res, "ds/imel/user", strlen("ds/imel/user"));
    memcpy(MqttInfo_t.client_id, MQTT_CLIENT_ID, strlen(MQTT_CLIENT_ID));
	// memcpy(MqttInfo_t.topic_pub_req,topic,sizeof(topic));
	// memcpy(MqttInfo_t.topic_sub_req,topic,sizeof(topic));
#else if(MQTT_TEST == 2)
    memcpy(MqttInfo_t.client_id, SystemInfo.device_id, strlen(SystemInfo.device_id));
	sprintf(MqttInfo_t.topic_pub_req,"cp/lechongbao/topic/%s",SystemInfo.device_id);
	sprintf(MqttInfo_t.topic_sub_req,"cs/lechongbao/topic/%s",SystemInfo.device_id);
	sprintf(MqttInfo_t.topic_pub_res,"cp/lechongbao/topic/%s",SystemInfo.device_id);
	sprintf(MqttInfo_t.topic_sub_res,"cs/lechongbao/topic/%s",SystemInfo.device_id);
#endif
    sprintf((void*)&MqttInfo_t.host_name, "%s", SystemInfo.serverurl);
    MqttInfo_t.port = SystemInfo.serverport;
    sprintf((void*)&MqttInfo_t.username, "%s", SystemInfo.mqttcount);
    sprintf((void*)&MqttInfo_t.password, "%s", SystemInfo.mqttpassword);
    
    LOG("topic_pub_req = %s.\n", MqttInfo_t.topic_pub_req);
    LOG("topic_sub_req = %s.\n", MqttInfo_t.topic_sub_req);
    LOG("client_id = %s.\n", MqttInfo_t.client_id);
    LOG("host_name = %s.\n", MqttInfo_t.host_name);
    LOG("port = %d.\n", MqttInfo_t.port);
    LOG("username = %s.\n", MqttInfo_t.username);
    LOG("password = %s.\n\n", MqttInfo_t.password);
}


static int Abstr_Ec200sMqttOpen(uint8_t clientidx, uint8_t *ipaddr, int port)
{
    int8_t cmd_req[256] = {0};
	int8_t cmd_ack[64] = {0};

	sprintf((void *)cmd_ack, "OK", clientidx);
	sprintf((void *)cmd_req, "AT+QMTOPEN=%d,\"%s\",%d\r", clientidx, ipaddr, (int)port);
    EC200S_DEBUG("cmd_ack=%s, cmd_req=%s.\n", cmd_ack, cmd_req);
	if (!Abstr_Ec200sSendCmd(cmd_req, cmd_ack, 20000, 0))
	{
		EC200S_DEBUG("[INFO] MQTT link %d open success,ipaddr=%s,port=%d.\r\n", clientidx, ipaddr, port);
		return OK;
	}
	else
	{
		EC200S_DEBUG("[INFO] MQTT link %d open failure.\r\n", clientidx);
		return FAIL;
	}
}

static int Abstr_Ec200sMqttOpenCheck(void)
{
    int8_t cmd_req[128] = {0};
	int8_t cmd_ack[64] = {0};

	sprintf((void *)cmd_ack, "OK");
	sprintf((void *)cmd_req, "AT+QMTOPEN?\r");
    EC200S_DEBUG("cmd_ack=%s, cmd_req=%s.\n", cmd_ack, cmd_req);
	if (!Abstr_Ec200sSendCmd(cmd_req, cmd_ack, 5000, 0))
	{
		EC200S_DEBUG("MQTT open success.\r\n");
		return OK;
	}
	else
	{
		EC200S_DEBUG("MQTT open failure.\r\n");
		return FAIL;
	}
}

static int Abstr_Ec200sMqttClose(uint8_t clientidx)
{
    int8_t cmd_req[128] = {0};
	int8_t cmd_ack[64] = {0};

	sprintf((void *)cmd_ack, "+QMTCLOSE: %d,0", clientidx);
	sprintf((void *)cmd_req, "AT+QMTCLOSE=%d\r", clientidx);
    EC200S_DEBUG("cmd_ack=%s, cmd_req=%s.\n", cmd_ack, cmd_req);
	if (!Abstr_Ec200sSendCmd(cmd_req, cmd_ack, 5000, 0))
	{
		EC200S_DEBUG("MQTT link %d close success\r\n", clientidx);
		return OK;
	}
	else
	{
		EC200S_DEBUG("MQTT link %d close failure.\r\n", clientidx);
		return FAIL;
	}
}


static int Abstr_Ec200sMqttConnect(uint8_t clientidx, uint8_t *clientId, uint8_t *username, uint8_t *password)
{
    int8_t cmd_req[256] = {0};
	int8_t cmd_ack[64] = {0};
	sprintf((void *)cmd_ack, "OK");
	sprintf((void *)cmd_req, "AT+QMTCONN=%d,\"%s\",\"%s\",\"%s\"\r", clientidx, clientId, username, password);
    EC200S_DEBUG("cmd_ack=%s, cmd_req=%s.\n", cmd_ack, cmd_req);
	if (!Abstr_Ec200sSendCmd(cmd_req, cmd_ack, 5000, 0))
	{
		EC200S_DEBUG("MQTT connect success.\r\n");
		return OK;
	}
	else
	{
		EC200S_DEBUG("MQTT connect failure.\r\n");
		return FAIL;
	}
}

static int Abstr_Ec200sMqttDisconnect(uint8_t clientidx)
{
    int8_t cmd_req[128] = {0};
	int8_t cmd_ack[64] = {0};

	sprintf((void *)cmd_ack, "+QMTDISC: %d,0", clientidx);
	sprintf((void *)cmd_req, "AT+QMTDISC=%d\r", clientidx);
    EC200S_DEBUG("cmd_ack=%s, cmd_req=%s.\n", cmd_ack, cmd_req);
	if (!Abstr_Ec200sSendCmd(cmd_req, cmd_ack, 5000, 0))
	{
		EC200S_DEBUG("MQTT disconnect success\r\n");
		return OK;
	}
	else
	{
		EC200S_DEBUG("MQTT disconnect failure.\r\n");
		return FAIL;
	}
}

static int Abstr_Ec200sMqttSub(uint8_t clientidx, uint8_t *subtopic)
{
    int8_t cmd_req[128] = {0};
	int8_t cmd_ack[64] = {0};

	sprintf((void *)cmd_ack, "OK");
	sprintf((void *)cmd_req, "AT+QMTSUB=%d,1,\"%s\",1\r", clientidx, subtopic);
    EC200S_DEBUG("cmd_ack=%s, cmd_req=%s.\n", cmd_ack, cmd_req);
	if (!Abstr_Ec200sSendCmd(cmd_req, cmd_ack, 20000, 0))
	{
		EC200S_DEBUG("MQTT sub topic=%s success\r\n", subtopic);
		return OK;
	}
	else
	{
		EC200S_DEBUG("MQTT sub topic=%s failure.\r\n", subtopic);
		return FAIL;
	}
}

//mqtt publish消息
int32_t Abstr_Ec200sMqttPublish(uint8_t *data, uint16_t len, QOS_LEVEL qos,uint8_t req)
{
	int8_t cmd_req[128] = {0};
	int8_t cmd_ack[16] = {0};
	int32_t res = 0;
	uint8_t i = 0;
	static uint16_t msgid = 0;
	uint8_t retain = 0; //服务器不保存该消息

	if (qos == 0)
	{
        msgid = 0;
	}
	else
	{
        msgid++;
	}

	AbstrEc200sInfo.ec200sState.Bits.ec200sSendDataState = 1;
	xSemaphoreTakeRecursive(AbstrEc200sMutex, portMAX_DELAY);
	for (i=0; i<3; i++) //每条数据重复发送三次
	{
		if (AbstrEc200sInfo.ec200sState.Bits.ec200sFtpState == 1) //正在进行FTP下载
		{
			EC200S_DEBUG("FTP Downloading, do not send data.\r\n");
			res = 0;
			break;
		}

		sprintf((void *)cmd_ack, "OK");
		if(req == DEVICE_REQ)
		{
			sprintf((void *)cmd_req, "AT+QMTPUBEX=%d,%d,%d,%d,\"%s\",%d\r",     \
		                        	MqttInfo_t.client_idx, msgid, qos, retain, MqttInfo_t.topic_pub_req, len);   \
		}
		else if(req == DEVICE_RES)
		{
			sprintf((void *)cmd_req, "AT+QMTPUBEX=%d,%d,%d,%d,\"%s\",%d\r",     \
		                        	MqttInfo_t.client_idx, msgid, qos, retain, MqttInfo_t.topic_pub_res, len);
		}
		res = Abstr_Ec200sSendCmd((void *)cmd_req, (void *)">", 1000, 0); //发送需要发送数据指令
		if (OK != res)
		{
			EC200S_DEBUG("mqtt send cmd not recv >.\r\n");
			continue;
		}

        //EC200S_DEBUG("Abstr_Ec200sSendData len=%d, data=%s.\r\n", len, data);
        //PrintfData("Abstr_Ec200sSendData: ", data, len);
		res = Abstr_Ec200sSendData((void *)data, len, (void *)cmd_ack, 15000);
		if (OK == res)
		{
            EC200S_DEBUG("EC200S send success.\r\n");
			break;
		}
		else
		{
			continue;
		}
	}
	xSemaphoreGiveRecursive(AbstrEc200sMutex);

	AbstrEc200sInfo.ec200sState.Bits.ec200sSendDataState = 0;
	if(res != OK)
	{
		EC200S_DEBUG("send data error, reconnect mqtt.\r\n");
		AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState = 0;
		Abstr_Ec200s_SendNotify();
	}
	return res;
}

//MQTT创建连接
static int32_t Abstr_Ec200sMqttCreat(int8_t ok, uint8_t retry)
{
    static uint8_t state = 0;
    static uint8_t openCnt = 0;
    int res = 0;
    
    switch (state)
    {
        case MQTT_STATE_OPEN:      //打开MQTT客户端
        {
            for (uint8_t i=0; i<5; i++) 
            {
                res = Abstr_Ec200sMqttOpen(MqttInfo_t.client_idx, MqttInfo_t.host_name, MqttInfo_t.port);
                if (res == OK)
                {
                    vTaskDelay(500);
                    break;
                }
            }
            
            if (res == OK)
            {
                state = MQTT_STATE_OPEN_CHK;
            }
            else
            {
                state = MQTT_STATE_FAIL;
            }   

            if (openCnt++ >= 6)
            {
                state = MQTT_STATE_FAIL;
                openCnt = 0;
            }
        }    
        break;
        
        case MQTT_STATE_OPEN_CHK:    //查询MQTT连接状态 
        {
            if (Abstr_Ec200sMqttOpenCheck() == OK)
            {
                state = MQTT_STATE_CONNECT;
                vTaskDelay(500);
            }
            else
            {
				if (openCnt++ >= 6)
				{
					state = MQTT_STATE_FAIL;
					openCnt = 0;
				}
                state = MQTT_STATE_OPEN;
            }
        
        }    
        break;

        case MQTT_STATE_CLOSE:
        {
            Abstr_Ec200sMqttClose(MqttInfo_t.client_idx);
            state = MQTT_STATE_OPEN;
        }
        break;
        
        case MQTT_STATE_CONNECT:   //连接MQTT服务器
        {
            res = Abstr_Ec200sMqttConnect(MqttInfo_t.client_idx, MqttInfo_t.client_id, \
                                            MqttInfo_t.username, MqttInfo_t.password);   \
            if (res == OK)
            {
				LOG("connect server success\r\n");
                state = MQTT_STATE_SUB;
                vTaskDelay(500);
            }
            else
            {
				LOG("connect server fail\r\n");
                state = MQTT_STATE_CLOSE;
            }
        }    
        break;
        
        case MQTT_STATE_SUB:    //订阅主题
        {
			res = Abstr_Ec200sMqttSub(MqttInfo_t.client_idx,MqttInfo_t.topic_sub_req);
            if (res == OK)
            {
                state = MQTT_STATE_READY;
                vTaskDelay(500);
            }
            else
            {
                Abstr_Ec200sMqttDisconnect(MqttInfo_t.client_idx);
                state = MQTT_STATE_CLOSE;
            }
        }    
        break;
        
        case MQTT_STATE_READY:     //MQTT网络环境准备就绪
        {
            state = MQTT_STATE_OPEN;
            AbstrEc200sGprsInitState++;
            openCnt = 0;
        }
        break;
        
        case MQTT_STATE_FAIL:     //MQTT连接失败
        {
            state = MQTT_STATE_OPEN;
            AbstrEc200sGprsInitState = EC200S_RESET;
            LOG("mqtt connect fail, reset 4G module.\n");
            vTaskDelay(500);
        }
        break;
    
        default:
        break;
    }
    
    return OK;
}


int32_t Abstr_Ec200sMqttRecv(uint8_t recvid)
{
    int8_t cmd_req[64] = {0};

    xSemaphoreTakeRecursive(AbstrEc200sMutex, portMAX_DELAY);

	sprintf((void *)cmd_req, "AT+QMTRECV=%d,%d\r", MqttInfo_t.client_idx, recvid);
    LOG("cmd_req=%s.\n", cmd_req);
	Abstr_Ec200sSendCmd(cmd_req, NULL, 0, 0);

	xSemaphoreGiveRecursive(AbstrEc200sMutex);

	return OK;
}


#ifdef EC200S_USE_TCP
static int32_t Abstr_Ec200sTcpSockeOpen(int32_t socket, int8_t *addr, int32_t port)
{
	int8_t cmd_req[64] = {0};
	int8_t cmd_ack[64] = {0};

	sprintf((void *)cmd_ack, "+QIOPEN: %d,0",socket);
	sprintf((void *)cmd_req, "AT+QIOPEN=1,%d,\"TCP\",\"%s\",%d,0,1\r", socket, addr, (int)port);
    EC200S_DEBUG("cmd_ack=%s, cmd_req=%s.\n", cmd_ack, cmd_req);
	if (!Abstr_Ec200sSendCmd(cmd_req, cmd_ack, 30000, 0))
	{
		EC200S_DEBUG("[INFO] link %d open success,addr=%s,port=%d.\r\n", socket, addr, port);
		return OK;
	}
	else
	{
		EC200S_DEBUG("[INFO] link %d open failure.\r\n", socket);
		return FAIL;
	}
}


//TCP连接过程
static int32_t Abstr_Ec200sTcpConnect(int8_t ok, uint8_t retry)
{
	static uint32_t state = 0;
	int32_t res = 0;
	uint8_t ready = 0;

	switch (state)
	{
        case NET_STATE_SOCKET0_OPEN:
        {
            ready = 0;
            for (uint8_t i = 0; i < 10; i++)
            {
                res = Abstr_Ec200sTcpSockeOpen(SOCKET_ID, (void*)NET_SERVER_IP, NET_SERVER_PORT);

                if (res == OK)
                {
                    ready = 1;
                    break;
                }
            }

            if (1 == ready)
            {
                state = NET_STATE_READY;
            }
            else
            {
                state = NET_STATE_FAILURE;
            }
        }
        break;

        case NET_STATE_READY:
        {
            state = NET_STATE_SOCKET0_OPEN;
            AbstrEc200sGprsInitState++;
        }
        break;

        case NET_STATE_FAILURE:
        {
            state = 0;
            AbstrEc200sGprsInitState = EC200S_RESET;
        }
        break;
	}
	return OK;
}


//在相应的建立的Socket上面发送GPRS数据
int32_t Abstr_Ec200sTcpSend(uint8_t *data, uint16_t len)
{
	int8_t cmd_req[32] = {0};
	int8_t cmd_ack[16] = {0};
	int32_t res = 0;
	uint8_t i = 0;

	AbstrEc200sInfo.ec200sState.Bits.ec200sSendDataState = 1;
	xSemaphoreTakeRecursive(AbstrEc200sMutex, portMAX_DELAY);
	for (i = 0; i < 5; i++) //每条数据重复发送三次
	{
		if (AbstrEc200sInfo.ec200sState.Bits.ec200sFtpState == 1) //正在进行FTP下载
		{
			EC200S_DEBUG("FTP Downloading, do not send data.\r\n");
			res = 0;
			break;
		}

		sprintf((void *)cmd_ack, "SEND OK");
		sprintf((void *)cmd_req, "AT+QISEND=0,%d\r", len);

		res = Abstr_Ec200sSendCmd((void *)cmd_req, (void *)">", 1000, 0); //发送需要发送数据指令
		if (OK != res)
		{
			EC200S_DEBUG("call GprsSendCmd=%d,error.\r\n", res);
			continue;
		}

		res = Abstr_Ec200sSendData((void *)data, len, (void *)cmd_ack, 10000);
		if (OK == res)
		{
			//PrintfData(data,len);
			break;
		}
		else
		{
			continue;
		}
	}
	xSemaphoreGiveRecursive(AbstrEc200sMutex);

	AbstrEc200sInfo.ec200sState.Bits.ec200sSendDataState = 0;
	if(res != OK)
	{
		EC200S_DEBUG("send data error,reconnect socket.\r\n");
		AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState = 0;
		Abstr_Ec200s_SendNotify();
	}
	return res;
}
#endif

//建立GPRS模块的相关联网功能，进行联网操作
int32_t Abstr_Ec200sReconnect(void)
{
	int8_t ok;
	uint8_t retry = 0;
	static uint8_t reconnectTimes = 0;

	EC200S_DEBUG("Enter Abstr_Ec200sReconnect\r\n");
	SystemInfo.connect_server_flag = FALSE;
	SystemInfo.onlineflag = FALSE;
    
#if defined(USE_TEST_ANTENNA)
	AbstrEc200sGprsInitState = EC200S_RESET;
#else
	if (AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState == 1)
	{
		AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState = 0;
//		if ((AbstrEc200sInfo.ec200sGprsInfo.gprsConnStatCallback != NULL) 
//                && (AbstrEc200sInfo.ec200sState.Bits.ec200sFtpState == 0))
//		{
//			AbstrEc200sInfo.ec200sGprsInfo.gprsConnStatCallback(AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState);
//		}
	}

//	if (AbstrEc200sInfo.ec200sState.Bits.ec200sIsFirstPowerOnState == 0)
//	{
//		AbstrEc200sGprsInitState = EC200S_RESET;
//	}
//	else
//	{
//		AbstrEc200sGprsInitState = EC200S_RESET;
//	}
    AbstrEc200sGprsInitState = EC200S_RESET;
#endif

	while (1)
	{
		vTaskDelay(AbstrEc200sGprsInitTable[AbstrEc200sGprsInitState].nwait);
		if (AbstrEc200sGprsInitTable[AbstrEc200sGprsInitState].cmd)
		{																																																		  //进行对GPRS模块的相关配置
			ok = Abstr_Ec200sSendCmd(AbstrEc200sGprsInitTable[AbstrEc200sGprsInitState].cmd, AbstrEc200sGprsInitTable[AbstrEc200sGprsInitState].res, AbstrEc200sGprsInitTable[AbstrEc200sGprsInitState].wait, 0); //发送相应的指令
		}

		if (AbstrEc200sGprsInitTable[AbstrEc200sGprsInitState].process)
		{
			if (OK == AbstrEc200sGprsInitTable[AbstrEc200sGprsInitState].process(ok, retry))
			{ //判断指令是否成功
				retry = 0;
			}
			else
			{
				retry++;
			}
		}

		if (AbstrEc200sGprsInitState == EC200S_STATE_NUM)
		{
			AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState = 1;
			AbstrEc200sInfo.ec200sState.Bits.ec200sIsFirstPowerOnState = 1;
            EC200S_DEBUG("Ec200s connect success\r\n");
			SystemInfo.connect_server_flag = TRUE;
			return OK;
		}
		else if (EC200S_RESET == AbstrEc200sGprsInitState)
		{
			EC200S_DEBUG("Gprs connect fail\r\n");
#if defined(USE_TEST_ANTENNA)
#else
			if (AbstrEc200sInfo.ec200sState.Bits.ec200sIsFirstPowerOnState == 1) //设备如果第一次进行TCP连接，则不用复位模块的电源
			{
				reconnectTimes++;
				if (reconnectTimes <= 4) //重连次数没有超过，则不复位电源
					AbstrEc200sGprsInitState = EC200S_RESET;
				else
				{
					AbstrEc200sGprsInitState = EC200S_RESET;
					AbstrEc200sInfo.ec200sState.Bits.ec200sIsFirstPowerOnState = 0; //标识模块需要复位电源
					reconnectTimes = 0;
				}
			}
#endif
		}
	}
}


/*
  Diffrent with strstr() ,will not stop when meet '\0'
*/
static void* mystrstr(char* fatherbuf, int fa_len, char* subbuf, int sub_len)
{
    int i = 0;
    for(i = 0;i < fa_len;i++)
    {
        if(memcmp((const void*)(fatherbuf + i), (const void*)subbuf, sub_len) == 0)
        {
            return (void*)(fatherbuf + i);
        }
    }
    return NULL;
}

#ifdef USE_FTP_DOWNLOAD
static uint32_t Abstr_Ec200sGetFileSize(char *file)
{
	uint16_t i= 0;
	uint8_t c;
	char *p = NULL;
	uint32_t fileSize = 0;
	uint16_t tmo = 10000;
	char cmd[64] = {0};			//指令进行的缓存

	xSemaphoreTakeRecursive(AbstrEc200sMutex, portMAX_DELAY);
	sprintf((void *)cmd, "AT+QFTPSIZE=\"%s\"\r", file);
	BswDrv_Ec200s_FifoFlush();
	BswDrv_Ec200sUartSend((void*)cmd, strlen((void *)cmd));

	//等待数据
	while (tmo) {
		vTaskDelay(50);
		tmo -= 50;
		while (BswDrv_Ec200s_GetCharFromFifo(&c) == TRUE) {
			if (i < (sizeof(AbstrEc200sBuffer)-1)) {
				AbstrEc200sBuffer[i++] = c;
			}
		}
		if((p = mystrstr((char*)AbstrEc200sBuffer,i,"+QFTPSIZE:",strlen("+QFTPSIZE:"))) != NULL) 
        {
			EC200S_DEBUG("p=%s\r\n",p);
			sscanf(p,"+QFTPSIZE: 0,%d\r\n",&fileSize);
            break;
		}
	}
	xSemaphoreGiveRecursive(AbstrEc200sMutex);
    SYSTEM_DEBUG("CMD:%s RETURN:%s \r\n",cmd,AbstrEc200sBuffer);
	return fileSize;
}


static int Abstr_Ec200sGetFtpData(char *file,uint32_t fileOffset,uint8_t *pBuff,uint16_t getdataLen)
{
	uint16_t i= 0;
	uint8_t c;
	uint8_t *p = NULL;
	uint8_t *p_connect = NULL;
	uint8_t *p_ftpget = NULL;
	uint16_t getSize = 0;			//FTP获取到的实际数据长度
	uint16_t tmo = 30000;
	uint8_t flag = 0;				//标识获取实际OK
	uint16_t searchGetSize= 0;
	char cmd[64] = {0};			//指令进行的缓存

	xSemaphoreTakeRecursive(AbstrEc200sMutex, portMAX_DELAY);
	sprintf((void *)cmd, "AT+QFTPGET=\"%s\",\"COM:\",%d,%d\r", file,fileOffset,getdataLen);
	BswDrv_Ec200s_FifoFlush();
	BswDrv_Ec200sUartSend((void*)cmd, strlen((void *)cmd));
	EC200S_DEBUG("cmd = %s\r\n",cmd);
	//等待数据

	searchGetSize = 0;
	while (tmo) 
	{
		vTaskDelay(50);
		tmo -= 50;
		while (BswDrv_Ec200s_GetCharFromFifo(&c) == TRUE) 
		{
			if (i < (sizeof(AbstrEc200sBuffer)-1)) 
			{
				AbstrEc200sBuffer[i++] = c;
			}
		}

		while(1)
		{
			//查找最后一个 "+QFTPGET: 0," 标识，防止固件中存在此字符串造成问题
			if((p=mystrstr((char*)AbstrEc200sBuffer + searchGetSize,i - searchGetSize,"+QFTPGET: 0,",strlen("+QFTPGET: 0,"))) != NULL) //获取到数据
			{
				searchGetSize += p - (AbstrEc200sBuffer + searchGetSize) + strlen("+QFTPGET: 0,");
				EC200S_DEBUG("searchGetSizeStr=%s,searchGetSize=%d\r\n",p,searchGetSize);
				continue;		
			}
			break;
		}
		if(searchGetSize > 0)		//找到了获取长度的标志
		{
			p_ftpget = AbstrEc200sBuffer + searchGetSize - strlen("+QFTPGET: 0,");
			if(memcmp(p_ftpget,"+QFTPGET: 0,%d\r\n",strlen("+QFTPGET: 0,%d\r\n")) != 0)//加安全判断，防止找到的最后的内容是此内容造成异常
			{
				sscanf((void*)p_ftpget, "+QFTPGET: 0,%d\r\n", (int *)&getSize);
				EC200S_DEBUG("p_ftpget=%s,getSize=%d\r\n",p_ftpget,getSize);
				if(getSize > 0)		//如果没有获取到数据的长度内容则一直等待超时
				{
					flag = 1; 
					break;
				}
			}
		}
	}
	
	xSemaphoreGiveRecursive(AbstrEc200sMutex);
	EC200S_DEBUG("i = %d,getData = %s\r\n",i,AbstrEc200sBuffer);
	//\r\nCONNECT\r\n<Output file data>\r\nOK\r\n+QFTPGET: 0,500
	if(flag)
	{
		p_connect = mystrstr((void *)AbstrEc200sBuffer,i,"CONNECT\r\n",strlen("CONNECT\r\n"));
		if(p_connect != NULL)
		{
			p_connect += strlen("CONNECT\r\n");

			p = p_connect + getSize;
			EC200S_DEBUG("p=%s\r\n",p);
			if(strncmp((void*)p, "\r\nOK\r\n", strlen("\r\nOK\r\n")) == 0)
			{
				memmove(pBuff,p_connect,getSize);
				#if 0
				EC200S_DEBUG("Abstr_Ec200sGetFtpData : ");
				for(i = 0; i < getSize;i++)
				{
					printf("%02x ",pBuff[i]);
					if(((i+1)%17) == 0)
					{
						printf("\r\n");
					}
					
				}
				printf("\r\n");
				#endif
				return getSize;
			}
			else
			{
				SYSTEM_DEBUG("FTP get dataLen error,getLen=%d,not get end ok flag\r\n",getSize);
			}
		}
		return FAIL;	
	}
	else
	{
		return FAIL;
	}

}

/*
 * @Functions:   int8_t Abstr_Ec200sFtpGetFrimware(int8_t* server, int8_t* username, int8_t* passward, int8_t* file)
 * @Description: EC200S模块FTP下载固件
 * @Parameters:  int8_t* server   FTP服务器IP或域名
 * 				 int8_t* username FTP用户名
 * 				 int8_t* passward FTP密码
 * 				 int8_t* file     FTP下载文件名
 * @Return:     0:FTP下载成功
				1:FTP下载没有文件
				2:FTP下载总包头解析错误
				3:FTP下载，擦除相应flash扇区出错
				4:FTP下载，固件包头错误
				5:FTP下载，写入固件到flash错误
				6:FTP下载，从EC200S获取固件CRC错误
			    7:FTP下载，从flash读出计算的CRC和时间CRC不匹配
				8:FTP下载，从EC200S中获取的固件长度错误
				9:FTP Get数据超时
 */
static int32_t Abstr_Ec200sSendRepeatCmd(int8_t *cmd, int8_t *ack, uint16_t delay)
{
	uint8_t i = 0;
	uint8_t sendCmdFlag = 0;

	for (i = 0; i < 10; i++)
	{
		if (Abstr_Ec200sSendCmd(cmd, ack, delay, 0) == OK)
		{
			sendCmdFlag = 1;
			break;
		}
		vTaskDelay(400);
	}
	if (sendCmdFlag == 0)
	{
		return FAIL;
	}
	return OK;
}


int8_t Abstr_Ec200sFtpGetFrimware(int8_t* server, int8_t* username, int8_t* passward, int8_t* file)
{
	int8_t 	ipconfig[64] = {0};			//指令进行的缓存
	int8_t 	*sp1 = NULL;				//解析文件路径
	int8_t 	 temp[64] = {0};
	uint8_t  retryTimes = 0;			//进行FTP获取文件数据次数
	uint32_t fileSize = 0;				//FTP文件的大小
	uint16_t ftpGetLen = 0;				//设置FTP GET数据长度
	int32_t  getDataLen = 0;			//从FTP获取到的数据长度
	uint32_t ftpFileOffsetSize = 0;		//获取固件的偏移大小
	uint32_t totalFwSize = 0;			//固件总的大小
	uint8_t  ftpGetState = EC200S_FTP_GET_HEAD_STEP;
	uint16_t calculateSum = 0;
	uint32_t i = 0;	
	int ret = EC200S_FTP_NONE_FILE;

	ABSTR_PACK_HEADER packhead = {0};
	FW_HEAD_INFO_STR fwHeadInfo = {0};
    
	Abstr_Ec200sSetFtpDownStatus(1);
	AbstrEc200sInfo.ec200sState.Bits.ec200sFtpDownLoadState = 1;	//FTP处于升级过程中

	if(BswSrv_Flash_ReadUpgrade(&UgradeHeadInfo,sizeof(UgradeHeadInfo)) != BSW_STATUS_SUCCESS)
	{
		EC200S_DEBUG("read ugradeInfo error\r\n");
	}

	if((server == NULL) || (username == NULL) || (passward == NULL) || file == NULL)
		return -1;

	memset(ipconfig, 0, sizeof(ipconfig));
	sprintf((void *)ipconfig, "AT+QICLOSE=%d,3000\r", SOCKET_ID);

	Abstr_Ec200sSendRepeatCmd((void *)ipconfig,(void *)"OK", 5000);		//关闭socket连接
	Abstr_Ec200sSendRepeatCmd((void *)"AT+QICSGP=1,1,\"CMNET\",\"\",\"\",1\r",(void *)"OK", 5000);	//Configure PDP context 1. APN is “UNINET” 
	Abstr_Ec200sSendRepeatCmd((void *)"AT+QIDEACT=1\r",(void *)"OK", 5000);	//Disable PDP context 1.
	Abstr_Ec200sSendRepeatCmd((void *)"AT+QIACT=1\r",(void *)"OK", 5000);	//Activate PDP context 1.
	Abstr_Ec200sSendRepeatCmd((void *)"AT+QIACT?\r",(void *)"OK", 5000);
	Abstr_Ec200sSendRepeatCmd((void *)"AT+QFTPCFG=\"contextid\",1\r",(void *)"OK", 5000);	//Configure the PDP context ID as 1. The PDP context

	memset(ipconfig, 0, sizeof(ipconfig));
	sprintf((void *)ipconfig, "AT+QFTPCFG=\"account\",\"%s\",\"%s\"\r", username,passward);
	Abstr_Ec200sSendRepeatCmd((void *)ipconfig,(void *)"OK", 5000);						//Set user name and password.
	Abstr_Ec200sSendRepeatCmd((void *)"AT+QFTPCFG=\"filetype\",0\r",(void *)"OK", 10000);	//Set file type as binary.
	Abstr_Ec200sSendRepeatCmd((void *)"AT+QFTPCFG=\"transmode\",1\r",(void *)"OK", 10000);	//Set transfer mode as passive mode.
	Abstr_Ec200sSendRepeatCmd((void *)"AT+QFTPCFG=\"rsptimeout\",120\r",(void *)"OK", 10000);

	//设置服务器地址
	sp1 = (int8_t *)strchr((void *)server, '/');
	memset(temp, 0, sizeof(temp));
	memset(ipconfig, 0, sizeof(ipconfig));
	strncpy((void *)temp,(void *)server,sp1-server);
	sprintf((void *)ipconfig, "AT+QFTPOPEN=\"%s\",21\r", temp);
	EC200S_DEBUG("serv=%s.\r\n",ipconfig);
	Abstr_Ec200sSendRepeatCmd((void *)ipconfig,(void *)"+QFTPOPEN: 0,0",25000);

	//设置下载路径
	memset(temp, 0, sizeof(temp));
	strncpy((void *)temp,(void *)(sp1+1),strlen((void *)(sp1+1)));
	temp[strlen((void *)temp)] = '/';
	memset(ipconfig, 0, sizeof(ipconfig));
	sprintf((void *)ipconfig, "AT+QFTPCWD=\"/%s\"\r", temp);
	EC200S_DEBUG("ftp get path=%s.\r\n",ipconfig);
	Abstr_Ec200sSendRepeatCmd((void *)ipconfig,(void *)"+QFTPCWD: 0,0",15000);

	retryTimes = 0;
START:
	{
		if(AbstrEc200sInfo.ec200sState.Bits.ec200sStopFtpState == 1)		//收到FTP关闭指令
		{
			EC200S_DEBUG("====receve stop FTP cmd.\r\n");
			AbstrEc200sInfo.ec200sState.Bits.ec200sStopFtpState = 0;
			ret = EC200S_FTP_STOP;
			goto EXIT1;
		}

		if (retryTimes++ >= 5)				//获取次数超过5次，结束FTP下载
		{
			EC200S_DEBUG("upgrade fail,exit.\r\n");
			ret = EC200S_FTP_NONE_FILE;
			goto EXIT1;
		}
		//获取文件长度
		fileSize = Abstr_Ec200sGetFileSize((char*)file);
		EC200S_DEBUG("file size=%d \r\n",fileSize);
		if(fileSize <= 0)
		{
			EC200S_DEBUG("+++++fileSize error.\r\n");
			goto START;
		}

		EC200S_DEBUG("start ftp retryTimes = %d \r\n", retryTimes);

		ftpGetLen = 16;			//标识从FTP Get数据长度
		totalFwSize = 0;
		ftpFileOffsetSize = 0;
		ftpGetState = EC200S_FTP_GET_HEAD_STEP;
		while (1)
		{
			vTaskDelay(50);

			if(AbstrEc200sInfo.ec200sState.Bits.ec200sStopFtpState == 1)		//收到FTP关闭指令
			{
				EC200S_DEBUG("receve stop FTP cmd.\r\n");
				AbstrEc200sInfo.ec200sState.Bits.ec200sStopFtpState = 0;
				ret = EC200S_FTP_STOP;
				goto EXIT1;
			}

			memset(AbstrEc200sBuffer, 0, sizeof(AbstrEc200sBuffer));
			BswDrv_Ec200s_FifoFlush();

			getDataLen = Abstr_Ec200sGetFtpData((char*)file, ftpFileOffsetSize, AbstrEc200sBuffer, ftpGetLen);
			if(getDataLen > 0)
			{
				ftpFileOffsetSize += getDataLen;

				switch(ftpGetState)
				{
					case EC200S_FTP_GET_HEAD_STEP:		//FTP下载获取头部信息
					{
						memcpy((void*)&packhead,AbstrEc200sBuffer,sizeof(packhead));
                        if ((0xaa == packhead.aa) && (0x55 == packhead.five)) 
						{
                            SYSTEM_DEBUG("FTP get head ok,fwCnt=%d\r\n", packhead.fwCnt);
                            if (0 == packhead.fwCnt) 
							{
                                SYSTEM_DEBUG("fwCnt=%d,err\r\n",packhead.fwCnt);
                                //BswDrv_Sc8042bOptFailNotice(57);        //todo by liutao
                                ret = EC200S_FTP_GET_HEAD_INFO_STEP;
								goto EXIT1;
                            }
							ftpGetState = EC200S_FTP_GET_HEAD_INFO_STEP;
							ftpGetLen = 16;			//标识从FTP Get数据长度
                        }
						else
						{
                            SYSTEM_DEBUG("FTP get head =%02x,%02x,err\r\n",AbstrEc200sBuffer[0],AbstrEc200sBuffer[1]);
                            //BswDrv_Sc8042bOptFailNotice(58);              //todo by liutao
                            ret = EC200S_FTP_PACK_HEAD_ERROR;
							goto EXIT1;
                        }
						break;
					}
					case EC200S_FTP_GET_HEAD_INFO_STEP:	//FTP下载获取固件信息
					{
						memcpy((void*)&fwHeadInfo,AbstrEc200sBuffer,sizeof(fwHeadInfo));

                        if(memcmp("LC01", fwHeadInfo.name, strlen("LC01")) == 0)
						{
                            if ((SystemParam.systemCurrentFwInfo.size == fwHeadInfo.size) && 
								(SystemParam.systemCurrentFwInfo.checkSum == fwHeadInfo.checkSum) 
								&& (SystemParam.systemCurrentFwInfo.ver == packhead.fwVer1)) 
							{
                                SYSTEM_DEBUG("fw same,no need upgrade\r\n");
                                //BswDrv_Sc8042bOptFailNotice(804);    //todo by liutao
                                ret = EC200S_FTP_SUCCESS;
								goto EXIT1;
                            }
                            ftpGetState = EC200S_FTP_GET_FIRMWARE;
							ftpGetLen = EC200S_FTP_LEN;
							totalFwSize = 0;
							calculateSum = 0;
                            SYSTEM_DEBUG("fw name ok,totalFwSize=%d,checkSum=%d,fwVer=%d.\n",
											fwHeadInfo.size,fwHeadInfo.checkSum,packhead.fwVer1);
                            BswSrv_Flash_EraseFirmware();
                        }
						else
						{
                            SYSTEM_DEBUG("fw name=%s,err\r\n",fwHeadInfo.name);
                            //BswDrv_Sc8042bOptFailNotice(59);            //todo by liutao
                            ret = EC200S_FTP_PACK_NAME_ERROR;
							goto EXIT1;
                        }

						break;
					}
					case EC200S_FTP_GET_FIRMWARE:		//FTP下载获取固件
					{
						for (i = 0; i < getDataLen; i++)
						{
							calculateSum += AbstrEc200sBuffer[i];
						}
						
						ret = BswSrv_Flash_WriteFirmware(AbstrEc200sBuffer, getDataLen, totalFwSize);
						if(ret == BSW_STATUS_ERROR)
						{
							SYSTEM_DEBUG("WriteFirmware error\r\n");
							ret = EC200S_FTP_FIRMWARE_WRITE_ERROR;
							goto EXIT1;
						}

						totalFwSize += getDataLen;
						if((fwHeadInfo.size - totalFwSize)/EC200S_FTP_LEN)
						{
							ftpGetLen = EC200S_FTP_LEN;
						}
						else
						{
							ftpGetLen = (fwHeadInfo.size - totalFwSize)%EC200S_FTP_LEN;
						}

						SYSTEM_DEBUG("FTP total=%d,getLen=%d---[%d%%].\r\n",fwHeadInfo.size,totalFwSize,totalFwSize*100/fwHeadInfo.size);


						if (totalFwSize == fwHeadInfo.size)
						{
							if (calculateSum == fwHeadInfo.checkSum) 
							{  //校验和正确，则进行对固件的相关信息进行存储，重启系统后再BootLoader中进行固件的拷贝
								SYSTEM_DEBUG("write file size %d, checksum 0x%4X,ftp get success finish! .\r\n", totalFwSize, calculateSum);

								uint16_t ck = 0;
								uint8_t rbyte = 0;
								for(i = 0; i < totalFwSize; i++)
								{
									FEED_WDG();
									BswSrv_Flash_ReadFirmware(&rbyte, 1, i);
									ck += rbyte;
								}
								if(ck != calculateSum)
								{
									SYSTEM_DEBUG("flash check error,readSum=0x%0x,fwSum=0x%0x\r\n",ck,calculateSum);
									ret = EC200S_FTP_FIRMWARE_CHECK_ERROR;
									goto EXIT1;
								}

								UgradeHeadInfo.checkSum = calculateSum;
								UgradeHeadInfo.fsize = totalFwSize;
								UgradeHeadInfo.updateFlag = 0xaa55;
								BswSrv_Flash_WriteUpgrade((void *)&UgradeHeadInfo,sizeof(UgradeHeadInfo));

								SystemParam.systemCurrentFwInfo.checkSum = calculateSum;
								SystemParam.systemCurrentFwInfo.size = totalFwSize;
								SystemParam.systemCurrentFwInfo.ver = packhead.fwVer1;
								App_SystemParamStorageAll();
								ret = EC200S_FTP_SUCCESS;

								//BswDrv_Sc8042bSpeech(VOIC_START_SUCCESS);  //todo by liutao
								vTaskDelay(1000);
							}
							else
							{
								SYSTEM_DEBUG("calculateSum=%d,actualChecksum=%d,error.\r\n",calculateSum,fwHeadInfo.checkSum);
								ret = EC200S_FTP_FIRMWARE_CHECK_ERROR;
							}
							goto EXIT1;
						}

						break;
					}
					default:break;
				}
			}
			else
			{
				EC200S_DEBUG("FTP get data error,retryTimes = %d\r\n",retryTimes);
				ret = EC200S_FTP_FIRMWARE_LEN_ERROR;
				goto EXIT1;
			}
			
		}
	}

EXIT1:

	if(ret == EC200S_FTP_NONE_FILE)
	{
		AbstrEc200sInfo.ec200sState.Bits.ec200sFtpDownLoadState = 3;
	}
	else if(ret == EC200S_FTP_STOP)
	{
		AbstrEc200sInfo.ec200sState.Bits.ec200sFtpDownLoadState = 4;
	}
	else if(ret == EC200S_FTP_SUCCESS)
	{
		AbstrEc200sInfo.ec200sState.Bits.ec200sFtpDownLoadState = 2;
	}
	else
	{
		AbstrEc200sInfo.ec200sState.Bits.ec200sFtpDownLoadState = 0;
	}

	
	Abstr_Ec200sSendCmd((void *)"AT+QFTPCLOSE\r", (void *)"OK", 3000, 0);
	Abstr_Ec200sSendCmd((void *)"AT+QIDEACT=1\r", (void *)"OK", 3000, 0);
	Abstr_Ec200sSetFtpDownStatus(0);
	EC200S_DEBUG("ftp reslut=%d.\r\n",ret);
	return ret;
}

uint8_t Abstr_Ec200sStatus(void)
{
	if (AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState == 0)
		return EC200S_TCPCONNECING;

	if ((AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState == 1) &&
		(AbstrEc200sInfo.ec200sState.Bits.ec200sFtpState == 0))
	{
		return EC200S_TCPCONNECTED;
	}

	if (AbstrEc200sInfo.ec200sState.Bits.ec200sFtpState == 1)
		return EC200S_FTPDOWN;

	return EC200S_TCPCONNECING;
}

void Abstr_Ec200sSetFtpDownStatus(uint8_t status)
{
	if (status)
	{
		AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState = 1; //标识4G联网
		AbstrEc200sInfo.ec200sState.Bits.ec200sFtpState = 1;	 //标识处于FTP下载
		SystemInfo.onlineflag = FALSE;								//清除TCP联网标识
	}
	else
	{
		AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState = 0; //标识4G断网
		AbstrEc200sInfo.ec200sState.Bits.ec200sFtpState = 0;
	}
}

void Abstr_Ec200sSetFtpStop(void)
{
	printf("Abstr_Ec200sSetFtpStop\r\n");
	AbstrEc200sInfo.ec200sState.Bits.ec200sStopFtpState = 1;
}

#endif  //USE_FTP_DOWNLOAD

static int8_t Abstr_Ec200sSocketStatusCheck(void)
{
	if(AbstrEc200sInfo.ec200sState.Bits.ec200sSendDataState == 1)
	{
		EC200S_DEBUG("Ec200s is send data.\r\n");
		return OK;
	}

	Abstr_Ec200sSendCmd((void *)"AT+CSQ\r", (void *)"OK", 5000, 0);
//	vTaskDelay(100);
//	if(Abstr_Ec200sSendCmd((void *)"AT+QISTATE?\r", (void *)"OK", 5000, 0) != OK)
//	{
//		AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState = 0;
//		AbstrEc200sInfo.ec200sRssi = 0;
//		SystemInfo.csq = 0;
//		EC200S_DEBUG("Gprs offline.\r\n");
//		return FAIL;
//	}
	return OK;
}

void Abstr_Ec200s_SendNotify(void)
{
	extern TaskHandle_t Ec200sAbstrTask_Handle;
	if (Ec200sAbstrTask_Handle != NULL)
	{
		xTaskNotifyGive(Ec200sAbstrTask_Handle);
	}
}

void Abstr_Ec200s_SendNotifyFromIsr(void)
{
	extern TaskHandle_t Ec200sAbstrTask_Handle;
	BaseType_t pxHigherPriorityTaskWoken;
	if (Ec200sAbstrTask_Handle != NULL)
	{
		vTaskNotifyGiveFromISR(Ec200sAbstrTask_Handle, &pxHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
	}
}

uint8_t Abstr_Ec200s_GetState(uint8_t status)
{
	uint8_t result = 0;

	if (AbstrEc200sInfo.ec200sState.Bits.ec200sPowerState == 1)
	{
		result |= EC200S_STATE_IS_POWERON;
	}

	if (AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState == 1)
	{
		result |= EC200S_STATE_IS_ONLINE;
	}

	if (AbstrEc200sInfo.ec200sState.Bits.ec200sFtpState == 1)
	{
		result |= EC200S_STATE_IS_FTP_DOWNLOAD;
	}
    
    if (AbstrEc200sInfo.ec200sState.Bits.ec200sHttpState == 1)
	{
		result |= EC200S_STATE_IS_HTTP_DOWNLOAD;
	}
    
	return (result & status);
}


void Abstr_Ec200sSetHttpDownStatus(uint8_t status)
{
	if (status)
	{
		AbstrEc200sInfo.ec200sState.Bits.ec200sHttpState = 1;	 //标识处于FTP下载
	}
	else
	{
		AbstrEc200sInfo.ec200sState.Bits.ec200sHttpState = 0;
	}
}


static uint32_t Abstr_Ec200sGetHttpFileSize(char *file)
{
	uint16_t i= 0;
	uint8_t c;
	char *p = NULL;
	uint32_t fileSize = 0;
	uint16_t tmo = 10000;
	char cmd[64] = {0};			//指令进行的缓存

	xSemaphoreTakeRecursive(AbstrEc200sMutex, portMAX_DELAY);
	sprintf((void *)cmd, "AT+QHTTPGET=80\r");
	BswDrv_Ec200s_FifoFlush();
	BswDrv_Ec200sUartSend((void*)cmd, strlen((void *)cmd));

	//等待数据
	while (tmo) {
		vTaskDelay(50);
		tmo -= 50;
		while (BswDrv_Ec200s_GetCharFromFifo(&c) == TRUE) {
			if (i < (sizeof(AbstrEc200sBuffer)-1)) {
				AbstrEc200sBuffer[i++] = c;
			}
		}
		if((p = mystrstr((char*)AbstrEc200sBuffer,i,"+QHTTPGET: 0,200,",strlen("+QHTTPGET: 0,200,"))) != NULL) 
        {
			EC200S_DEBUG("p=%s\r\n",p);
			sscanf(p,"+QHTTPGET: 0,200,%d",&fileSize);
            break;
		}
	}
	xSemaphoreGiveRecursive(AbstrEc200sMutex);
    SYSTEM_DEBUG("CMD:%s RETURN:%s \r\n",cmd,AbstrEc200sBuffer);
	return fileSize;
}


static uint32_t Abstr_Ec200sGetHttpDataSize(uint32_t fileOffset, uint16_t getdataLen)
{
	uint16_t i= 0;
	uint8_t c;
	char *p = NULL;
	uint32_t fileSize = 0;
	uint16_t tmo = 10000;
	char cmd[64] = {0};			//指令进行的缓存

	xSemaphoreTakeRecursive(AbstrEc200sMutex, portMAX_DELAY);
	memset(cmd, 0, sizeof(cmd));
	sprintf((void *)cmd, "AT+QHTTPGETEX=60,%d,%d\r\n", fileOffset, getdataLen); 
	BswDrv_Ec200s_FifoFlush();
	BswDrv_Ec200sUartSend((void*)cmd, strlen((void *)cmd)); 
	  
	//等待数据
	while (tmo) {
		vTaskDelay(50);
		tmo -= 50;
		while (BswDrv_Ec200s_GetCharFromFifo(&c) == TRUE) {
			if (i < (sizeof(AbstrEc200sBuffer)-1)) {
				AbstrEc200sBuffer[i++] = c;
			}
		}
		if((p = mystrstr((char*)AbstrEc200sBuffer,i,"+QHTTPGET: 0,206,",strlen("+QHTTPGET: 0,206,"))) != NULL) 
        {
			EC200S_DEBUG("p=%s\r\n",p);
			sscanf(p,"+QHTTPGET: 0,206,%d",&fileSize);
            break;
		}
	}
	xSemaphoreGiveRecursive(AbstrEc200sMutex);
    SYSTEM_DEBUG("CMD:%s RETURN:%s \r\n",cmd,AbstrEc200sBuffer);
	return fileSize;
}


static int Abstr_Ec200sGetHttpData(char *file,uint32_t fileOffset,uint8_t *pBuff,uint16_t getdataLen)
{
	uint16_t i= 0;
	uint8_t c;
	uint8_t *p = NULL;
	uint8_t *p_connect = NULL; 
	uint32_t getSize = 0;			//FTP获取到的实际数据长度 
//	uint32_t fileSize = 0;				//HTTP文件的大小
	uint16_t tmo = 30000;
	uint8_t flag = 0;				//标识获取实际OK
//	uint16_t searchGetSize= 0;
	char cmd[64] = {0};			//指令进行的缓存
	
//	int32_t res = 1; 
	getSize = Abstr_Ec200sGetHttpDataSize(fileOffset,getdataLen);
	EC200S_DEBUG("fileOffset = %d, getdataLen = %d, getSize = %d\r\n", fileOffset, getdataLen, getSize);
	if(getSize == 0) return FAIL;	
	
	xSemaphoreTakeRecursive(AbstrEc200sMutex, portMAX_DELAY);
//	sprintf((void *)cmd, "AT+QHTTPGETEX=60,%d,%d\r", fileOffset,getdataLen);
	memset(cmd, 0, sizeof(cmd));
	sprintf((void *)cmd, "AT+QHTTPREAD=80\r");
	BswDrv_Ec200s_FifoFlush();
	BswDrv_Ec200sUartSend((void*)cmd, strlen((void *)cmd));
	EC200S_DEBUG("cmd = %s\r\n",cmd);
	//等待数据

//	searchGetSize = 0;
	while (tmo) 
	{
		FEED_WDG();
		vTaskDelay(50);
		tmo -= 50;
		while (BswDrv_Ec200s_GetCharFromFifo(&c) == TRUE) 
		{
			if (i < (sizeof(AbstrEc200sBuffer)-1)) 
			{
				AbstrEc200sBuffer[i++] = c;
			}
		}
		
        //如果没有获取到数据的长度内容则一直等待超时
		if(i >= (strlen("CONNECT\r\n")+getSize+strlen("OK\r\n")+strlen("+QHTTPREAD: 0\r\n")))		
		{
			flag = 1; 
			EC200S_DEBUG("i = %d,getSize = %d\r\n",i,getSize);
			break;
		}
	}
	
	xSemaphoreGiveRecursive(AbstrEc200sMutex);
	//EC200S_DEBUG("i = %d,tmo = %d,  getData = %s\r\n", i, tmo, AbstrEc200sBuffer);
	//\r\nCONNECT\r\n<Output file data>\r\nOK\r\n+QHTTPREAD: 0
	if(flag)
	{
		p_connect = mystrstr((void *)AbstrEc200sBuffer,i,"CONNECT\r\n",strlen("CONNECT\r\n"));
		if(p_connect != NULL)
		{
			p_connect += strlen("CONNECT\r\n");

			p = p_connect + getSize;
			EC200S_DEBUG("p=%s\r\n",p);
			if(strncmp((void*)p, "\r\nOK\r\n", strlen("\r\nOK\r\n")) == 0)
			{
				memmove(pBuff,p_connect,getSize);
				#if 0
				EC200S_DEBUG("Abstr_Ec200sGetHttpData : ");
				for(i = 0; i < getSize;i++)
				{
					printf("%02x ",pBuff[i]);
					if(((i+1)%17) == 0)
					{
						printf("\r\n");
					}
					
				}
				printf("\r\n");
				#endif
				return getSize;
			}
			else
			{
				SYSTEM_DEBUG("HTTP get dataLen error,getLen=%d,not get end ok flag\r\n",getSize);
			}
		}
		EC200S_DEBUG("p_connect == NULL,  return FAIL;\r\n");
		return FAIL;	
	}
	else
	{
		EC200S_DEBUG("flag=%d,  return FAIL;\r\n",flag);
		return FAIL;
	}
}
unsigned int crc = 0xffff;
unsigned int CRC_16(char *Pointer, int Number)
{
	unsigned char flag;
	unsigned char Counter;
	
	for(;Number>0;Number--)
	{
		crc = crc ^ (*Pointer);
		for(Counter = 0; Counter < 8; Counter++)
		{
			flag = crc & 0x01;
			crc = crc >> 1;
			if(flag)
			{
				crc = crc ^ 0xa001;
			}
		}
		Pointer ++;
	}
	return crc;
}

/*
 * @Functions:   int8_t Abstr_Ec200sHttpGetFrimware(int8_t* url_file)
 * @Description: EC200S模块HTTP下载固件
 * @Parameters:  int8_t* url_file   URL文件名
 * @Return:     0:HTTP下载成功
				1:HTTP下载没有文件
				2:HTTP下载总包头解析错误
				3:HTTP下载，擦除相应flash扇区出错
				4:HTTP下载，固件包头错误
				5:HTTP下载，写入固件到flash错误
				6:HTTP下载，从EC200S获取固件CRC错误
			    7:HTTP下载，从flash读出计算的CRC和时间CRC不匹配
				8:HTTP下载，从EC200S中获取的固件长度错误
				9:HTTP Get数据超时
 */
int8_t Abstr_Ec200sHttpGetFrimware(int8_t* url_file)
{
	int8_t 	ipconfig[64] = {0};			//指令进行的缓存  
	uint8_t  retryTimes = 0;			//进行HTTP获取文件数据次数
	uint32_t fileSize = 0;				//HTTP文件的大小
	uint16_t HttpGetLen = 0;			//设置HTTP GET数据长度
	int32_t  getDataLen = 0;			//从HTTP获取到的数据长度
	uint32_t HttpFileOffsetSize = 0;	//获取固件的偏移大小
	uint32_t totalFwSize = 0;			//固件总的大小
	uint8_t  HttpGetState = EC200S_HTTP_GET_HEAD_STEP;
	uint16_t calculateSum = 0;
	uint32_t i = 0;	
	uint32_t n = 0,j = 0;
	uint32_t getDataCnt = 0;
	int ret = EC200S_FTP_NONE_FILE;
	uint16_t url_file_len = strlen((const char*)url_file);
	uint8_t  cnt = 0;
	uint8_t  retrycnt = 0;			//进行HTTP小包尝试次数
	FW_HEAD_INFO fwHeadInfo = {0};
    uint16_t TempCrc = 0,TempCrc1 = 0;
	vTaskSuspend(Mcard_TaskHandler);
	Abstr_Ec200sSetHttpDownStatus(1);
	upgrade_flag = 1;
	AbstrEc200sInfo.ec200sState.Bits.ec200sHttpDownLoadState = 1;	//FTP处于升级过程中
	if(url_file == NULL)
		return -1;
	memset(ipconfig, 0, sizeof(ipconfig));

	sprintf((void *)ipconfig, "AT+QHTTPURL=%d,80\r", strlen((const char*)url_file)); 
	Abstr_Ec200sSendRepeatCmd((void *)ipconfig,(void *)"CONNECT", 5000);
	Abstr_Ec200sSendRepeatCmd(url_file,(void *)"OK", 5000);		  //打开HTTP服务器

	retryTimes = 0;
START:
	{
		if(AbstrEc200sInfo.ec200sState.Bits.ec200sStopHttpState == 1)		//收到HTTP关闭指令
		{
			EC200S_DEBUG("====receve stop HTTP cmd.\r\n");
			AbstrEc200sInfo.ec200sState.Bits.ec200sStopHttpState = 0;
			ret = EC200S_HTTP_STOP;
			goto EXIT1;
		}

		if (retryTimes++ >= 5)				//获取次数超过5次，结束HTTP下载
		{
			EC200S_DEBUG("upgrade fail,exit.\r\n");
			ret = EC200S_HTTP_NONE_FILE;
            //升级失败，获取文件大小失败
            //BswSrv_TtsPlay((void*)VOIC_UPGRADE_FAIL_GET_FILESIZE_ERR); 
			goto EXIT1;
		}
		
		//获取文件长度
		fileSize = Abstr_Ec200sGetHttpFileSize((char*)url_file);
		EC200S_DEBUG("file size=%d \r\n",fileSize);
		
		if(fileSize <= 0)
		{
			EC200S_DEBUG("+++++fileSize error.\r\n");
			goto EXIT1;
		}
		
		Abstr_Ec200sSendCmd((void *)"AT+QHTTPSTOP\r", (void *)"OK", 3000, 0); 
		EC200S_DEBUG("start http retryTimes = %d \r\n", retryTimes);

		HttpGetLen = 128;			//标识从HTTP Get固件头信息数据长度 
		totalFwSize = 0;
		HttpFileOffsetSize = 0;
		HttpGetState = EC200S_HTTP_GET_HEAD_STEP;   //获取固件头信息
		
		while (1)
		{
			vTaskDelay(20);
			FEED_WDG();
			if(AbstrEc200sInfo.ec200sState.Bits.ec200sStopHttpState == 1)		//收到HTTP关闭指令
			{
				EC200S_DEBUG("receve stop FTP cmd.\r\n");
				AbstrEc200sInfo.ec200sState.Bits.ec200sStopHttpState = 0;
				ret = EC200S_HTTP_STOP;
				goto EXIT1;
			}

			for(cnt=0; cnt < 10; cnt++)
			{
				FEED_WDG();
				vTaskDelay(50);
				memset(AbstrEc200sBuffer, 0, sizeof(AbstrEc200sBuffer));
				BswDrv_Ec200s_FifoFlush(); 
				getDataLen = Abstr_Ec200sGetHttpData((char*)url_file, HttpFileOffsetSize, AbstrEc200sBuffer, HttpGetLen);
				if(getDataLen != FAIL) break;
			}  
            
			if(getDataLen > 0)
			{
				switch(HttpGetState)
				{
					case EC200S_HTTP_GET_HEAD_STEP:		//HTTP下载获取头部信息
					{
						memcpy((void*)&fwHeadInfo, AbstrEc200sBuffer, sizeof(fwHeadInfo));
                        if ((0xaa != fwHeadInfo.aa) || (0x55 != fwHeadInfo.five)) 
						{ 
                            EC200S_DEBUG("HTTP get head =%02x,%02x,err\r\n", AbstrEc200sBuffer[0], AbstrEc200sBuffer[1]);
                            ret = EC200S_HTTP_PACK_HEAD_ERROR;
							goto EXIT1;
                        }  
						else 
						{
                            HttpGetState = EC200S_HTTP_GET_FIRMWARE;
							App_Ledturnoff();
							HttpGetLen = EC200S_HTTP_LEN;
							totalFwSize = 0;
							calculateSum = 0;
                            LOG("fw name ok, totalFwSize=%d, checkSum=%d.\n",
											fwHeadInfo.size, fwHeadInfo.checkSum);
                            App_Flash_Erase(APP_BACK_ADDR,APP_BACK_SIZE+4*1024);
                        } 
						HttpFileOffsetSize += getDataLen;

						break;
					}
					case EC200S_HTTP_GET_FIRMWARE:		//HTTP下载获取固件
					{
						TempCrc1 = CRC_16((char *)AbstrEc200sBuffer, (EC200S_HTTP_LEN));
						 
						 
						if (retrycnt++ >= 10)				//尝试次数超过10次，结束HTTP下载
						{
							EC200S_DEBUG("http upgrade fail,exit.\r\n");
							ret = EC200S_HTTP_NONE_FILE;
                            //升级失败，下载没有文件
                            //BswSrv_TtsPlay((void*)VOIC_UPGRADE_FAIL_NO_FILE);
							goto EXIT1;
						}
						
							retrycnt = 0;
							if ((totalFwSize + getDataLen) >= fwHeadInfo.size)
							{
								getDataCnt = fwHeadInfo.size - totalFwSize;	
							}else{
								getDataCnt = getDataLen;
							}
                            
							for(n = 0; n < 3; n++)
							{
								App_Write_Flash(AbstrEc200sBuffer,APP_BACK_ADDR+totalFwSize,getDataLen);
								App_Read_Flash(flash_data,APP_BACK_ADDR+totalFwSize,getDataLen);
								for ( j = 0; j < getDataLen; j++)
								{
									if(flash_data[j] != AbstrEc200sBuffer[j])
									{
										break;
									}
									else
									{
										TempCrc += flash_data[j];
									}
								}
								if(j == getDataLen)
								{
									break;
								}
								else
								{
									LOG("write flash fail %d\r\n",n);
								}
								
							}
							totalFwSize += getDataCnt;  
							LOG("HTTP total=%d,getDataCnt=%d,getLen=%d---[%d%%].\r\n",fwHeadInfo.size,getDataCnt,totalFwSize,totalFwSize*100/fwHeadInfo.size); 
							if (totalFwSize == fwHeadInfo.size) 
							{
                                //校验和正确，则进行对固件的相关信息进行存储，重启系统后再BootLoader中进行固件的拷贝
								if (TempCrc == fwHeadInfo.checkSum) 
								{  
									LOG("write file size %d, checksum 0x%4X, http get success finish! .\r\n", TempCrc, fwHeadInfo.checkSum);

									uint16_t ck = 0;
									uint8_t rbyte = 0;
                                    
									// for(i = 0; i < totalFwSize; i++) 
									// {
									// 	FEED_WDG();
									// 	App_Read_Flash(&rbyte,APP_BACK_ADDR+i,1);
									// 	ck += rbyte;
									// }
                                    
									// if(ck != TempCrc)
									// {
									// 	LOG("flash check error,readSum=0x%0x,fwSum=0x%0x\r\n", ck, calculateSum);
									// 	ret = EC200S_HTTP_FIRMWARE_CHECK_ERROR;
									// 	App_Flash_Erase(SYS_UPGRADE_ADDR,SYS_UPGRADE_SIZE);
                                    //     //升级失败，校验失败
                                    //     //BswSrv_TtsPlay((void*)VOIC_UPGRADE_FAIL_SUM_ERROR);
									// 	goto EXIT1;
									// }
									// else
									{
										upgrade_info.upgrade_flag = 0xa5;
										upgrade_info.size = totalFwSize;
										upgrade_info.crc = TempCrc;
										App_Write_Flash((void *)&upgrade_info, SYS_UPGRADE_ADDR,sizeof(Upgrade_Info));
										ret = EC200S_HTTP_SUCCESS; 
									}
								}
								else
								{
									LOG("calculateSum=%d,%d,actualChecksum=%d,error.\r\n",TempCrc,TempCrc1,fwHeadInfo.checkSum);
									ret = EC200S_HTTP_FIRMWARE_CHECK_ERROR;
                                    //升级失败，校验失败
                                    //BswSrv_TtsPlay((void*)VOIC_UPGRADE_FAIL_SUM_ERROR); 
								}
								goto EXIT1;
							}
							HttpFileOffsetSize += getDataLen;


						break;
					}
					default:break;
				}
			}
			else
			{
				EC200S_DEBUG("HTTP get data error,retryTimes = %d,  getDataLen = %d\r\n",retryTimes,getDataLen);
				ret = EC200S_HTTP_FIRMWARE_LEN_ERROR; 
                //升级失败，固件长度错误
                //BswSrv_TtsPlay((void*)VOIC_UPGRADE_FAIL_LEN_ERROR); 
				goto EXIT1;
			}
		}
	}

EXIT1:
	vTaskResume(Mcard_TaskHandler);
    vTaskDelay(3000);
	if(UgradeHeadInfo.SuccessNum == 0xFFFF) UgradeHeadInfo.SuccessNum=0;
	if(UgradeHeadInfo.FailNum == 0xFFFF)    UgradeHeadInfo.FailNum=0;
	if(ret == EC200S_HTTP_STOP)
	{
		AbstrEc200sInfo.ec200sState.Bits.ec200sHttpDownLoadState = 4;
		UgradeHeadInfo.FailNum++;
	} 
	else if(ret == EC200S_HTTP_NONE_FILE)
	{
		AbstrEc200sInfo.ec200sState.Bits.ec200sHttpDownLoadState = 3;
		UgradeHeadInfo.FailNum++;
	}
	else if(ret == EC200S_HTTP_SUCCESS)
	{
		AbstrEc200sInfo.ec200sState.Bits.ec200sHttpDownLoadState = 2;
		UgradeHeadInfo.SuccessNum++;
		//BswSrv_TtsPlayHttpFrimwareSuccess(UgradeHeadInfo.SuccessNum);
		//vTaskDelay(3000);
	}
	else
	{
		AbstrEc200sInfo.ec200sState.Bits.ec200sHttpDownLoadState = 0;
		UgradeHeadInfo.FailNum++;
	}
     
	Abstr_Ec200sSendCmd((void *)"AT+QHTTPSTOP\r", (void *)"OK", 3000, 0); 
	Abstr_Ec200sSetHttpDownStatus(0);
	upgrade_flag = 0;
	vTaskDelay(1000);
	LOG("http reslut=%d.\r\n",ret);
	NVIC_SystemReset();
	
	return ret;
}


/* HTTP下载升级任务 */
void App_UpgradeTask(void *pvParameters)
{
    int ret = EC200S_FTP_SUCCESS;
    ret = Abstr_Ec200sHttpGetFrimware(pvParameters);
	if(ret == EC200S_FTP_SUCCESS)
	{
		App_UpgradeAckHandler(0);
	}
	else
	{
		App_UpgradeAckHandler(1);
	}

	vTaskDelete(NULL);
}

void Abstr_Ec200sTask(void)
{
    AbstrEc200sMutex = xSemaphoreCreateRecursiveMutex();

    //Abstr_Ec200sMqttInit();
	while (1)
	{
		Abstr_Ec200sReconnect();
		
		while (1)
		{
			ulTaskNotifyTake(pdTRUE, 10000);
#if defined(USE_TEST_ANTENNA)
#else
			if(Abstr_Ec200s_GetState(EC200S_STATE_IS_ONLINE))
			{
				if(Abstr_Ec200sSocketStatusCheck() != OK)
				{
					break;
				}
				
//				Abstr_Ec200sQcell();
			}
            else
            {
                EC200S_DEBUG("Gprs reconnect.\r\n");
                //掉线二维码背光灯闪烁
				break;
            }

#endif
		}
	}
}








