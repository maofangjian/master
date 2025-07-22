/*** 
 * @Author: chenyuan
 * @Date: 2025-04-21 13:58:15
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-05-15 18:06:27
 * @FilePath: \13.FreeRTOS实验\Hardware\Config\I2S\i2s.h
 * @Description: 
 * @
 * @Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#ifndef _I2S_H_
#define _I2S_H_
#include "gd32f30x.h"
//I2S IO口
#define I2S_EN_PORT    GPIOB
#define I2S_EN_PIN     GPIO_PIN_14
#define I2S_CLK_PORT   GPIOB
#define I2S_CLK_PIN    GPIO_PIN_13
#define I2S_WS_PORT    GPIOB
#define I2S_WS_PIN     GPIO_PIN_12
#define I2S_SD_PORT    GPIOB
#define I2S_SD_PIN     GPIO_PIN_15
#define I2S_PORT       GPIOB

#define I2S_SPI        SPI1

#define PA_ENABLE      gpio_bit_set(I2S_EN_PORT,I2S_EN_PIN)
#define PA_DISABLE     gpio_bit_reset(I2S_EN_PORT,I2S_EN_PIN)

#define AUDIO_START_FLASH_BASE        0X00000028
#define AUDIO_END_FLASH_BASE          0X0000002C
typedef enum
{
    ITEM_NULL = 0,
    AUDIO_00,   //欢迎使用充电桩
    AUDIO_01,   //0
    AUDIO_02,   //1
    AUDIO_03,   //2
    AUDIO_04,   //3
    AUDIO_05,   //4
    AUDIO_06,   //5
    AUDIO_07,   //6
    AUDIO_08,   //7
    AUDIO_09,   //8
    AUDIO_10,   //9
    AUDIO_11,   //十
    AUDIO_12,   //百
    AUDIO_13,   //点
    AUDIO_14,   //元
    AUDIO_15,   //卡余额
    AUDIO_16,   //无效卡
    AUDIO_17,   //号口开始充电
    AUDIO_18,   //号口结束充电
    AUDIO_19,   //选择插座充电
    ITME_MAX,
}AUDIO_ITEM;

typedef struct 
{
    uint32_t flash_addr;  //在flash地址
    uint32_t flash_byte_num;   //音频占用的字节数
}AUDIO_INFO;
extern uint8_t play_item[];
extern volatile uint8_t dma_complete_flag;
void i2s_dma_init(void);
void App_audio_task(void *pvParameters);
#endif
