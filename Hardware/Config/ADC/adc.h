#ifndef _ADC_H_
#define _ADC_H_
#include "gd32f30x.h"

typedef enum
{
    ADC_NTC_1 = 0,  //ADC温度检测点1
    ADC_NTC_2,  //ADC温度检测点2
    ADC_NTC_3,  //ADC温度检测点3
}NTC_ADC_DEF;

void YA_Adc_Init(void);
extern void App_TempTask(void);
#endif

