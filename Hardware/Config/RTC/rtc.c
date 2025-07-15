#include "rtc.h"
#include "global.h"
static uint32_t rtc_time = 0;
//时间戳转换为时间
int Rtc_To_Realtime(time_t time, uint8_t *Day)
{
    struct tm *time_now;

    time_now = localtime(&time);
    Day[0] = time_now->tm_wday;
    Day[1] = time_now->tm_year - 100;
    Day[2] = time_now->tm_mon + 1;
    Day[3] = time_now->tm_mday;
    Day[4] = time_now->tm_hour;
    Day[5] = time_now->tm_min;
    Day[6] = time_now->tm_sec;
    return SUCCESS;
}
//时间戳获取
void App_Get_Rtc_Time(uint8_t *day)
{
    uint32_t time;
    time = rtc_counter_get();
    Rtc_To_Realtime(time, day);
}


uint32_t App_GetRtcCount(void)
{
    return rtc_time;
}

//rtc时间戳设置
void App_Set_Rtc_Time(uint32_t time)
{
    rtc_time = time;
}

//rtc时钟配置
void rtc_configuration(void)
{
 
    /* 备份域复位 */
    bkp_deinit();
 
    /* 使能外部低速时钟 */
    rcu_osci_on(RCU_IRC40K);
    /* 等待低速晶体振荡器稳定 */
    rcu_osci_stab_wait(RCU_IRC40K);
 
    /* RTC 时钟源选择 */
    rcu_rtc_clock_config(RCU_RTCSRC_IRC40K);
 
    /* 使能RTC时钟 */
    rcu_periph_clock_enable(RCU_RTC);
 
    /* 等待寄存器与APB1时钟同步 */
    rtc_register_sync_wait();
 
    /* 等待最后一次操作完成 */
    rtc_lwoff_wait();

    /*设置预分频 内部时钟 40 kHz 分频后就是 1S*/
    rtc_prescaler_set(40590);
 
    /* 等待最后一次操作完成 */
    rtc_lwoff_wait();
}
 
void nvic_rtc_configuration(void)
{
    nvic_irq_enable(RTC_IRQn, 1, 1);
}


void time_adjust(uint32_t current_senconds)
{
 
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    pmu_backup_write_enable();
    
    /* 等待最后一次操作完成*/
    rtc_lwoff_wait();
    
    /* 改变当前时间 */
    rtc_counter_set(current_senconds+8*60*60);
    
    /*等待最后一次操作完成 */
    rtc_lwoff_wait();
 
}

void RTC_init(void)
{
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    pmu_backup_write_enable();

    rtc_configuration();
        /* 首次调整时间  传入一个时间戳*/
    time_adjust(1743668026);


    

    // //读取备份区寄存器0的数据，也就是 查看时间是否已经配置
    // if (1)
    // {
    //     /* 备份寄存器的值不是0xa5a5,说明RTC没有配置过 */
    //     /* 配置 */
    //     rtc_configuration();
    //     /* 首次调整时间  传入一个时间戳*/
    //     time_adjust(1743668026);
    //     bkp_write_data(BKP_DATA_0, 0xA5A5);
    // }

}


void Rtc_TimerInit(void)
{
    // 使能定时器时钟
    rcu_periph_clock_enable(RCU_TIMER2);
    timer_deinit(TIMER2); // 复位定时器

    // 配置定时器参数
    timer_parameter_struct timer_initpara;
    timer_initpara.prescaler = 12000-1;        // 预分频值
    timer_initpara.alignedmode = TIMER_COUNTER_EDGE; // 边沿对齐模式
    timer_initpara.counterdirection = TIMER_COUNTER_UP; // 向上计数
    timer_initpara.period = 10000-1;              // 自动重装载值
    timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
    timer_init(TIMER2, &timer_initpara);

    // 立即加载预分频器值
    timer_prescaler_config(TIMER2,12000-1,TIMER_PSC_RELOAD_NOW);

    // 清除中断标志（防止首次误触发）
    timer_interrupt_flag_clear(TIMER2, TIMER_INT_FLAG_UP);

    // 使能更新中断
    timer_interrupt_enable(TIMER2, TIMER_INT_UP);

    // 配置NVIC中断优先级
    nvic_irq_enable(TIMER2_IRQn, 0, 1); // 优先级组1，子优先级1

    // 启动定时器
    timer_enable(TIMER2);
		
}


void TIMER2_IRQHandler(void)
{
     if (timer_interrupt_flag_get(TIMER2, TIMER_INT_FLAG_UP) == SET){
		timer_interrupt_flag_clear(TIMER2, TIMER_INT_FLAG_UP);
        rtc_time++;
    }
}