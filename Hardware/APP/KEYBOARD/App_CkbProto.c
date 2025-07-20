#include "App_CkbProto.h"
#include "system.h"
#include "usart.h"
#include "fifo.h"
#include "rtc.h"
#include "gpio.h"
// #include "BswDrv_Gun.h"
#include "mcard.h"
#include "start.h"
#include "App_CkbProto.h"
// #include "BswDrv_Voice.h"
// #include "App_ProductTestProto.h"
bool led_keyboard = 0;
uint8_t CkbBuff[256];
uint8_t CkbSn = 0;
static uint32_t lastRecvCkbMsgTime = 0;

void App_CkbConnectStatuProc(void)
{
	
    uint32_t currentTime = App_GetRtcCount(); 
    if (currentTime - lastRecvCkbMsgTime >= 180 && currentTime - lastRecvCkbMsgTime <= 0xffff)
    {
		LOG("long time no recv keyBoard msg, reset.\r\n");
        lastRecvCkbMsgTime =  currentTime;
		DIS_OFF;
		Delay_ms(1000);
		DIS_ON;
    }
}


static int App_CkbProtoGetPktSum(uint8_t *pData, uint16_t len)
{
    int i;
    uint8_t  sum = 0;

    for (i=0; i<len; i++) {
        sum += pData[i];
    }
    return sum;
}

static int App_CkbProtoSendPkt(uint8_t sn, uint8_t cmd, CKB_STR *pPkt, uint16_t len)
{
    uint8_t *pbuff = (void*)pPkt;

    pPkt->head.ff = 0xFF;
    pPkt->head.bb = 0xBB;
    pPkt->head.len = len + 4;
    pPkt->head.ver = 0x01;
    pPkt->head.sn = sn;
    pPkt->head.cmd = cmd;
    pbuff[sizeof(CKB_HEAD_STR) + len] = App_CkbProtoGetPktSum((void*)&pPkt->head.ver, len+3);
    
	Bsp_UartSendData(DIS_UART, (void*)pPkt, sizeof(CKB_HEAD_STR)+len+1);
	return OK;
}

//灯显示控制
void App_CkbProtoLedCtrol(uint8_t ledIndex, uint8_t lightOnFlag)
{
    CKB_STR *pPkt = (CKB_STR *)CkbBuff;    
    CKB_LED_CTRL *pLedCtrl = (CKB_LED_CTRL*)pPkt->data;
    memset(CkbBuff, 0, sizeof(CkbBuff));
    pLedCtrl->key = ledIndex;
    pLedCtrl->lightOnFlag = lightOnFlag;
	LOG("led[%d] = %d.\n",ledIndex,lightOnFlag);

    App_CkbProtoSendPkt(CkbSn++, CKB_CMD_LED_CTRL, pPkt, sizeof(CKB_LED_CTRL));
}

//心跳回复
void App_CkbProtoHeartBeatAck(void)
{
    CKB_STR *pPkt = (CKB_STR *)CkbBuff; 
    CKB_HEARTBEAT_REPORT_ACK *pHeartBeatAck = (CKB_HEARTBEAT_REPORT_ACK*)pPkt->data;

    for (uint8_t i = 0; i < PORTMAX; i++)
	{
		pHeartBeatAck->gunStatu[i] = Port_charge_status[i];
	}

    App_CkbProtoSendPkt(CkbSn++, CKB_CMD_HEARTBEAT, pPkt, sizeof(CKB_HEARTBEAT_REPORT_ACK));
}


/*=================================消息处理===========================================*/

//按键板心跳处理
static void App_CkbProtoHandle_HeartBeat(uint8_t *data, uint16_t len)
{
    CKB_HEARTBEAT_REPORT *pHeartBeat = (CKB_HEARTBEAT_REPORT *)data;
	lastRecvCkbMsgTime = App_GetRtcCount();
    LOG("ckb protoVer = %s, ckbFault=0x%2X.\r\n", pHeartBeat->protoVer, pHeartBeat->faultCode);
    memcpy((void*)&SystemInfo.ckbVer, pHeartBeat->protoVer, sizeof(SystemInfo.ckbVer));
    App_CkbProtoHeartBeatAck();
    
}
void App_CkbProtoCardCtrol(uint8_t cardflag)
{
	CKB_STR *pPkt = (CKB_STR *)CkbBuff;    
    CKB_CARD_CTRL *pCardCtrl = (CKB_CARD_CTRL*)pPkt->data;

    memset(CkbBuff, 0, sizeof(CkbBuff));
	pCardCtrl->cardfalg = cardflag;
	pCardCtrl->led_index = 0;
//	LOG("led[%d] = %d.\n",ledIndex,lightOnFlag);

    App_CkbProtoSendPkt(CkbSn++, CKB_CMD_CARD_CTRL, pPkt, sizeof(CKB_CARD_CTRL));
}

//按键板按下处理
static void App_CkbProtoHandle_CardProc(uint8_t *data, uint16_t len)
{
	uint8_t strTmp[256] = {0};
	SystemPortInfo_t *pGunInfo = NULL;
    CKB_CARD_CTRL *pCardctrl = (CKB_CARD_CTRL *)data;
	if((gChgInfo.McardCheckFlag == MCARD_REQ_ACK)&&(pCardctrl->led_index != 0))
	{
		pGunInfo = &SystemOrderInfo[pCardctrl->led_index-1];
		if(pGunInfo->gunState == PORT_IDLE)
		{
			App_CkbProtoCardCtrol(0);
			gChgInfo.current_usr_gun_id = pCardctrl->led_index;
			gChgInfo.McardCheckFlag  = MCARD_SENDING;
			// sprintf((char*)&strTmp, " %s %d 充电", VOIC_CARDTOUCH2, pCardctrl->led_index);
			// BswSrv_TtsPlay(strTmp);
		}
		else
		{
			// sprintf((char*)&strTmp, "%s", VOIC_CARDAUTH1); 
			// LOG("zhumang shuaka      M1_CARD                                gun working :%d.\r\n", gChgInfo.result); 
			// BswSrv_TtsPlay(strTmp); 
		}
	}
	else
	{
		if(pCardctrl->led_index != 0)
		{
			App_CkbProtoLedCtrol(pCardctrl->led_index, 0);
		}
		
	}
//    LOG("ckb protoVer = %s, ckbFault=0x%2X.\r\n", pHeartBeat->protoVer, pHeartBeat->faultCode);
    
}

void App_CkbProtoHandleData(uint8_t *data, uint16_t len)
{
    CKB_STR *pCkbPkt = (CKB_STR *)data;
//    PrintfData("recv data: ", data, len);

    LOG("recv ckb cmd = %d.\r\n", pCkbPkt->head.cmd);
    switch (pCkbPkt->head.cmd)
    {
        case CKB_CMD_HEARTBEAT:
            App_CkbProtoHandle_HeartBeat(pCkbPkt->data, pCkbPkt->head.len);
        break;

		case CKB_CMD_LED_CTRL:
		
        break;

        case CKB_CMD_CARD_CTRL:
			App_CkbProtoHandle_CardProc(pCkbPkt->data, pCkbPkt->head.len);
        break;

        default:
        break;
        
    }
}

void App_CkbProtoTask(void *pvParameters)
{
	uint8_t  data;
	uint8_t  pktBuff[256]={0};
	uint8_t  step = CKB_FIND_FF;
	uint16_t pktLen;
	uint16_t length;
	uint8_t  sum;
    uint32_t time;
    lastRecvCkbMsgTime = App_GetRtcCount();
    while (1)
    {
        if (CKB_FIND_FF != step) {
            if (2 < (uint32_t)(App_GetRtcCount() - time)) {
                LOG("too long not recv data,step=%d,error.\n",step);
                step = CKB_FIND_FF;
            }
        }
    	while (TRUE == App_UartGetOneData(DIS_UART, &data)) {
        	switch (step) {
        		case CKB_FIND_FF:
        		{
        			if (data == 0xFF) {
                        time = App_GetRtcCount();
        				pktLen=0;
        				pktBuff[pktLen] = 0xFF;
        				pktLen++;
        				step = CKB_FIND_BB;
        			}
        		}
        		break;
        		case CKB_FIND_BB:
        		{
        			if (data == 0xBB) {
        				pktBuff[pktLen] = 0xBB;
        				pktLen++;
        				step = CKB_FIND_LEN;
        			} else {
        				step = CKB_FIND_FF;
        			}
        		}
        		break;
        		case CKB_FIND_LEN:
        		{
        			if (pktLen == 2) {
        				length = data;
        			} else if (pktLen == 3) {
        				length |= (data << 8);
        				sum = 0;
                        if (5 > length) {
                            LOG("length=%d,error.\n", length);
                            step = CKB_FIND_FF;
                            break;
                        }
                        step = CKB_FIND_VER;
        			}
        			pktBuff[pktLen] = data;
        			pktLen++;
        		}
        		break;
        		case CKB_FIND_VER:
        		{
        			if (data == 0x01) {
        				sum += data;
        				pktBuff[pktLen] = data;
        				length--;
        				pktLen++;
        				step = CKB_FIND_SN;
        				//LOG("CKB find version = %d.\n", data);
        			} else {
        				step = CKB_FIND_FF;
        			}
        		}
        		break;
        		case CKB_FIND_SN:
        		{
        			sum += data;
        			pktBuff[pktLen] = data;
        			length--;
        			pktLen++;
        			step = CKB_FIND_CMD;
        			//LOG("CKB find sn = %d.\n", sn);
        		}
        		break;
        		case CKB_FIND_CMD:
        		{
        			sum += data;
        			pktBuff[pktLen] = data;
        			if ((--length) > 1) {
                        step = CKB_FIND_DATA;
                    }else{
                        step = CKB_FIND_CHECKSUM;
                    }
        			pktLen++;
        			//LOG("CKB find cmd = %d.\n", cmd);
        		}
        		break;
        		case CKB_FIND_DATA:
        		{
        			sum += data;
        			pktBuff[pktLen] = data;
        			pktLen++;
        			length--;
        			if (length==1) {
        				step = CKB_FIND_CHECKSUM;
        			}
        		}
        		break;
        		case CKB_FIND_CHECKSUM:
        		{
        			pktBuff[pktLen] = data;
        			pktLen++;
        			if (data == sum) {
        				lastRecvCkbMsgTime = App_GetRtcCount();
        				App_CkbProtoHandleData((void*)pktBuff, pktLen);
        			} else {
        				LOG("CKB checksum err.\n");
        			}
        			step = CKB_FIND_FF;
        		}
        		break;
        		default:
        		{
        			step = CKB_FIND_FF;
        		}
        		break;
        	}
        }
        vTaskDelay(20);
    }
    
}











