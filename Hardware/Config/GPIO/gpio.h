#ifndef GPIO_H
#define GPIO_H

#ifdef cplusplus
 extern "C" {
#endif

#include "gd32f30x.h"


#define CURRENT_OVER1_PIN               GPIO_PIN_8    //通道1过流检测引脚
#define CURRENT_OVER1_PORT              GPIOB
#define CURRENT_OVER1_EXTI_LINE         EXTI_8
#define CURRENT_OVER1_PORT_EXTI_SOURCE  GPIO_PORT_SOURCE_GPIOB
#define CURRENT_OVER1_PIN_EXTI_SOURCE   GPIO_PIN_SOURCE_8
#define CURRENT_OVER1_EXTI_IRQn         EXTI5_9_IRQn

#define CURRENT_OVER2_PIN               GPIO_PIN_9    //通道2过流检测引脚
#define CURRENT_OVER2_PORT              GPIOB
#define CURRENT_OVER2_EXTI_LINE         EXTI_9
#define CURRENT_OVER2_PORT_EXTI_SOURCE  GPIO_PORT_SOURCE_GPIOB
#define CURRENT_OVER2_PIN_EXTI_SOURCE   GPIO_PIN_SOURCE_9
#define CURRENT_OVER2_EXTI_IRQn         EXTI5_9_IRQn

#define CURRENT_OVER3_PIN               GPIO_PIN_0    //通道3过流检测引脚
#define CURRENT_OVER3_PORT              GPIOE
#define CURRENT_OVER3_EXTI_LINE         EXTI_0
#define CURRENT_OVER3_PORT_EXTI_SOURCE  GPIO_PORT_SOURCE_GPIOE
#define CURRENT_OVER3_PIN_EXTI_SOURCE   GPIO_PIN_SOURCE_0
#define CURRENT_OVER3_EXTI_IRQn         EXTI0_IRQn 

#define CURRENT_OVER4_PIN               GPIO_PIN_1    //通道4过流检测引脚
#define CURRENT_OVER4_PORT              GPIOE
#define CURRENT_OVER4_EXTI_LINE         EXTI_1
#define CURRENT_OVER4_PORT_EXTI_SOURCE  GPIO_PORT_SOURCE_GPIOE
#define CURRENT_OVER4_PIN_EXTI_SOURCE   GPIO_PIN_SOURCE_1
#define CURRENT_OVER4_EXTI_IRQn         EXTI1_IRQn

#define CURRENT_OVER5_PIN               GPIO_PIN_2    //通道5过流检测引脚
#define CURRENT_OVER5_PORT              GPIOE
#define CURRENT_OVER5_EXTI_LINE         EXTI_2
#define CURRENT_OVER5_PORT_EXTI_SOURCE  GPIO_PORT_SOURCE_GPIOE
#define CURRENT_OVER5_PIN_EXTI_SOURCE   GPIO_PIN_SOURCE_2
#define CURRENT_OVER5_EXTI_IRQn         EXTI2_IRQn

#define CURRENT_OVER6_PIN               GPIO_PIN_3    //通道6过流检测引脚
#define CURRENT_OVER6_PORT              GPIOE
#define CURRENT_OVER6_EXTI_LINE         EXTI_3
#define CURRENT_OVER6_PORT_EXTI_SOURCE  GPIO_PORT_SOURCE_GPIOE
#define CURRENT_OVER6_PIN_EXTI_SOURCE   GPIO_PIN_SOURCE_3
#define CURRENT_OVER6_EXTI_IRQn         EXTI3_IRQn

#define CURRENT_OVER7_PIN               GPIO_PIN_5    //通道7过流检测引脚
#define CURRENT_OVER7_PORT              GPIOE
#define CURRENT_OVER7_EXTI_LINE         EXTI_5
#define CURRENT_OVER7_PORT_EXTI_SOURCE  GPIO_PORT_SOURCE_GPIOE
#define CURRENT_OVER7_PIN_EXTI_SOURCE   GPIO_PIN_SOURCE_5
#define CURRENT_OVER7_EXTI_IRQn         EXTI5_9_IRQn

#define CURRENT_OVER8_PIN               GPIO_PIN_4    //通道8过流检测引脚
#define CURRENT_OVER8_PORT              GPIOE
#define CURRENT_OVER8_EXTI_LINE         EXTI_4
#define CURRENT_OVER8_PORT_EXTI_SOURCE  GPIO_PORT_SOURCE_GPIOE
#define CURRENT_OVER8_PIN_EXTI_SOURCE   GPIO_PIN_SOURCE_4
#define CURRENT_OVER8_EXTI_IRQn         EXTI4_IRQn


#define CURRENT_OVER9_PIN               GPIO_PIN_13    //通道9过流检测引脚
#define CURRENT_OVER9_PORT              GPIOC
#define CURRENT_OVER9_EXTI_LINE         EXTI_13
#define CURRENT_OVER9_PORT_EXTI_SOURCE  GPIO_PORT_SOURCE_GPIOC
#define CURRENT_OVER9_PIN_EXTI_SOURCE   GPIO_PIN_SOURCE_13
#define CURRENT_OVER9_EXTI_IRQn         EXTI10_15_IRQn

#define CURRENT_OVER10_PIN               GPIO_PIN_6    //通道10过流检测引脚
#define CURRENT_OVER10_PORT              GPIOE
#define CURRENT_OVER10_EXTI_LINE         EXTI_6
#define CURRENT_OVER10_PORT_EXTI_SOURCE  GPIO_PORT_SOURCE_GPIOE
#define CURRENT_OVER10_PIN_EXTI_SOURCE   GPIO_PIN_SOURCE_6
#define CURRENT_OVER10_EXTI_IRQn         EXTI5_9_IRQn

#define CURRENT_OVER11_PIN               GPIO_PIN_15    //通道11过流检测引脚
#define CURRENT_OVER11_PORT              GPIOC
#define CURRENT_OVER11_EXTI_LINE         EXTI_15
#define CURRENT_OVER11_PORT_EXTI_SOURCE  GPIO_PORT_SOURCE_GPIOC
#define CURRENT_OVER11_PIN_EXTI_SOURCE   GPIO_PIN_SOURCE_15
#define CURRENT_OVER11_EXTI_IRQn         EXTI10_15_IRQn

#define CURRENT_OVER12_PIN               GPIO_PIN_14    //通道12过流检测引脚
#define CURRENT_OVER12_PORT              GPIOC
#define CURRENT_OVER12_EXTI_LINE         EXTI_14
#define CURRENT_OVER12_PORT_EXTI_SOURCE  GPIO_PORT_SOURCE_GPIOC
#define CURRENT_OVER12_PIN_EXTI_SOURCE   GPIO_PIN_SOURCE_14
#define CURRENT_OVER12_EXTI_IRQn         EXTI10_15_IRQn

#define ZERO_DET_PIN                    GPIO_PIN_7    //过零检测引脚
#define ZERO_DET_PORT                   GPIOB
#define ZERO_DET_EXTI_LINE              EXTI_7
#define ZERO_DET_PORT_EXTI_SOURCE       GPIO_PORT_SOURCE_GPIOB
#define ZERO_DET_PIN_EXTI_SOURCE        GPIO_PIN_SOURCE_7
#define ZERO_DET_EXTI_IRQn              EXTI5_9_IRQn

#define DIS_EN_PIN         GPIO_PIN_4
#define DIS_EN_PORT        GPIOD
#define DIS_ON   gpio_bit_set(DIS_EN_PORT,DIS_EN_PIN)
#define DIS_OFF  gpio_bit_reset(DIS_EN_PORT,DIS_EN_PIN)

#define INSERT_DET_EN_PIN  GPIO_PIN_6    //插入检测使能引脚
#define INSERT_DET_EN_PORT GPIOA    //插入检测使能引脚

#define INSERT_DET_ON   gpio_bit_set(INSERT_DET_EN_PORT,INSERT_DET_EN_PIN)
#define INSERT_DET_OFF  gpio_bit_reset(INSERT_DET_EN_PORT,INSERT_DET_EN_PIN)

#define LED_EN_QR_PIN   GPIO_PIN_1  //二维码指示灯
#define LED_EN_QR_PORT  GPIOD

#define RELAY_1_PIN     GPIO_PIN_5   //继电器1
#define RELAY_1_PORT    GPIOC
#define RELAY_2_PIN     GPIO_PIN_0  //继电器2
#define RELAY_2_PORT    GPIOB
#define RELAY_3_PIN     GPIO_PIN_1  //继电器3
#define RELAY_3_PORT    GPIOB
#define RELAY_4_PIN     GPIO_PIN_2 //继电器4
#define RELAY_4_PORT    GPIOB
#define RELAY_5_PIN     GPIO_PIN_7 //继电器5
#define RELAY_5_PORT    GPIOE
#define RELAY_6_PIN     GPIO_PIN_8 //继电器6
#define RELAY_6_PORT    GPIOE
#define RELAY_7_PIN     GPIO_PIN_9 //继电器7
#define RELAY_7_PORT    GPIOE
#define RELAY_8_PIN     GPIO_PIN_10 //继电器8
#define RELAY_8_PORT    GPIOE
#define RELAY_9_PIN     GPIO_PIN_11 //继电器9
#define RELAY_9_PORT    GPIOE
#define RELAY_10_PIN    GPIO_PIN_12 //继电器10
#define RELAY_10_PORT   GPIOE
#define RELAY_11_PIN    GPIO_PIN_13 //继电器11
#define RELAY_11_PORT   GPIOE
#define RELAY_12_PIN    GPIO_PIN_14 //继电器12
#define RELAY_12_PORT   GPIOE


#define PWR_4G_PIN      GPIO_PIN_11
#define PWR_4G_PORT     GPIOA

#define PWR_4G_ON       gpio_bit_set(PWR_4G_PORT,PWR_4G_PIN)
#define PWR_4G_OFF      gpio_bit_reset(PWR_4G_PORT,PWR_4G_PIN)

#define BUZE_1_PORT     GPIOA
#define BUZI_1_PIN      GPIO_PIN_1
#define BUZE_2_PORT     GPIOA
#define BUZI_2_PIN      GPIO_PIN_2
#define BUZI_1_ON       gpio_bit_set(BUZE_1_PORT,BUZI_1_PIN)
#define BUZI_1_OFF      gpio_bit_reset(BUZE_1_PORT,BUZI_1_PIN)
#define BUZI_2_ON       gpio_bit_set(BUZE_2_PORT,BUZI_2_PIN)
#define BUZI_2_OFF      gpio_bit_reset(BUZE_2_PORT,BUZI_2_PIN)

#define EN_QR_ON        gpio_bit_set(LED_EN_QR_PORT,LED_EN_QR_PIN)
#define EN_QR_OFF       gpio_bit_reset(LED_EN_QR_PORT,LED_EN_QR_PIN)


#define FUSE_1_PORT     GPIOC
#define FUSE_1_PIN      GPIO_PIN_7
#define FUSE_2_PORT     GPIOC
#define FUSE_2_PIN      GPIO_PIN_6
#define FUSE_3_PORT     GPIOD
#define FUSE_3_PIN      GPIO_PIN_15
#define FUSE_4_PORT     GPIOD
#define FUSE_4_PIN      GPIO_PIN_14
#define FUSE_5_PORT     GPIOD
#define FUSE_5_PIN      GPIO_PIN_13
#define FUSE_6_PORT     GPIOD
#define FUSE_6_PIN      GPIO_PIN_12
#define FUSE_7_PORT     GPIOD
#define FUSE_7_PIN      GPIO_PIN_11
#define FUSE_8_PORT     GPIOD
#define FUSE_8_PIN      GPIO_PIN_10
#define FUSE_9_PORT     GPIOD
#define FUSE_9_PIN      GPIO_PIN_9
#define FUSE_10_PORT    GPIOB
#define FUSE_10_PIN     GPIO_PIN_11
#define FUSE_11_PORT    GPIOB
#define FUSE_11_PIN     GPIO_PIN_10
#define FUSE_12_PORT    GPIOE
#define FUSE_12_PIN     GPIO_PIN_15
/* configure GPIO */
void YA_GpioInit(void);



#ifdef cplusplus
}
#endif

#endif /* GPIO_H */
