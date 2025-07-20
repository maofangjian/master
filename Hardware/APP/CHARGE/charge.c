#include "charge.h"
#include "led.h"
#include "system.h"
#include "relay_control.h"
#include "rtc.h"
#include "meter.h"
#include "FreeRTOS.h"
#include "start.h"
#include "semphr.h"
#include "order.h"
#include "global.h"
#include "netmessage.h"
#include "Device_check.h"
SystemPortInfo_t SystemOrderInfo[PORTMAX];
PORT_CHARGING_FULL_TO_EXIT_STR   gunChargingFullToExitInfo[PORTMAX] = {0}; //枪头假充电
Charge_Param ChargeInfo[PORTMAX];
SemaphoreHandle_t Chgmutex = NULL;
PORT_STATUS Port_charge_status[PORTMAX];
void App_Restore_Charge_State(void)
{
    SystemPortInfo_t *portinfo = NULL;
    Port_Status_Def port_data;
    App_Relay_Control(0,ON);
    Delay_ms(100);
    memset(&Fuse_Det[0],0,sizeof(Fuse_Def)*PORTMAX);
    Delay_ms(1000);
    for(uint8_t i = 1; i <= PORTMAX; i++)
    {                    
        App_Fuse_Check(i-1,1);
        App_Relay_Control(i,OFF);
        portinfo = &SystemOrderInfo[i - 1];
        Bsp_GetPortStatus(i-1,&port_data);
        if(portinfo->gunChgStatu != PORT_CHARGING_IDLE && ((port_data.status&&0x08) == 0))
        {
            Delay_ms(100);
            App_Relay_Control(i,ON);
            App_Led_Ctrl(i,LED_OFF);
            App_ChargeInit(portinfo->gunId);
            portinfo->gunChgStatu = PORT_CHARGING_UNKNOW_POWER;
            portinfo->isSync = POWER_RECOVER;
        }  
    }
}

void App_GetChargeTime(uint8_t port,uint32_t power)
{
    SystemPortInfo_t *portinfo = &SystemOrderInfo[port-1];
    // segment_str *segmet = &portinfo->powerInfo.segmet[0];
//    for(uint8_t i = 0; i < SEGMENT_NUM_MAX; i++)
//    {
//        if(segmet->startPower <= (power/10)&&segmet->endPower > (power/10))
//        {
//            portinfo->powerSegmentIndex = i;
//        }
//        segmet++;
//    }
}


/*
    充电控制判断网络状态，充电中断网后重新联网需重新上报充电开启通知
*/
void App_ChgCtrlProcNetStatus(void)
{
    SystemPortInfo_t *pGunInfo = NULL;
    static int is_online_history = 1;
    int is_online;

    if (SystemInfo.onlineflag && SystemInfo.connect_server_flag)
    {
        is_online = 1;
    }
    else
    {
        is_online = 0;
    }
    
    if (is_online != is_online_history)
    {
        if (1 == is_online)
        {
            for (int i = 1; i <= PORTMAX; i++)
            {
                pGunInfo = &SystemOrderInfo[i - 1];
                if (pGunInfo->gunChgStatu)
                {
                    //充电开启通知同步NET_RECOVER状态
                    pGunInfo->isSync = NET_RECOVER;
                }
            }
        }
        is_online_history = is_online;
    }
}

void App_ChargeInit(uint8_t port)
{
    Charge_Param *chargeinfo = &ChargeInfo[port-1];
    chargeinfo->chargefullpower = CHARGE_FULL_POWER2;
    chargeinfo->chargefulltime = 60*60; //1小时判断充满
}

void App_PowerControl(uint8_t port,uint32_t power,SystemPortInfo_t *portinfo)
{
    uint32_t rtc_time = App_GetRtcCount();
    uint32_t time = 0;
    Charge_Param *chargeinfo = &ChargeInfo[port-1];
    if(power <= CHARGE_FULL_POWER1)
    {
        if(portinfo->gunChgStatu != PORT_CHARGING_GUN_PULL)
        {
            portinfo->gunChgStatu = PORT_CHARGING_GUN_PULL;
            chargeinfo->starttime = rtc_time;
        }
        else
        {
            if((rtc_time - portinfo->startTime) >= 120 )  //两分钟检测拔出
            {
                time = rtc_time - chargeinfo->starttime;
                if((power == 0) && (time >= 30))
                {
                    portinfo->stopReason = STOP_PULL_PORT;
                    LOG("port pull out\r\n");
                }
                else if((power > 0) && (time >= 30))
                {
                    portinfo->stopReason = STOP_CHARGE_FULL;
                    LOG("port charge full\r\n");
                }

            }
        }
    }
    else 
    {
        if((power <= CHARGE_FULL_POWER2) || (gunChargingFullToExitInfo[port-1].ChargeFullExitFlag == 1))
        {
            
            if(portinfo->gunChgStatu != PORT_CHARGING_FULL)
            {
                Port_charge_status[port-1] = PORTFULL;
                portinfo->gunChgStatu = PORT_CHARGING_FULL;
                if(gunChargingFullToExitInfo[port-1].ChargeFullExitFlag == 0)
                {
                    chargeinfo->starttime = rtc_time;
                    gunChargingFullToExitInfo[port-1].BeginTimebackup =rtc_time;
                }
                else
                {
                    chargeinfo->starttime = gunChargingFullToExitInfo[port-1].BeginTimebackup;
                }
            }
            else
            {
                gunChargingFullToExitInfo[port-1].ChargeFullExitFlag = 1;
                if(power < CHARGE_FULL_POWER2)
                {
                    gunChargingFullToExitInfo[port-1].FullbeginTime = rtc_time;
                }
                if(rtc_time - gunChargingFullToExitInfo[port-1].FullbeginTime >= 60)  //假充时间一分钟
                {
                    gunChargingFullToExitInfo[port-1].ChargeFullExitFlag = 0;
                }
                if((rtc_time - chargeinfo->starttime) >= chargeinfo->chargefulltime) //充满时间判断
                {
                    LOG("Device charge pull\r\n");
                    portinfo->stopReason = STOP_CHARGE_FULL;
                    return;
                }
            }
        }
        else if(power > CHARGE_FULL_POWER2)
        {
            Port_charge_status[port-1] = PORTCHARGING;
            portinfo->gunChgStatu = PORT_CHARGING_WORK;
            if(portinfo->gunState == POER_PLUG_IN)
            {
                portinfo->gunState = PORT_CHARGE;
            }
        }
    }
    App_GetChargeTime(port,power);
}

void App_BackgroundOpenProc(void)
{
    Port_Status_Def portstatus;
    Charge_Param *chargeinfo;
    for(uint8_t i = 1; i <= PORTMAX; i++)
    {
        chargeinfo = &ChargeInfo[i-1];
        Bsp_GetPortStatus(i-1,&portstatus);
        if(chargeinfo->istest)
        {
            if(portstatus.status != 0)
            {
                chargeinfo->istest = 0;
                App_Relay_Control(i,OFF);
            }
        }
    }
}

//充电金额计算
void App_ChgCtrlMoneyCount(void)
{
    SystemPortInfo_t *portinfo = NULL;
	uint32_t chgpara_time;
    uint32_t power_server_cost = 0;  
    uint32_t elec_price = 0;
    uint32_t max_power_segment = 0;
    for (uint8_t i = 1; i <= PORTMAX; i++)
    {
        portinfo = &SystemOrderInfo[i - 1];
        if (portinfo->gunChgStatu) //正在充电中
        {
            power_server_cost = 0;
            elec_price = 0;
            if(portinfo->chgMode == CHARGING_TIME_MODE)
            {
                portinfo->cost_money_time = portinfo->realChargingTime;
                if (portinfo->cost_money_time >= portinfo->chgpara)
                {
                    portinfo->cost_money_time = portinfo->chgpara;
                    portinfo->stopReason = STOP_NO_MONEY;
                    App_StopChargingPro(i);//上报结束订单
                }
            }
            else if(portinfo->chgMode == CHARGING_ELEC_POWER_MODE)
            {
                elec_price = portinfo->chargingElec*portinfo->chgpara/100; //电价 = 电量*单价
                for (uint8_t n = 0; n < SEGMENT_NUM_MAX; n++)
                {
                    if (portinfo->SegmentChargingTime[n] != 0 && ((portinfo->SegmentChargingTime[n]*100/portinfo->realChargingTime) >=10)) //最大功率时长占总充电时长1/10;
                    {
                        max_power_segment = n; //找充电最大时长的那段
                    }
                    
                }
                power_server_cost = portinfo->SegmentChargingPrice[max_power_segment]*portinfo->realChargingTime/60;
                portinfo->cost_money_time = elec_price + power_server_cost;
                if (portinfo->cost_money_time >= portinfo->money)
                {
                    portinfo->cost_money_time = portinfo->money;
                    portinfo->stopReason = STOP_NO_MONEY;
                    App_StopChargingPro(i);//上报结束订单
                }
                
            }
            else if(portinfo->chgMode == CHARGING_POWER_MODE)
            {
                for (uint8_t n = 0; n < SEGMENT_NUM_MAX; n++)
                {
                    if (portinfo->SegmentChargingTime[n] != 0 && ((portinfo->SegmentChargingTime[n]*100/portinfo->realChargingTime) >=10)) //最大功率时长占总充电时长1/10;
                    {
                        max_power_segment = n; //找充电最大时长的那段
                    }
                    
                }
                

                power_server_cost = portinfo->SegmentChargingPrice[max_power_segment]*portinfo->realChargingTime/60;
                LOG("max_power_segment:%d,real:%d,power_server_cost:%d\r\n",max_power_segment,portinfo->realChargingTime,power_server_cost);
                portinfo->cost_money_time =  power_server_cost;
                if (portinfo->cost_money_time >= portinfo->money)
                {
                    portinfo->cost_money_time = portinfo->money;
                    portinfo->stopReason = STOP_NO_MONEY;
                    App_StopChargingPro(i);//上报结束订单
                }
            }
            else if(portinfo->chgMode == CHARGING_ELEC_MODE)
            {
                elec_price = portinfo->chargingElec*portinfo->chgpara/100; //电价 = 电量*单价
                power_server_cost = portinfo->chargingElec*portinfo->elec_server_price/100; //电价 = 电量*单价
                portinfo->cost_money_time =  power_server_cost + elec_price;
                if (portinfo->cost_money_time >= portinfo->money)
                {
                    portinfo->cost_money_time = portinfo->money;
                    portinfo->stopReason = STOP_NO_MONEY;
                    App_StopChargingPro(i);//上报结束订单
                }

            }


                if (portinfo->realChargingTime >= CHARGE_MAX_TIME)
                {
                    portinfo->stopReason = STOP_OVER_12H;
                    App_StopChargingPro(i);//上报结束订单
                }
                
                
            }
        
    }
    
}

void App_StopChargingPro(uint8_t port)
{
    SystemPortInfo_t *portinfo = &SystemOrderInfo[port - 1];
    portinfo->gunId = port;
    portinfo->gunState = PORT_IDLE;
    portinfo->gunChgStatu = PORT_CHARGING_IDLE;
    portinfo->chargesendcnt = 0;
    portinfo->stopTime = App_GetRtcCount();
    LOG("stop reason :%d\r\n",portinfo->stopReason);
    App_GenerateOrderInfo(port);
}


void App_CaculateChargeMoney(SystemPortInfo_t *portinfo)
{

}

void App_ChagreControl(void)
{
    Port_Status_Def portstatus;
    uint32_t total_power = 0;
    SystemPortInfo_t *portinfo = NULL;
    Charge_Param *chargeinfo = NULL;
    uint32_t time = App_GetRtcCount();
    for(uint8_t i = 1; i <= PORTMAX; i++)
    {
        chargeinfo = &ChargeInfo[i-1];
        portinfo = &SystemOrderInfo[i - 1];
        Bsp_GetPortStatus(i-1,&portstatus);
        if(portinfo->gunChgStatu)
        {
            LOG("port :%d gunChgStatu:%d\r\n",i,portinfo->gunChgStatu);
            if(portstatus.status & 0x01)
            {
                portinfo->stopReason = STOP_OVER_CURRENT;
            }
            else if(portstatus.status & 0x02)
            {
                portinfo->stopReason = STOP_OVER_VOLTAGE;
            }
            else if(portstatus.status & 0x04)
            {
                portinfo->stopReason = STOP_OVER_SHORT;
            }
            else if(portstatus.status & 0x08)
            {
                portinfo->stopReason = STOP_FUSE_BREAK;
            }
            else if(portstatus.status & 0x10)
            {
                portinfo->stopReason = STOP_OVER_TEMP;
            }

            else if(portstatus.status == 0)
            {
                if(portinfo->stopReason == STOP_NULL)
                {
                    // if(portstatus.energy > portinfo->startElec)
                    // {
                    //     portinfo->chargingElec += portstatus.energy - portinfo->startElec;
                    //     portinfo->startElec = portstatus.energy;
                    // }
                    // else if(portstatus.energy < portinfo->startElec)
                    // {
                    //     portinfo->startElec = portstatus.energy;
                    // }
                    App_PowerControl(i,portstatus.power,portinfo);
                }

            }
            if(portinfo->stopReason == STOP_NULL)
            {
                if(PORT_MAX_POWER <= (portstatus.power/10))
                {
                    portinfo->stopReason = STOP_OVER_PORT_POWER;
                    LOG("power too large %d %d\r\n",i,portstatus.power);
                }
                total_power += portstatus.power/10;
                if(TOTAL_MAX_POWER <= total_power)
                {
                    LOG("device too large %d %d\r\n",i,portstatus.power);
                    portinfo->stopReason = STOP_OVER_TOTAL_POWER;
                }
            }
            if(portinfo->stopReason != STOP_NULL)
            {
                LOG("stop reason %d\r\n",portinfo->stopReason);
                App_StopChargingPro(i);//上报结束订单
            }

            if((time - chargeinfo->chargetime) >= 60)  //一分钟统计一次功率
            {
                chargeinfo->chargetime = time;
                portinfo->realChargingTime++;
                portinfo->temporary_elec += portstatus.power*0.1/0.1/100/60;
                portinfo->chargingElec = (uint32_t)portinfo->temporary_elec;
                
                portinfo->powerSegmentIndex = portstatus.power/10/100;
                portinfo->SegmentChargingTime[portinfo->powerSegmentIndex]++;
                if(portinfo->chgMode == 1) //一口价
                {
                   // App_CaculateChargeMoney(portinfo);
                }
            }
        }
        else
        {
            gunChargingFullToExitInfo[i-1].ChargeFullExitFlag = 0;
        }
        vTaskDelay(50);
    }
}


void App_ChargeTask(void *pvParameters)
{
    uint32_t time = 0;
    uint8_t i = 0;
    Port_Status_Def port_data;
    uint32_t time_cnt = 0;
    Chgmutex = xSemaphoreCreateMutex();
    App_Relay_Control_All(OFF);  //关闭所有继电器
    while (1)
    {
        vTaskDelay(50);
        if(App_GetRtcCount() - time != 0)
        {
            time = App_GetRtcCount();
            if(MeterInitflag == 1)
            {
                MeterInitflag = 0;
                App_System_Order_Load();
                App_Energy_Load();
            }
            for(i = 0; i < BL_CHANEL_MAX; i++)
            {
                App_Deal_Bl0939_Data((BL0939_ADDR_DEF)i);
            }
            if(time_cnt % 10 == 0)
            {
                for (uint8_t i = 0; i < PORTMAX; i++)
                {
                    Bsp_GetPortStatus(i,&port_data);
                    LOG("port :%d volatage : %d current :%d power :%d elec :%d\r\n",i+1,port_data.volatage,port_data.current,port_data.power,port_data.energy);
                }
            }
            App_ChagreControl();  //充电状态处理
            App_BackgroundOpenProc();
            time_cnt++;
        }
        FEED_WDG();
    }
    
}

