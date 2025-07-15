#include "gpio.h"
#include "timer.h"
#include "led.h"
#include "global.h"
#include "relay_control.h"
void RCU_Init(void)
{
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);	
	rcu_periph_clock_enable(RCU_GPIOC);	
	rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_GPIOE);
    rcu_periph_clock_enable(RCU_AF);
}


//led使能
void YA_Led_Init(void)
{

    rcu_periph_clock_enable(RCU_AF);
    gpio_pin_remap_config(GPIO_PD01_REMAP, ENABLE);
    gpio_init(LED_EN_QR_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, LED_EN_QR_PIN);
    GPIO_BOP(LED_EN_QR_PORT) = LED_EN_QR_PIN;
    EN_QR_OFF;
    ledstatus = LED_FLASH;
}

void YA_Disboard_Init(void)
{
    gpio_init(DIS_EN_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, DIS_EN_PIN);
    DIS_ON;
}

//过流检测引脚使能
void YA_Current_Over_Init(void)
{	
	gpio_init(CURRENT_OVER1_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CURRENT_OVER1_PIN);
	gpio_init(CURRENT_OVER2_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CURRENT_OVER2_PIN);
	gpio_init(CURRENT_OVER3_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CURRENT_OVER3_PIN);
	gpio_init(CURRENT_OVER4_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CURRENT_OVER4_PIN);
    gpio_init(CURRENT_OVER5_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CURRENT_OVER5_PIN);
    gpio_init(CURRENT_OVER6_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CURRENT_OVER6_PIN);
    gpio_init(CURRENT_OVER7_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CURRENT_OVER7_PIN);
    gpio_init(CURRENT_OVER8_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CURRENT_OVER8_PIN);
    gpio_init(CURRENT_OVER9_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CURRENT_OVER9_PIN);
    gpio_init(CURRENT_OVER10_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CURRENT_OVER10_PIN);
    gpio_init(CURRENT_OVER11_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CURRENT_OVER11_PIN);
    gpio_init(CURRENT_OVER12_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CURRENT_OVER12_PIN);

    gpio_exti_source_select(CURRENT_OVER1_PORT_EXTI_SOURCE, CURRENT_OVER1_PIN_EXTI_SOURCE);
    gpio_exti_source_select(CURRENT_OVER2_PORT_EXTI_SOURCE, CURRENT_OVER2_PIN_EXTI_SOURCE);
    gpio_exti_source_select(CURRENT_OVER3_PORT_EXTI_SOURCE, CURRENT_OVER3_PIN_EXTI_SOURCE);
    gpio_exti_source_select(CURRENT_OVER4_PORT_EXTI_SOURCE, CURRENT_OVER4_PIN_EXTI_SOURCE);
    gpio_exti_source_select(CURRENT_OVER5_PORT_EXTI_SOURCE, CURRENT_OVER5_PIN_EXTI_SOURCE);
    gpio_exti_source_select(CURRENT_OVER6_PORT_EXTI_SOURCE, CURRENT_OVER6_PIN_EXTI_SOURCE);
    gpio_exti_source_select(CURRENT_OVER7_PORT_EXTI_SOURCE, CURRENT_OVER7_PIN_EXTI_SOURCE);
    gpio_exti_source_select(CURRENT_OVER8_PORT_EXTI_SOURCE, CURRENT_OVER8_PIN_EXTI_SOURCE);
    gpio_exti_source_select(CURRENT_OVER9_PORT_EXTI_SOURCE, CURRENT_OVER9_PIN_EXTI_SOURCE);
    gpio_exti_source_select(CURRENT_OVER10_PORT_EXTI_SOURCE, CURRENT_OVER10_PIN_EXTI_SOURCE);
    gpio_exti_source_select(CURRENT_OVER11_PORT_EXTI_SOURCE, CURRENT_OVER11_PIN_EXTI_SOURCE);
    gpio_exti_source_select(CURRENT_OVER12_PORT_EXTI_SOURCE, CURRENT_OVER12_PIN_EXTI_SOURCE);

    exti_init(CURRENT_OVER1_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_init(CURRENT_OVER2_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_init(CURRENT_OVER3_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_init(CURRENT_OVER4_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_init(CURRENT_OVER5_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_init(CURRENT_OVER6_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_init(CURRENT_OVER7_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_init(CURRENT_OVER8_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_init(CURRENT_OVER9_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_init(CURRENT_OVER10_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_init(CURRENT_OVER11_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_init(CURRENT_OVER12_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);

    exti_interrupt_flag_clear(CURRENT_OVER1_EXTI_LINE);
    exti_interrupt_flag_clear(CURRENT_OVER2_EXTI_LINE);
    exti_interrupt_flag_clear(CURRENT_OVER3_EXTI_LINE);
    exti_interrupt_flag_clear(CURRENT_OVER4_EXTI_LINE);
    exti_interrupt_flag_clear(CURRENT_OVER5_EXTI_LINE);
    exti_interrupt_flag_clear(CURRENT_OVER6_EXTI_LINE);
    exti_interrupt_flag_clear(CURRENT_OVER7_EXTI_LINE);
    exti_interrupt_flag_clear(CURRENT_OVER8_EXTI_LINE);
    exti_interrupt_flag_clear(CURRENT_OVER9_EXTI_LINE);
    exti_interrupt_flag_clear(CURRENT_OVER10_EXTI_LINE);
    exti_interrupt_flag_clear(CURRENT_OVER11_EXTI_LINE);
    exti_interrupt_flag_clear(CURRENT_OVER12_EXTI_LINE);

    nvic_irq_enable(CURRENT_OVER1_EXTI_IRQn, 2U, 0U);
    nvic_irq_enable(CURRENT_OVER2_EXTI_IRQn, 2U, 0U);
    nvic_irq_enable(CURRENT_OVER3_EXTI_IRQn, 2U, 0U);
    nvic_irq_enable(CURRENT_OVER4_EXTI_IRQn, 2U, 0U);
    nvic_irq_enable(CURRENT_OVER5_EXTI_IRQn, 2U, 0U);
    nvic_irq_enable(CURRENT_OVER6_EXTI_IRQn, 2U, 0U);
    nvic_irq_enable(CURRENT_OVER7_EXTI_IRQn, 2U, 0U);
    nvic_irq_enable(CURRENT_OVER8_EXTI_IRQn, 2U, 0U);
    nvic_irq_enable(CURRENT_OVER9_EXTI_IRQn, 2U, 0U);
    nvic_irq_enable(CURRENT_OVER10_EXTI_IRQn, 2U, 0U);
    nvic_irq_enable(CURRENT_OVER11_EXTI_IRQn, 2U, 0U);
    nvic_irq_enable(CURRENT_OVER12_EXTI_IRQn, 2U, 0U);
	
}



//继电器引脚初始化
void YA_Relay_Init(void)
{
	rcu_periph_clock_enable(RCU_AF);
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);//PB4¹Ü½ÅÖØÓ³Éä
    gpio_init(RELAY_1_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RELAY_1_PIN);
    gpio_bit_reset(RELAY_1_PORT,RELAY_1_PIN);
    gpio_init(RELAY_2_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RELAY_2_PIN);
    gpio_bit_reset(RELAY_2_PORT,RELAY_2_PIN);
    gpio_init(RELAY_3_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RELAY_3_PIN);
    gpio_bit_reset(RELAY_3_PORT,RELAY_3_PIN);
    gpio_init(RELAY_4_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RELAY_4_PIN);
    gpio_bit_reset(RELAY_4_PORT,RELAY_4_PIN);
    gpio_init(RELAY_5_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RELAY_5_PIN);
    gpio_bit_reset(RELAY_6_PORT,RELAY_5_PIN);
    gpio_init(RELAY_6_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RELAY_6_PIN);
    gpio_bit_reset(RELAY_6_PORT,RELAY_6_PIN);
    gpio_init(RELAY_7_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RELAY_7_PIN);
    gpio_bit_reset(RELAY_7_PORT,RELAY_7_PIN);
    gpio_init(RELAY_8_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RELAY_8_PIN);
    gpio_bit_reset(RELAY_8_PORT,RELAY_8_PIN);
    gpio_init(RELAY_9_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RELAY_9_PIN);
    gpio_bit_reset(RELAY_9_PORT,RELAY_9_PIN);
    gpio_init(RELAY_10_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RELAY_10_PIN);
    gpio_bit_reset(RELAY_10_PORT,RELAY_10_PIN);
    gpio_init(RELAY_11_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RELAY_11_PIN);
    gpio_bit_reset(RELAY_11_PORT,RELAY_11_PIN);
    gpio_init(RELAY_12_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RELAY_12_PIN);
    gpio_bit_reset(RELAY_12_PORT,RELAY_12_PIN);
}

//过零信号检测
void YA_Zero_Det_Init(void)
{
	gpio_init(ZERO_DET_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, ZERO_DET_PIN);
	
	//配置外部中断
	nvic_irq_enable(ZERO_DET_EXTI_IRQn, 2U, 1U);
	gpio_exti_source_select(ZERO_DET_PORT_EXTI_SOURCE, ZERO_DET_PIN_EXTI_SOURCE);
	exti_init(ZERO_DET_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    exti_interrupt_flag_clear(ZERO_DET_EXTI_LINE);
	exti_interrupt_enable(ZERO_DET_EXTI_LINE);
}


//4G电源引脚初始化
void YA_4G_Pwr_Init(void)
{
	// rcu_periph_clock_enable(RCU_AF);
	// gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);//PB3
    gpio_init(PWR_4G_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, PWR_4G_PIN);
    gpio_bit_reset(PWR_4G_PORT,PWR_4G_PIN);
}

//蜂鸣器引脚初始化
void YA_BUZE_Init(void)
{
    gpio_init(BUZE_1_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, BUZI_1_PIN|BUZI_2_PIN);
    BUZI_1_OFF;
    BUZI_2_OFF;
}

//插入检测使能引脚使能
void YA_Insert_Det_Init(void)
{
    gpio_init(INSERT_DET_EN_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, INSERT_DET_EN_PIN);
    INSERT_DET_ON;
}

//保险丝检测引脚使能
void YA_Fuse_Init(void)
{
    for (uint8_t i = 0; i < PORTMAX; i++)
    {
        gpio_init(Fuse_Array[i].port, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, Fuse_Array[i].pin);
    }
}

void YA_GpioInit(void)
{
	RCU_Init();
    YA_4G_Pwr_Init();
	// YA_Led_Init();
    //YA_Current_Over_Init();
    YA_Relay_Init();
    YA_Insert_Det_Init();
    YA_Zero_Det_Init();
    YA_BUZE_Init();
    YA_Fuse_Init();
    YA_Disboard_Init();
}





















