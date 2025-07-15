/*** 
 * @Author: chenyuan
 * @Date: 2024-08-21 18:27:24
 * @LastEditors: chenyuan
 * @LastEditTime: 2024-08-22 14:46:37
 * @FilePath: \N32G45x_FreeRTOS工程模板\Bsw\BswSrv\BswSrv_GD25xx.h
 * @Description: 
 * @
 * @Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#ifndef _BSWSRV_GD32XX_H_
#define _BSWSRV_GD32XX_H_


#include "gd32f30x.h"
#include "spi.h"
//GD25 Instruction Set Table 1
#define	GD25_WriteEnable 										0x06
#define	GD25_WriteDisable 									    0x04
#define	GD25_Read_Status_Register_1 				            0x05
#define	GD25_Read_Status_Register_2 				            0x35
#define	GD25_Write_Status_Register 					            0x01
#define	GD25_Page_Program 									    0x02
#define	GD25_Quad_Page_Program 							        0x32
#define	GD25_Block_Erase_64KB 							        0xD8
#define	GD25_Block_Erase_32KB 							        0x52
#define	GD25_Sector_Erase_4KB 							        0x20
#define	GD25_Chip_Erase 										0xC7
#define	GD25_Erase_Suspend 									    0x75
#define	GD25_Erase_Resume 									    0x7A
#define	GD25_Power_down 										0xB9
#define	GD25_High_Performance_Mode 					            0xA3
#define	GD25_Continuous_Read_ModeReset			                0xFF
#define	GD25_Release_Power_down							        0xAB
#define	GD25_HPM_OR_Device_ID								    0xAB
#define	GD25_Manufacturer_OR_Device_ID 			                0x90
#define	GD25_Read_Unique_ID	 								    0x4B
#define	GD25_JEDEC_ID 									        0x9F

//GD25 Instruction Set Table 2(Read Instructions)
#define	GD25_Read_Data 									0x03
#define	GD25_Fast_Read 									0x0B
#define	GD25_Fast_Read_Dual_Output 			0x3B
#define	GD25_Fast_Read_Dual_I_O 				0xBB
#define	GD25_Fast_Read_Quad_Output 			0x6B
#define	GD25_Fast_Read_Quad_I_O 				0xEB
#define	GD25_Octal_Word Read_Quad_I_O 	0xE3

#define Dummy_Byte												0xFF
/**************************************************************/



#define GD25x_OK            ((uint8_t)0x00)
#define GD25x_ERROR         ((uint8_t)0x01)
#define GD25x_BUSY          ((uint8_t)0x02)
#define GD25x_TIMEOUT				((uint8_t)0x03)


/* Flag Status Register */
#define GD25128FV_FSR_BUSY                    ((uint8_t)0x01)    /*!< busy */
#define GD25128FV_FSR_WREN                    ((uint8_t)0x02)    /*!< write enable */
#define GD25128FV_FSR_QE                      ((uint8_t)0x02)    /*!< quad enable */




#define GD25xx_CS_LOW() 			SPI0_CS_LOW()
#define GD25xx_CS_HIGH() 			SPI0_CS_HIGH()

#define GD25xx_TIMEOUT_VALUE			1000
#define GD25_PageSize						256


uint8_t BswSrv_GD25xx_Read_Byte(void);
uint8_t BswSrv_GD25xx_Write_Byte(uint8_t Tx_Byte);
void BswSrv_GD25xx_WriteEnable(void);
static void BswSrv_GD25xx_Wait_for_Write_End(void);
uint8_t BswSrv_GD25xx_BufferRead(uint8_t *ReadBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);

void BswSrv_GD25xx_PageWrite(uint8_t *WriteBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite);
void BswSrv_GD25xx_BufferWrite(uint8_t *WriteBuffer, uint32_t ReadAddr, uint32_t NumByteToWrite);
void BswSrv_GD25xx_ReadID(void);
void BswSrv_GD25xx_SectorErase(uint32_t Address);
void BSP_W25Qxx_SectorErase(uint32_t SectorAddr);





#endif
