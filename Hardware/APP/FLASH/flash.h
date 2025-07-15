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

#define SYS_PORT_ADDR              (SYS_UPGRADE_ADDR+SYS_UPGRADE_SIZE)
#define SYS_PORT_LEN               (16*1024)
#define SYS_PORT_BACK_ADDR         (SYS_PORT_ADDR+SYS_PORT_LEN)
#define SYS_PORT_BACK_LEN          (SYS_PORT_LEN)
#define SYS_PORT_HEAD_ADDR         (SYS_PORT_BACK_ADDR+SYS_PORT_BACK_LEN)
#define SYS_PORT_HEAD_LEN          (2*1024)

#define SYS_ORDER_HEAD_ADDR        (SYS_PORT_HEAD_ADDR+SYS_PORT_HEAD_LEN)
#define SYS_ORDER_HEAD_LEN         (2*1024)
#define SYS_ORDER_ADDR             (SYS_ORDER_HEAD_ADDR+SYS_ORDER_HEAD_LEN)
#define SYS_ORDER_LEN              (16*1024)


#define	RECORD_BLOCK_SIZE_PER_PAGE		        256
#define	RECORD_BLOCK_NUM_PER_PAGE		        (FLASH_SECTOR_SIZE/RECORD_BLOCK_SIZE_PER_PAGE)        //2048/128=16每个扇区存 16笔订单
#define	RECORD_FLASH_SIZE				        (128*64)	//(16K)
#define RECORD_MAX_BLOCK_NUM			        (RECORD_FLASH_SIZE/RECORD_BLOCK_SIZE_PER_PAGE)      //最大可存储订单数量 128笔 也是环形队列的深度
#define MAX_RECORD						        (RECORD_MAX_BLOCK_NUM-RECORD_BLOCK_NUM_PER_PAGE)    //实际最大存储订单数量 112笔 需要空一个扇区


uint8_t App_Read_Flash(uint8_t *read_data,uint32_t address, uint32_t size);
uint8_t App_Write_Flash(uint8_t *write_data,uint32_t address, uint32_t size);
#endif

