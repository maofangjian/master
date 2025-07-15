#include "key.h"

static uint32_t KEY_PORT[KEYn] = {YA_KEY_GPIO_PORT};
static uint32_t KEY_PIN[KEYn] = {YA_KEY_PIN};
static rcu_periph_enum KEY_CLK[KEYn] = {YA_KEY_GPIO_CLK};
static exti_line_enum KEY_EXTI_LINE[KEYn] = {YA_KEY_EXTI_LINE};
static uint8_t KEY_PORT_SOURCE[KEYn] = {YA_KEY_EXTI_PORT_SOURCE};
static uint8_t KEY_PIN_SOURCE[KEYn] = {YA_KEY_EXTI_PIN_SOURCE};
static uint8_t KEY_IRQn[KEYn] = {YA_KEY_EXTI_IRQn};

/* configure key */
void YA_KeyInit(key_typedef_enum key_num, keymode_typedef_enum key_mode)
{
    /* enable the key clock */
    rcu_periph_clock_enable(KEY_CLK[key_num]);
    rcu_periph_clock_enable(RCU_AF);

    /* configure button pin as input */
    gpio_init(KEY_PORT[key_num], GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, KEY_PIN[key_num]);

    if (key_mode == KEY_MODE_EXTI) {
        /* enable and set key EXTI interrupt to the lowest priority */
        nvic_irq_enable(KEY_IRQn[key_num], 0U, 4U);

        /* connect key EXTI line to key GPIO pin */
        gpio_exti_source_select(KEY_PORT_SOURCE[key_num], KEY_PIN_SOURCE[key_num]);

        /* configure key EXTI line */
        exti_init(KEY_EXTI_LINE[key_num], EXTI_INTERRUPT, EXTI_TRIG_FALLING);
        exti_interrupt_flag_clear(KEY_EXTI_LINE[key_num]);
    }

}


/* return the selected key state */
uint8_t YA_KeyStateGet(key_typedef_enum key)
{
	return gpio_input_bit_get(KEY_PORT[key], KEY_PIN[key]);
}

