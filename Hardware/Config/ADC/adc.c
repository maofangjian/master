#include "adc.h"
#include "systick.h"
#include "rtc.h"
#include "meter.h"
#include "start.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <math.h>
uint16_t Read_Adc_Value(NTC_ADC_DEF type)
{
	uint16_t data = 0;

  	//设置指定ADC的规则组通道，一个序列，采样时间
    adc_software_trigger_enable(ADC0, ADC_INSERTED_CHANNEL);
	while((adc_flag_get(ADC0,ADC_FLAG_EOIC) == SET) && (adc_flag_get(ADC0,ADC_FLAG_EOC) == SET))
    {
		switch(type) {
			case ADC_NTC_1:
				 data = adc_inserted_data_read(ADC0,ADC_INSERTED_CHANNEL_0);//返回最近一次ADC0规则组的转换结果
				break;
			case ADC_NTC_2:
				data = adc_inserted_data_read(ADC0,ADC_INSERTED_CHANNEL_1);	//返回最近一次ADC0规则组的转换结果
				break;
			case ADC_NTC_3:
				data = adc_inserted_data_read(ADC0,ADC_INSERTED_CHANNEL_2);	//返回最近一次ADC0规则组的转换结果
				break;
			default:
				break;
		}
       
		adc_flag_clear(ADC0,ADC_FLAG_EOIC);
        adc_flag_clear(ADC0,ADC_FLAG_EOC);
    }
    return data;  
}

float Temp_Conversion(uint32_t value)
{
    float T2 = 298.15;  //T2
    float Ka = 273.15;
    float Rp = 10000.0;
    float Bx = 3950.0;
    float rt = 0;
    float temp;
    rt = (float)((20000*value)/(4096-value));
    temp = log(rt/Rp);//ln(Rt/Rp)
    temp/=Bx;//ln(Rt/Rp)/B
    temp+=(1/T2);
    temp = 1/(temp);
    temp-=Ka;
    return temp;
}


uint8_t YA_Get_Temp_Value(NTC_ADC_DEF type)
{
    uint32_t adc_average = 0;
    uint32_t adc_total = 0;
    float ntc_temp = 0;
    for(uint8_t i = 0; i < 20; i++)
    {
        adc_total += Read_Adc_Value(type);   
    }
    adc_average = adc_total/20;
    ntc_temp = Temp_Conversion(adc_average)/4;
    return (uint8_t)ntc_temp;
}

//温度传感器ADC初始化
void YA_Adc_Init(void)
{
    rcu_periph_clock_enable(RCU_ADC0);
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV12); 
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2);//温度检测点1 2 3 
    adc_channel_length_config(ADC0, ADC_INSERTED_CHANNEL, 3);	//顺序进行规则转换的数目
    adc_inserted_channel_config(ADC0, 0, ADC_CHANNEL_10, ADC_SAMPLETIME_239POINT5);//指定规则通道组0为通道14的转化通道
    adc_inserted_channel_config(ADC0, 1, ADC_CHANNEL_11, ADC_SAMPLETIME_239POINT5);//指定规则通道组1为通道15的转化通道
	adc_inserted_channel_config(ADC0, 2, ADC_CHANNEL_12, ADC_SAMPLETIME_239POINT5);//指定规则通道组2为通道8的转化通道

	/* ADC trigger config */
    adc_external_trigger_source_config(ADC0, ADC_INSERTED_CHANNEL, ADC0_1_2_EXTTRIG_INSERTED_NONE);
    /* ADC mode config */
    adc_mode_config(ADC_MODE_FREE);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    /* ADC SCAN function enable */
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
	
    adc_external_trigger_config(ADC0, ADC_INSERTED_CHANNEL, ENABLE);
    
	adc_flag_clear(ADC0,ADC_FLAG_EOIC);
    adc_flag_clear(ADC0,ADC_FLAG_EOC);
    /* enable ADC interface */
    adc_enable(ADC0);

    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADC0);
}


void App_TempTask(void)
{
    static uint8_t hightempcnt = 0;
    uint32_t time = 0;
    Port_Status_Def portstatus;
    while (1)
    {
        vTaskDelay(100);
        if(App_GetRtcCount() - time != 0)
        {
            time = App_GetRtcCount();
            // SystemInfo.temp[0] = YA_Get_Temp_Value(ADC_NTC_1);
            // Delay_ms(1);
            // SystemInfo.temp[1] = YA_Get_Temp_Value(ADC_NTC_2);
            // Delay_ms(1);
            // SystemInfo.temp[2] = YA_Get_Temp_Value(ADC_NTC_3);
            if(Report_Data[0].Temp_Chanel_A >= 100 || Report_Data[1].Temp_Chanel_A >= 100 || Report_Data[2].Temp_Chanel_A >= 100)
            {
                hightempcnt++;
                if(hightempcnt >= 20)
                {
                    for(uint8_t i = 0; i < PORTMAX; i++)
                    {
                        Bsp_GetPortStatus(i,&portstatus);
                        portstatus.status |= 0x10;
                        Bsp_SetPortStatus(i,&portstatus);
                    }
                }
            }
            else if(Report_Data[0].Temp_Chanel_A <= 80 && Report_Data[1].Temp_Chanel_A <= 80 && Report_Data[2].Temp_Chanel_A <= 80)
            {
                for(uint8_t i = 0; i < PORTMAX; i++)
                {
                    Bsp_GetPortStatus(i,&portstatus);
                    portstatus.status &= ~0x10;
                    Bsp_SetPortStatus(i,&portstatus);
                }
                hightempcnt = 0;
            }
            //LOG("temp :%d %d %d\r\n",SystemInfo.temp[0],SystemInfo.temp[1],SystemInfo.temp[2]);
        }
        FEED_WDG();
    }
    
}

