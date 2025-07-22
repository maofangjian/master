/*
 * @Author: chenyuan
 * @Date: 2025-04-08 17:19:29
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-04-11 18:41:42
 * @FilePath: \13.FreeRTOS实验\Hardware\APP\ORDER\order.c
 * @Description: 
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#include "order.h"
#include "flash.h"
#include "global.h"
#include "charge.h"
#include "start.h"
#include "sys.h"
#include "led.h"
#include "relay_control.h"
#include "netmessage.h"
#include "App_Voice.h"
ORDER_HEAD_DATA order_head_data;

void App_Order_Clear(void)
{
    order_head_data.read_index = 0;
    order_head_data.write_index = 0;
    App_Write_Flash((void*)&order_head_data,SYS_ORDER_HEAD_ADDR,sizeof(ORDER_HEAD_DATA));
}


uint8_t Bsp_Checkbuff(void)
{
    if(order_head_data.write_index >= RECORD_BLOCK_SIZE_PER_PAGE ||order_head_data.read_index >= RECORD_BLOCK_SIZE_PER_PAGE)
    {
        return FALSE;
    }
    return TRUE;
}

void App_OrderInit(void)
{
    App_Read_Flash((void*)&order_head_data,SYS_ORDER_HEAD_ADDR,sizeof(ORDER_HEAD_DATA));
    if(order_head_data.read_index >= RECORD_BLOCK_SIZE_PER_PAGE && order_head_data.write_index >= RECORD_BLOCK_SIZE_PER_PAGE)
    {
        App_Order_Clear();
    }
}

uint16_t App_GetOrderCnt(void)
{
    LOG("get index :%d %d\r\n",order_head_data.read_index,order_head_data.write_index);
    return (order_head_data.write_index+RECORD_BLOCK_SIZE_PER_PAGE-order_head_data.read_index)%RECORD_BLOCK_SIZE_PER_PAGE;
}

uint16_t App_WriteOrder(ORDER_INFO *order)
{
    if (Bsp_Checkbuff() == 0)
    {
        return 0;
    }
    uint32_t write_page = order_head_data.write_index/RECORD_BLOCK_NUM_PER_PAGE;
    uint32_t write_pos = order_head_data.write_index%RECORD_BLOCK_NUM_PER_PAGE;

    uint32_t write_address = write_page*FLASH_SECTOR_SIZE+write_pos*RECORD_BLOCK_SIZE_PER_PAGE;
    order->checksum = Bsp_AccCheckSum((void*)order,sizeof(ORDER_INFO)-2);
    App_Write_Flash((void*)order,SYS_ORDER_ADDR+write_address,sizeof(ORDER_INFO));
    order_head_data.write_index = (order_head_data.write_index+1)%RECORD_MAX_BLOCK_NUM;
    LOG("write index :%d %d\r\n",order_head_data.read_index,order_head_data.write_index);
    App_Write_Flash((void*)&order_head_data,SYS_ORDER_HEAD_ADDR,sizeof(ORDER_HEAD_DATA));
    App_ReadFirstOrder(order);
    return 0;
}

void App_GenerateOrderInfo(uint8_t port)
{
    SystemPortInfo_t *portinfo = &SystemOrderInfo[port - 1];
    ORDER_INFO order;
    memset(&order,0,sizeof(ORDER_INFO));
    order.port = port;
    order.chargemode = portinfo->chgMode;
    order.chargepar = portinfo->chgpara;
    for(uint8_t i = 0; i < SEGMENT_NUM_MAX; i++)
    {
        order.chargepowersegment[i] =  portinfo->SegmentChargingTime[i];
    }
    order.chargetime = portinfo->realChargingTime;
    order.ordercost = portinfo->cost_money_time;
    order.startelec = 0;
    order.stopelec = portinfo->chargingElec;
    order.starttime = portinfo->startTime;
    order.stopreason = portinfo->stopReason;
    order.stoptime = portinfo->stopTime;
    if (order.stopreason != STOP_FUSE_BREAK)
    {
        Port_charge_status[port-1] = PORTIDLE;
    }
     App_Led_Ctrl(portinfo->gunId,LED_ON);
    App_Relay_Control(portinfo->gunId,OFF);
    App_CkbProtoLedCtrol(port,PORTIDLE);
    BswSrv_TtsPlayStopChgTip(portinfo->gunId);
    memcpy(order.orderNo,portinfo->order,sizeof(portinfo->order));
    memset(portinfo,0,sizeof(SystemPortInfo_t));
    App_WriteOrder(&order);
    App_System_Order_Save();
    
}

uint16_t App_ReadFirstOrder(ORDER_INFO *order)
{
    if(App_GetOrderCnt() != 0)  //有订单未发送后台
    {
        uint32_t savepage = order_head_data.read_index/RECORD_BLOCK_NUM_PER_PAGE;
        uint32_t savepos = order_head_data.read_index%RECORD_BLOCK_NUM_PER_PAGE;
        uint32_t readaddr = savepage*FLASH_SECTOR_SIZE+savepos*RECORD_BLOCK_SIZE_PER_PAGE;
        App_Read_Flash((void*)order,SYS_ORDER_ADDR+readaddr,sizeof(ORDER_INFO));

        return order_head_data.read_index+1;
    }
    return FALSE;
}


void App_RemoveFirstOrder(void)
{
    uint8_t order_cnt = App_GetOrderCnt();
    if(order_cnt)
    {
        order_head_data.read_index = (order_head_data.read_index+1)%RECORD_MAX_BLOCK_NUM;
        LOG("read index :%d %d\r\n",order_head_data.read_index,order_head_data.write_index);
        App_Write_Flash((void*)&order_head_data,SYS_ORDER_HEAD_ADDR,sizeof(ORDER_HEAD_DATA));
    }
}



void App_SendOrderToBackstage(void)
{
    static uint16_t order_index = 0;
    ORDER_INFO order;
    uint16_t i = 0;
    memset(&order,0,sizeof(ORDER_INFO));
    if((i = App_ReadFirstOrder(&order)) != FALSE)
    {
        Charge_Param *chargeinfo = &ChargeInfo[order.port-1];
        if(order.checksum == Bsp_AccCheckSum((void *)&order,sizeof(order)-2))
        {
            App_ChargeStopNotify(&order);
            if(i != order_index)
            {
                order_index = i;
                chargeinfo->send_order_id = order.port;
                SystemInfo.sendordercnt = 0;
            }
            if(SystemInfo.sendordercnt++ >= 5)
            {
                App_RemoveFirstOrder();
                App_System_Order_Save();
                SystemInfo.sendordercnt = 0;
            }
        }
        else
        {
            App_RemoveFirstOrder();
        }
    }
}
