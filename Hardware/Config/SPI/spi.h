#ifndef SPI_H
#define SPI_H

#ifdef cplusplus
 extern "C" {
#endif

#include "gd32f30x.h"
typedef enum
{
    SPI_FLASH, //外置flash
    SPI_NFC, //刷卡模块
}SPI_TYPE;

#define FLASH_SPI           SPI0
#define NFC_SPI             SPI2

#define SPI0_CS_PORT        GPIOA
#define SPI0_CS_PIN         GPIO_PIN_4
#define SPI0_CLK_PORT       GPIOA
#define SPI0_CLK_PIN        GPIO_PIN_5
#define SPI0_MISO_PORT      GPIOA
#define SPI0_MISO_PIN       GPIO_PIN_6
#define SPI0_MOSI_PORT      GPIOA
#define SPI0_MOSI_PIN       GPIO_PIN_7
#define W25Q64_WP_PORT      GPIOC
#define W25Q64_WP_PIN       GPIO_PIN_4

#define W25Q64_WP_EN            gpio_bit_set(W25Q64_WP_PORT, W25Q64_WP_PIN)
#define W25Q64_WP_DISEN         gpio_bit_reset(W25Q64_WP_PORT, W25Q64_WP_PIN)
#define SPI0_CS_LOW()           gpio_bit_reset(SPI0_CS_PORT, SPI0_CS_PIN)
#define SPI0_CS_HIGH()          gpio_bit_set(SPI0_CS_PORT, SPI0_CS_PIN)

#define SPI2_CS_PORT        GPIOB
#define SPI2_CS_PIN         GPIO_PIN_6
#define SPI2_CLK_PORT       GPIOB
#define SPI2_CLK_PIN        GPIO_PIN_3
#define SPI2_MISO_PORT      GPIOB
#define SPI2_MISO_PIN       GPIO_PIN_4
#define SPI2_MOSI_PORT      GPIOB
#define SPI2_MOSI_PIN       GPIO_PIN_5

#define NFC_NPD_PORT       GPIOD
#define NFC_NPD_PIN        GPIO_PIN_7

#define NPD_LOW()               gpio_bit_reset(NFC_NPD_PORT, NFC_NPD_PIN)
#define NPD_HIGH()              gpio_bit_set(NFC_NPD_PORT, NFC_NPD_PIN)
#define SPI2_CS_LOW()           gpio_bit_reset(SPI2_CS_PORT, SPI2_CS_PIN)
#define SPI2_CS_HIGH()          gpio_bit_set(SPI2_CS_PORT, SPI2_CS_PIN)

uint8_t SpiReadWrite(uint32_t SPIx,uint8_t InData);
void YA_SpiWriteData(uint32_t SPIx,uint8_t byteCount,uint8_t* pData);
void YA_SpiRead_NbyteData(uint32_t spi_x, uint8_t *data, uint32_t len);
uint8_t YA_SpiReadData(uint32_t SPIx);
void YA_SpiInit(SPI_TYPE type);

#ifdef cplusplus
}
#endif

#endif /* SPI_H */
