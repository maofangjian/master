#ifndef _FLASH_H_
#define _FLASH_H_
#include "gd32f30x.h"

#define FLASH_SIZE                  (512*1024)  //GD32F303 flash大小
#define FLASH_SECTOR_SIZE           (2*1024)  //一个扇区大小2k
#define FLASH_BUF_SIZE              (FLASH_SECTOR_SIZE)
#define APP_MAIN_ADDR               FLASH_BASE
#define APP_MAIN_SIZE               (196*1024)

#define APP_BACK_ADDR               (APP_MAIN_ADDR+APP_MAIN_SIZE)
#define APP_BACK_SIZE               APP_MAIN_SIZE


#define SYS_UPGRADE_ADDR            (APP_BACK_ADDR+APP_BACK_SIZE)
#define SYS_UPGRADE_SIZE            (4096)


uint8_t App_Read_Flash(uint8_t *read_data,uint32_t address, uint32_t size);
uint8_t App_Write_Flash(uint8_t *write_data,uint32_t address, uint32_t size);
#endif

