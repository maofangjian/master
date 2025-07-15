#ifndef _START_H_
#define _START_H_

#include "gd32f30x.h"
#include "charge.h"
#include "global.h"
#include "BswAbstr_Ec200s.h"
#pragma pack(1)

typedef struct 
{
    uint8_t device_id[128];   //设备ID
    uint8_t mqttcount[128];  //mqtt账号
    uint8_t mqttpassword[128];  //mqtt密码
    uint8_t serverurl[128];   //域名
    uint32_t serverport;    //端口号
    uint8_t sim_id[ICCID_LEN+1];       //sim卡id
    uint8_t ec200_version[32]; //4G模块版本号
    uint8_t csq;            //信号强度
    uint8_t onlineflag;    //在线标志位
    uint8_t connect_server_flag; //连接服务器标志
    uint8_t temp[3];      //温度
    uint8_t ckbVer[16];    //按键板版本号
    uint8_t sendordercnt;   //订单发送次数
    uint8_t  nfcCardState; //nfc刷卡状态
    uint16_t totalpower; //充电总功率
    uint32_t faultstate[PORTMAX]; //故障码
    uint8_t  sendrecordtimes; //发送交易次数
    uint8_t  sendheartbeattimes;//发送心跳次数
    uint32_t portinfostashtime; //端口信息存储时间 20分钟存储一次

}System_Config;

#pragma pack()
extern System_Config SystemInfo;
void App_System_Order_Load(void);
void System_Start_Config(void);
void App_System_Order_Save(void);
#endif
