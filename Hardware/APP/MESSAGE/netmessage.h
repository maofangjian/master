#ifndef _NET_MESSAGE_H_
#define _NET_MESSAGE_H_
#include "global.h"
#include "order.h"
typedef enum {
    NET_CMD_START_UP = 1, //设备登陆
    NET_CMD_LOCATION,  //定位信息应答
    NET_CMD_NFC_REQ, //刷卡
    NET_CMD_REMOTE_CHARG, //远程启动充电
    NET_CMD_STOP_CHARGE, //远程停止充电
    NET_CMD_STOP_NOTIFY, //充电结束通知
    NET_CMD_HEARTBEAT, //心跳上报
    NET_CMD_UPGRADE, //升级
    NET_CMD_CONTROL, //远程配置
}MSG_CMD;

enum{
    CHARGE_NORMAL = 0, 
    FIRST_START,   
};

enum{
    DEVICE_REQ = 0, //设备请求
    DEVICE_RES,   //设备应答
};

enum{
    REMOTE_CONTROL_REBOOT = 0, //设备重启
    REMOTE_CONTROL_SETIP, //切换ip地址
};
void App_ChargeStartNotify(uint8_t port);
void App_ChargeStopNotify(ORDER_INFO *order);
void Bsp_MessagePack(MSG_CMD cmd,uint8_t *data,uint16_t len,uint8_t req);
void App_StartUpMessage(void);
void App_NetMessageHandler(uint8_t *data,uint16_t len);
void App_LocationMessage(void);
void App_HeartBeatSendMessage(void);
void App_NetMessagePro(uint8_t *data,uint8_t cmd ,uint16_t len);
void App_UpgradeAckHandler(uint8_t result);
void App_SendCardAuthReq(uint8_t flag);
#endif
