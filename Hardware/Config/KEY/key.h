#ifndef KEY_H
#define KEY_H

#ifdef cplusplus
 extern "C" {
#endif

#include "gd32f30x.h"

typedef enum 
{
    KEY_SET = 0,
//    KEY_TAMPER = 1,
//    KEY_USER = 2
} key_typedef_enum;

typedef enum 
{
    KEY_MODE_GPIO = 0,
    KEY_MODE_EXTI = 1
} keymode_typedef_enum;

#define KEYn                         1U

#define YA_KEY_PIN                   GPIO_PIN_15
#define YA_KEY_GPIO_PORT             GPIOA
#define YA_KEY_GPIO_CLK              RCU_GPIOA
#define YA_KEY_EXTI_LINE             EXTI_15
#define YA_KEY_EXTI_PORT_SOURCE      GPIO_PORT_SOURCE_GPIOA
#define YA_KEY_EXTI_PIN_SOURCE       GPIO_PIN_SOURCE_15
#define YA_KEY_EXTI_IRQn             EXTI10_15_IRQn

/* configure key */
void YA_KeyInit(key_typedef_enum key_num, keymode_typedef_enum key_mode);
/* return the selected key state */
uint8_t YA_KeyStateGet(key_typedef_enum key);

#ifdef cplusplus
}
#endif

#endif /* KEY_H */

