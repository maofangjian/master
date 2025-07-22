#include "Device_check.h"
#include "meter.h"
#include "relay_control.h"
#include "rtc.h"
#include "timer.h"

/**
 * @description: 通道3过流中断
 * @Author: 
 * @Date: 2025-03-17 15:13:16
 * @return {*}
 */
void EXTI0_IRQHandler(void)
{
    Port_Status_Def port_data;
    if (RESET != exti_interrupt_flag_get(CURRENT_OVER3_EXTI_LINE) && (MeterInitflag == 1)) 
	{
        Bsp_GetPortStatus(2,&port_data);
        port_data.status |= 0x04;
        Bsp_SetPortStatus(2,&port_data);
		exti_interrupt_flag_clear(CURRENT_OVER3_EXTI_LINE);
    }
}


/**
 * @description: 通道4过流中断
 * @Author: 
 * @Date: 2025-03-17 15:13:16
 * @return {*}
 */
void EXTI1_IRQHandler(void)
{
    Port_Status_Def port_data;
    if (RESET != exti_interrupt_flag_get(CURRENT_OVER4_EXTI_LINE) && (MeterInitflag == 1)) 
	{
        Bsp_GetPortStatus(3,&port_data);
        port_data.status |= 0x04;
        Bsp_SetPortStatus(3,&port_data);
		exti_interrupt_flag_clear(CURRENT_OVER4_EXTI_LINE);
    }
}


/**
 * @description: 通道5过流中断
 * @Author: 
 * @Date: 2025-03-17 15:13:16
 * @return {*}
 */
void EXTI2_IRQHandler(void)
{
    Port_Status_Def port_data;
    if (RESET != exti_interrupt_flag_get(CURRENT_OVER5_EXTI_LINE) && (MeterInitflag == 1)) 
	{
        Bsp_GetPortStatus(4,&port_data);
        port_data.status |= 0x04;
        Bsp_SetPortStatus(4,&port_data);
		exti_interrupt_flag_clear(CURRENT_OVER5_EXTI_LINE);
    }
}


/**
 * @description: 通道6过流中断
 * @Author: 
 * @Date: 2025-03-17 15:13:16
 * @return {*}
 */
void EXTI3_IRQHandler(void)
{
    Port_Status_Def port_data;
    if (RESET != exti_interrupt_flag_get(CURRENT_OVER6_EXTI_LINE) && (MeterInitflag == 1)) 
	{
        Bsp_GetPortStatus(5,&port_data);
        port_data.status |= 0x04;
        Bsp_SetPortStatus(5,&port_data);
		exti_interrupt_flag_clear(CURRENT_OVER6_EXTI_LINE);
    }
}

/**
 * @description: 通道8过流中断
 * @Author: 
 * @Date: 2025-03-17 15:13:16
 * @return {*}
 */
void EXTI4_IRQHandler(void)
{
    Port_Status_Def port_data;
    if (RESET != exti_interrupt_flag_get(CURRENT_OVER8_EXTI_LINE) && (MeterInitflag == 1)) 
	{
        Bsp_GetPortStatus(7,&port_data);
        port_data.status |= 0x04;
        Bsp_SetPortStatus(7,&port_data);
		exti_interrupt_flag_clear(CURRENT_OVER8_EXTI_LINE);
    }
}

/**
 * @description: 
 * @Author: 
 * @Date: 2025-03-17 15:13:16
 * @return {*}
 */
void EXTI5_9_IRQHandler(void)
{
    Port_Status_Def port_data;
    if (RESET != exti_interrupt_flag_get(CURRENT_OVER1_EXTI_LINE) && (MeterInitflag == 1)) //通道1过流中断
	{
        Bsp_GetPortStatus(0,&port_data);
        port_data.status |= 0x04;
        Bsp_SetPortStatus(0,&port_data);
		exti_interrupt_flag_clear(CURRENT_OVER1_EXTI_LINE);
    }

    if (RESET != exti_interrupt_flag_get(CURRENT_OVER2_EXTI_LINE) && (MeterInitflag == 1)) //通道2过流中断
	{
        Bsp_GetPortStatus(1,&port_data);
        port_data.status |= 0x04;
        Bsp_SetPortStatus(1,&port_data);
		exti_interrupt_flag_clear(CURRENT_OVER2_EXTI_LINE);
    }

    if (RESET != exti_interrupt_flag_get(CURRENT_OVER7_EXTI_LINE) && (MeterInitflag == 1)) //通道7过流中断
	{
        Bsp_GetPortStatus(6,&port_data);
        port_data.status |= 0x04;
        Bsp_SetPortStatus(6,&port_data);
		exti_interrupt_flag_clear(CURRENT_OVER7_EXTI_LINE);
    }

    if (RESET != exti_interrupt_flag_get(CURRENT_OVER10_EXTI_LINE) && (MeterInitflag == 1)) //通道10过流中断
	{
        Bsp_GetPortStatus(9,&port_data);
        port_data.status |= 0x04;
        Bsp_SetPortStatus(9,&port_data);
		exti_interrupt_flag_clear(CURRENT_OVER10_EXTI_LINE);
    }

    //过零中断
    if (RESET != exti_interrupt_flag_get(ZERO_DET_EXTI_LINE)) 
    {  
        exti_interrupt_flag_clear(ZERO_DET_EXTI_LINE);
        if(gpio_input_bit_get(ZERO_DET_PORT,ZERO_DET_PIN) == 0)//电压过零点中断
        {
            if(relay_open != 0)//开继电器
            {
                relay_flag = 1;
                App_Relay_Delay_Start(37);	
            }
            else if(relay_close != 0)
            {
                relay_flag = 0;
                App_Relay_Delay_Start(42);
            }
        }
    }

}


/**
 * @description: 
 * @Author: 
 * @Date: 2025-03-17 16:33:22
 * @return {*}
 */
void EXTI10_15_IRQHandler(void)
{
    Port_Status_Def port_data;
    if (RESET != exti_interrupt_flag_get(CURRENT_OVER9_EXTI_LINE) && (MeterInitflag == 1)) //通道9过流中断
	{
        Bsp_GetPortStatus(8,&port_data);
        port_data.status |= 0x04;
        Bsp_SetPortStatus(8,&port_data);
		exti_interrupt_flag_clear(CURRENT_OVER9_EXTI_LINE);
    }

    if (RESET != exti_interrupt_flag_get(CURRENT_OVER11_EXTI_LINE) && (MeterInitflag == 1)) //通道11过流中断
	{
        Bsp_GetPortStatus(10,&port_data);
        port_data.status |= 0x04;
        Bsp_SetPortStatus(10,&port_data);
		exti_interrupt_flag_clear(CURRENT_OVER11_EXTI_LINE);
    }
    if (RESET != exti_interrupt_flag_get(CURRENT_OVER12_EXTI_LINE) && (MeterInitflag == 1)) //通道12过流中断
	{
        Bsp_GetPortStatus(11,&port_data);
        port_data.status |= 0x04;
        Bsp_SetPortStatus(11,&port_data);
		exti_interrupt_flag_clear(CURRENT_OVER12_EXTI_LINE);
    }

}

/**
 * @description: 订单开启保险丝自检
 * @Author: 
 * @Date: 2025.3.27 23.44.25
 * @return 0 :保险丝正常  -1:保险丝异常
 */
uint8_t App_OpenOrder_Check(uint8_t port)
{
    uint8_t ret = 0;
    App_Relay_Control(port,ON);
    memset(&Fuse_Det[port-1],0,sizeof(Fuse_Def));
    Delay_ms(1000);
    ret = App_Fuse_Check(port-1,0);
    App_Relay_Control(port,OFF);
    return ret;
}

/**
 * @description: 保险丝单口自检
 * @Author: 
 * @Date: 2025.3.27 23.44.25
 * @return 0 :保险丝正常  -1:保险丝异常
 */
int App_Fuse_Check(uint8_t port,uint8_t check_all)
{
    uint16_t high_duty = 0;
    uint16_t low_duty = 0;
    static uint32_t time = 0;
    Port_Status_Def port_data;
    if(App_GetRtcCount() - time > 0 || (check_all == 1))
    {
        time = App_GetRtcCount();
        high_duty = (Fuse_Det[port].high_time*100)/Fuse_Det[port].total_time;
        low_duty = (Fuse_Det[port].low_time*100)/Fuse_Det[port].total_time;
        LOG("port :%d duty:%d high_duty :%d low_duty :%d total:%d\r\n",port,high_duty,Fuse_Det[port].high_time,Fuse_Det[port].low_time,Fuse_Det[port].total_time);
        if (high_duty >= 95 && low_duty < 15 && Fuse_Det[port].total_time > 50)  //保险丝异常
        {
            Bsp_GetPortStatus(port,&port_data);
            port_data.status |= 0x08;   //保险丝异常
            Bsp_SetPortStatus(port,&port_data);
            memset(&Fuse_Det[port],0,sizeof(Fuse_Def));
            LOG("port :%d error\r\n",port);
            Port_charge_status[port] = PORTFAULT;
            return FAIL;
        }
        else if(high_duty >= 15 && high_duty <= 85 && Fuse_Det[port].total_time > 50) //保险丝正常
        {
            Bsp_GetPortStatus(port,&port_data);
            port_data.status &= ~0x08;   //保险丝正常
            Bsp_SetPortStatus(port,&port_data);
            memset(&Fuse_Det[port],0,sizeof(Fuse_Def));
            return OK;  
        }
    }
	return OK;
        
}

/**
 * @description: 晚上12点保险丝全口自检
 * @Author: 
 * @Date: 2025.3.27 23.45.26
 * @return 
 */
void App_Fuse_Deal_Task(void)
{
    static uint32_t time  = 0;
    static uint8_t check_flag = 1;
    SystemPortInfo_t *portinfo = NULL;
    char day[7];
    if (App_GetRtcCount() - time > 0)
    {
        time = App_GetRtcCount();
        App_Get_Rtc_Time(day);
        if((day[4] == 0)&&(day[5] == 0)&&(check_flag == 1))
        {
            check_flag = 0;
            for (uint8_t i = 1; i <= PORTMAX; i++)
            {
                portinfo = &SystemOrderInfo[i - 1];
                if (portinfo->gunChgStatu == PORT_CHARGING_IDLE)
                {
                    App_Relay_Control(i,ON);
                    memset(&Fuse_Det[i-1],0,sizeof(Fuse_Def));
                    Delay_ms(1000);
                    App_Fuse_Check(i-1,0);
                    App_Relay_Control(i,OFF);
                }
            }
        }
        if((day[4] != 0)||(day[5] != 0))
        {
            check_flag = 1;
        }
    }
    
}


/**
 * @description: 充电检测保险丝
 * @Author: 
 * @Date: 2025.3.27 23.45.26
 * @return 
 */
void App_ChgCtrlProcFuseStatus(void)
{
    SystemPortInfo_t *portinfo = NULL;
    uint8_t ret = 0;
    static uint32_t fuse_time = 0;
    static uint32_t fuse_error_time[PORTMAX] = {0};
    if(App_GetRtcCount() - fuse_time > 0)
    {
        fuse_time =  App_GetRtcCount();
        for (uint8_t i = 1; i <= PORTMAX; i++)
        {
            portinfo = &SystemOrderInfo[i - 1];
            if (portinfo->gunChgStatu) //正在充电中
            {
                ret = App_Fuse_Check(i-1,0);
                if(ret == FAIL)
                {
                    fuse_error_time[i-1]++;
                    if(fuse_error_time[i-1] >= 3)
                    {
                        portinfo->stopReason = STOP_FUSE_BREAK;
                        App_StopChargingPro(i);//上报结束订单
                        Port_charge_status[i-1] = PORTFAULT;
                    }
                }
                else
                {
                    fuse_error_time[i-1] = 0;
                }
            }
        }
        memset(&Fuse_Det[0],0,sizeof(Fuse_Def)*PORTMAX);
    }
}

