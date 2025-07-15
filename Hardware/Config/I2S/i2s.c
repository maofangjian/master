
#include "i2s.h"
#include "global.h"
#include "App_GD25xx.h"
// 音频参数定义
#define SAMPLE_RATE     44100     // 44.1kHz采样率
#define AUDIO_BUF_SIZE  1024      // 缓冲区大小

// 全局变量
uint8_t audio_buffer1[AUDIO_BUF_SIZE]; // 音频数据缓冲区
uint8_t audio_buffer2[AUDIO_BUF_SIZE]; // 音频数据缓冲区
volatile uint8_t dma_complete_flag = 0;         // DMA传输完成标志

AUDIO_INFO AudioInfo[ITME_MAX] = {0};
uint8_t play_item[ITME_MAX] = {0};
/* I2S和DMA初始化函数 */
void i2s_dma_init(void)
{
    dma_parameter_struct  dma_init_struct;
    // 1. 启用外设时钟
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_SPI1);  // 假设使用SPI0的I2S功能
    rcu_periph_clock_enable(RCU_DMA0);
    rcu_periph_clock_enable(RCU_AF);

    nvic_irq_enable(DMA0_Channel4_IRQn, 1, 0);
    // 2. 配置I2S引脚 (PA4=WS, PA5=CK, PA7=SD)
    gpio_init(I2S_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, I2S_SD_PIN |I2S_CLK_PIN|I2S_WS_PIN);
    gpio_init(I2S_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, I2S_EN_PIN);	

    // 3. 配置I2S参数
    spi_i2s_deinit(I2S_SPI);
    /* configure I2S0 */
    i2s_init(I2S_SPI, I2S_MODE_MASTERTX, I2S_STD_PHILLIPS, I2S_CKPL_HIGH);

    i2s_psc_config(I2S_SPI, I2S_AUDIOSAMPLE_8K, I2S_FRAMEFORMAT_DT16B_CH16B, I2S_MCKOUT_DISABLE);

    spi_i2s_interrupt_disable(I2S_SPI, SPI_I2S_INT_TBE);
        /* enable SPI */
    i2s_enable(I2S_SPI);

    dma_deinit(DMA0, DMA_CH4);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr = (uint32_t)audio_buffer1; // 初始缓冲区地址
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_init_struct.number = AUDIO_BUF_SIZE/2;
    dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(I2S_SPI);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;  // 最高优先级防数据断流
    
    dma_init(DMA0, DMA_CH4, dma_init_struct);
    dma_circulation_disable(DMA0, DMA_CH4);

    dma_memory_to_memory_disable(DMA0, DMA_CH4);

    dma_interrupt_enable(DMA0, DMA_CH4, DMA_FLAG_FTF);    // 使能传输完成中断

    spi_dma_enable(I2S_SPI, SPI_DMA_TRANSMIT);

    dma_channel_disable(DMA0,DMA_CH4);

}


/**
 * @description: I2S DMA发送函数
 * @Author: chenyuan
 * @Date: 2024-08-09 14:42:46
 * @param {uint8_t} data  要发送的数据
 * @param {uint32_t} len   发送长度
 * @return {*}
 */
void App_I2S_SendData(uint32_t address, uint32_t len)
{
    if(dma_complete_flag == 0)
    {
        dma_complete_flag = 1;
        dma_channel_disable(DMA0, DMA_CH4);
        dma_memory_address_config(DMA0, DMA_CH4, address);
        dma_transfer_number_config(DMA0, DMA_CH4, len);
        dma_channel_enable(DMA0,DMA_CH4);
    }
}


/**
 * @description: 语音播报任务
 * @Author: 
 * @Date: 2025-03-18 17:03:46
 * @param {void} *pvParameters
 * @return {*}
 */
void App_audio_task(void *pvParameters)
{
    uint8_t first_voice = 0;
    uint32_t sector_num = 0;
    uint32_t time_out = 0;
    uint32_t i = 0;
    while (1)
    {
        vTaskDelay(50);
        FEED_WDG();
        for (i = 0; i < ITME_MAX; i++)
        {
            /* code */   
            if(play_item[i] != ITEM_NULL)  //有语音需要播报
            {
                PA_ENABLE;
                first_voice = 0;
                sector_num = 0;
                time_out = 0;
                taskENTER_CRITICAL();
                while(1)
                {
                    if(sector_num < AudioInfo[play_item[i]].flash_byte_num/AUDIO_BUF_SIZE-5)
                    {
                        if(dma_complete_flag == 0)
                        {
                            time_out = 0;
                            if(first_voice == 0)
                            {
                                first_voice = 1;
                                memset(audio_buffer1,0,sizeof(audio_buffer1));
                                BswSrv_GD25xx_BufferRead(audio_buffer1,AudioInfo[play_item[i]].flash_addr+sector_num*AUDIO_BUF_SIZE,AUDIO_BUF_SIZE);
                            }
                            else
                            {
                                memset(audio_buffer1,0,sizeof(audio_buffer1));
                                memcpy(audio_buffer1,audio_buffer2,sizeof(audio_buffer2));
                            }
                            if(sector_num == AudioInfo[play_item[i]].flash_byte_num/AUDIO_BUF_SIZE)
                            {
                                dma_complete_flag = 0;
                                App_I2S_SendData((uint32_t)audio_buffer1,(AudioInfo[play_item[i]].flash_byte_num%AUDIO_BUF_SIZE)/2);
                            }
                            else
                            {
                                dma_complete_flag = 0;
                                App_I2S_SendData((uint32_t)audio_buffer1,AUDIO_BUF_SIZE/2);
                            }
                            sector_num++;
                            memset(audio_buffer2,0,sizeof(audio_buffer2));
                            BswSrv_GD25xx_BufferRead(audio_buffer2,AudioInfo[play_item[i]].flash_addr+sector_num*AUDIO_BUF_SIZE,AUDIO_BUF_SIZE);
                        }
                        else
                        {
                            time_out++;
                            FEED_WDG();
                            if(time_out > 200000)  //超时时间
                            {
                                play_item[i] = ITEM_NULL;
                                dma_complete_flag = 0;
                                PA_DISABLE;
                                taskEXIT_CRITICAL(); 
                                first_voice = 0;
                                break;
                            }
                        }
                        
                    }
                    else
                    {
                        play_item[i] = ITEM_NULL;
                        dma_complete_flag = 0;
                        first_voice = 0;
                        PA_DISABLE;
                        taskEXIT_CRITICAL(); 
                        Delay_ms(200);
                        break;
                    }
                }
            }
            else
            {
                //dma_channel_disable(DMA0,DMA_CH4);
            }
        }
        PA_DISABLE;
        dma_channel_disable(DMA0,DMA_CH4);
        
        
    } 
}


/**
 * @description: 音频数据初始化
 * @Author: 
 * @Date: 2025-03-18 17:03:46
 * @return {*}
 */
void App_Audio_Data_Init(void)
{
    uint8_t audio_start_addr[4] = {0};
    uint8_t audio_end_addr[4] = {0x00};  

    play_item[0] = AUDIO_00;     
    for(uint8_t i = 1; i < ITME_MAX; i++)
    {
        BswSrv_GD25xx_BufferRead(audio_start_addr,AUDIO_START_FLASH_BASE+(i-1)*(0x0f+1),4);
        BswSrv_GD25xx_BufferRead(audio_end_addr,AUDIO_END_FLASH_BASE+(i-1)*(0x0f+1),4);
        AudioInfo[i].flash_addr = audio_start_addr[0]<<24|audio_start_addr[1]<<16|audio_start_addr[2]<<8|audio_start_addr[3];
        AudioInfo[i].flash_byte_num =(audio_end_addr[0]<<24|audio_end_addr[1]<<16|audio_end_addr[2]<<8|audio_end_addr[3]);
        if(AudioInfo[i].flash_byte_num >= 0xffffff)
        {
            AudioInfo[i].flash_byte_num = 0;
        }
        printf("%d %x %x %x\r\n",i,AUDIO_START_FLASH_BASE+(i-1)*(0x0f+1),AudioInfo[i].flash_addr,AudioInfo[i].flash_byte_num);

    }

}