#ifndef _ORDER_H_
#define _ORDER_H_

#include "gd32f30x.h"
#include "global.h"
#define ORDER_MAX_NUM           20//最大订单数
#pragma pack(1)
typedef struct 
{
    uint16_t read_index;
    uint16_t write_index;
}ORDER_HEAD_DATA;


typedef struct 
{
    uint8_t port;       //端口号
    uint8_t stopreason;   //订单结束原因
    uint8_t orderNo[21];  //订单号
    uint32_t starttime;     //开始时间
    uint32_t stoptime;      //结束原因
    uint32_t startelec;     //开始充电电量
    uint32_t stopelec;      //结束时电量
    uint32_t ordercost;    //订单费用
    uint32_t template;        //计费模版
    uint8_t  chargemode;    //充电模式
    uint32_t chargepar;     //充电参数
    uint32_t chargetime;   //充电时间
    uint32_t chargepowersegment[SEGMENT_NUM_MAX];    //充电功率段
    uint16_t checksum;
}ORDER_INFO;
#pragma pack()

void App_GenerateOrderInfo(uint8_t port);
void App_OrderInit(void);
void App_RemoveFirstOrder(void);
void App_SendOrderToBackstage(void);
uint16_t App_WriteOrder(ORDER_INFO *order);
uint16_t App_ReadFirstOrder(ORDER_INFO *order);
#endif
