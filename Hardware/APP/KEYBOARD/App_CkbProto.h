/*** 
 * @Author: chenyuan
 * @Date: 2025-05-22 18:29:24
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-05-28 10:41:21
 * @FilePath: \13.FreeRTOS实验\Hardware\APP\KEYBOARD\App_CkbProto.h
 * @Description: 
 * @
 * @Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#ifndef __APP_CKBPROTO_H__
#define __APP_CKBPROTO_H__

#include "global.h"
#include "charge.h"

#define CKB_PKT_LEN				256


typedef enum {
    CKB_CMD_HEARTBEAT = 1,
    CKB_CMD_LED_CTRL = 2,
    CKB_CMD_CARD_CTRL = 3,
}CKB_CMD;

typedef enum {
	CKB_FIND_FF=0,
	CKB_FIND_BB=1,
	CKB_FIND_LEN=2,
	CKB_FIND_VER=3,
	CKB_FIND_SN=4,
	CKB_FIND_CMD=5,
	CKB_FIND_DATA=6,
	CKB_FIND_CHECKSUM=7,
}CKB_FIND;

#pragma pack(1)
typedef struct {
    uint8_t  ff;                                    //0.
    uint8_t  bb;                                    //1.
	uint16_t len;								    //2.长度
    uint8_t  ver;                                   //3.版本号
    uint8_t  sn;                                    //4.报文流水号
    uint8_t  cmd;                                   //6.命令代码
}CKB_HEAD_STR;



typedef struct {
    CKB_HEAD_STR head;
    uint8_t  data[CKB_PKT_LEN-sizeof(CKB_HEAD_STR)];
}CKB_STR;

typedef struct {
    uint8_t protoVer[16];         //按键板版本号
    uint8_t faultCode;            //器件故障
}CKB_HEARTBEAT_REPORT;

typedef struct {
    PORT_STATUS gunStatu[PORTMAX];         //Bit0: 0 空闲 1使用中  Bit1: 充满位  BIit2:故障位 
}CKB_HEARTBEAT_REPORT_ACK;

typedef struct {
    uint8_t  key;                   //0控制全灭全亮,1~12代表按键1~12
    PORT_STATUS  lightOnFlag;       
}CKB_LED_CTRL;

typedef struct{
    uint8_t cardfalg;         //刷卡控制，0:无刷卡 1:刷卡
    uint8_t led_index;        //返回按下的按键
}CKB_CARD_CTRL;

typedef struct {
    uint8_t result;             //0接收成功；1接收失败
}CKB_LED_CTRL_ACK;
#pragma pack()
extern bool led_keyboard;
void App_CkbProtoHeartBeatAck(void);
void App_CkbProtoTask(void *pvParameters);
void App_CkbConnectStatuProc(void);
void App_CkbProtoLedCtrol(uint8_t ledIndex, uint8_t lightOnFlag);
void App_CkbProtoCardCtrol(uint8_t cardflag);

#endif //__APP_CKBPROTO_H__

