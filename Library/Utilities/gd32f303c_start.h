/*!
    \file  gd32f303c_start.h
    \brief definitions for GD32f303C_START's leds, keys and COM ports hardware resources
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-06-30, V1.0.0, firmware for GD32F30x
*/

#ifndef GD32F303C_START_H
#define GD32F303C_START_H

#ifdef cplusplus
 extern "C" {
#endif

#include "gd32f30x.h"
     
/* exported types */
typedef enum 
{
    LED1
} led_typedef_enum;

typedef enum 
{
    KEY_USER
} key_typedef_enum;

typedef enum 
{
    KEY_MODE_GPIO = 0,
    KEY_MODE_EXTI = 1
} keymode_typedef_enum;

/* eval board low layer led */
#define LEDn                             1U

#define LED1_PIN                         GPIO_PIN_6
#define LED1_GPIO_PORT                   GPIOC
#define LED1_GPIO_CLK                    RCU_GPIOC

#define KEYn                             1U

/* user push-button */
#define USER_KEY_PIN                     GPIO_PIN_0
#define USER_KEY_GPIO_PORT               GPIOA
#define USER_KEY_GPIO_CLK                RCU_GPIOA
#define USER_KEY_EXTI_LINE               EXTI_0
#define USER_KEY_EXTI_PORT_SOURCE        GPIO_PORT_SOURCE_GPIOA
#define USER_KEY_EXTI_PIN_SOURCE         GPIO_PIN_SOURCE_0
#define USER_KEY_EXTI_IRQn               EXTI0_IRQn

/* function declarations */
/* configure led GPIO */
void gd_eval_led_init(led_typedef_enum lednum);
/* turn on selected led */
void gd_eval_led_on(led_typedef_enum lednum);
/* turn off selected led */
void gd_eval_led_off(led_typedef_enum lednum);
/* toggle the selected led */
void gd_eval_led_toggle(led_typedef_enum lednum);
/* configure key */
void gd_eval_key_init(key_typedef_enum key_num, keymode_typedef_enum key_mode);
/* return the selected key state */
uint8_t gd_eval_key_state_get(key_typedef_enum key);

#ifdef cplusplus
}
#endif

#endif /* GD32F303C_START_H */
