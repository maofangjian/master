#include "App_GD25xx.h"
#include "spi.h"
#include "timer.h"
#include "global.h"
/**
  * @brief  向FLASH发送 写使能 命令
  * @param  none
  * @retval none
  */
void BswSrv_GD25xx_WriteEnable(void)
{
	uint8_t cmd = GD25_WriteEnable;
	
	/* 通讯开始，CS拉低 */
	GD25xx_CS_LOW();
	
  /* 发送写使能命令*/

    // BswDrv_Spi_Write_Data(SPI_EXTERAL_FLASH,&cmd,1);
	
	YA_SpiWriteData(FLASH_SPI,1,&cmd);
	/* 通讯结束，CS拉高 */
	GD25xx_CS_HIGH();
	Delay_ms(1);
}


/**
  * @brief  FLASH 等待写结束
  * @param  none
  * @retval none
  */
static void BswSrv_GD25xx_Wait_for_Write_End(void)
{
  uint8_t state = 0;
	uint8_t cmd = GD25_Read_Status_Register_1;

	/* 通讯开始，CS拉低 */
  GD25xx_CS_LOW();

  /* 发送命令 */
  //BswDrv_Spi_Write_Data(SPI_EXTERAL_FLASH,&cmd,1);
  YA_SpiWriteData(FLASH_SPI,1,&cmd);	
  do
  {
    	//BswDrv_Spi_Read_Data(SPI_EXTERAL_FLASH,&state,1);
		YA_SpiRead_NbyteData(FLASH_SPI,&state,1);	
  }while((state & 0x01) == SET);
	/* 通讯结束，CS拉高 */
  GD25xx_CS_HIGH();
  Delay_ms(1);
}

/**
  * @brief   读取FLASH数据
  * @param 	 ReadBuffer，存储读出的数据的指针
  * @param   ReadAddr，读取地址
  * @param   NumByte，读取数据长度
  * @retval  无
  */
uint8_t BswSrv_GD25xx_BufferRead(uint8_t *ReadBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
	uint8_t cmd[4];
	cmd[0] = GD25_Fast_Read;
	cmd[1] = (uint8_t)(ReadAddr >> 16);
	cmd[2] = (uint8_t)(ReadAddr >> 8);
	cmd[3] = (uint8_t)ReadAddr;

	/* 通讯开始，CS拉低 */
	GD25xx_CS_LOW();
  
	/* 写入指令、地址 */	
	//BswDrv_Spi_Write_Data(SPI_EXTERAL_FLASH,cmd,4);
	SpiReadWrite(FLASH_SPI,GD25_Fast_Read);
	SpiReadWrite(FLASH_SPI,(uint8_t)(ReadAddr >> 16));
	SpiReadWrite(FLASH_SPI,(uint8_t)(ReadAddr >> 8));
	SpiReadWrite(FLASH_SPI,(uint8_t)ReadAddr);

    //BswDrv_Spi_Write_Read_Byte(SPI_EXTERAL_FLASH,0xff);
	SpiReadWrite(FLASH_SPI,0xff);
	/* 读取数据 */
    //BswDrv_Spi_Read_Data(SPI_EXTERAL_FLASH,ReadBuffer,NumByteToRead);
	for (uint32_t i = 0; i < NumByteToRead; i++)
	{
		ReadBuffer[i] = SpiReadWrite(FLASH_SPI,0xff);
	}
	
	// YA_SpiRead_NbyteData(FLASH_SPI,ReadBuffer,NumByteToRead);
	/* 通讯结束，CS拉高 */
	GD25xx_CS_HIGH();
	Delay_ms(1);
	
	return GD25x_OK;
}

 /**
  * @brief  对FLASH进行页写入数据，调用本函数写入数据前需要先擦除扇区
  * @param	WriteBuffer，要写入的数据的指针
  * @param  WriteAddr，写入地址
  * @param  NumByteToWrite，写入数据长度
  * @retval 无
  */
void BswSrv_GD25xx_PageWrite(uint8_t *WriteBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite)
{
//	uint16_t i;
	
	// uint8_t cmd[4];
	// cmd[0] = GD25_Page_Program;
	// cmd[1] = (uint8_t)(WriteAddr >> 16);
	// cmd[2] = (uint8_t)(WriteAddr >> 8);
	// cmd[3] = (uint8_t)WriteAddr;
	/* 发送FLASH写使能命令 */
	BswSrv_GD25xx_WriteEnable();
	/* 通讯开始，CS拉低 */
	GD25xx_CS_LOW();
	/* 写入指令、地址 */	
	//BswDrv_Spi_Write_Data(SPI_EXTERAL_FLASH,cmd,4);
	SpiReadWrite(FLASH_SPI,GD25_Page_Program);
	SpiReadWrite(FLASH_SPI,(uint8_t)(WriteAddr >> 16));
	SpiReadWrite(FLASH_SPI,(uint8_t)(WriteAddr >> 8));
	SpiReadWrite(FLASH_SPI,(uint8_t)WriteAddr);
	/* 写入数据 */
    //BswDrv_Spi_Write_Data(SPI_EXTERAL_FLASH,WriteBuffer,NumByteToWrite);
	for (uint32_t i = 0; i < NumByteToWrite; i++)
	{
		SpiReadWrite(FLASH_SPI,(uint8_t)WriteBuffer[i]);
	}
	
	//YA_SpiWriteData(FLASH_SPI,NumByteToWrite,WriteBuffer);
	/* 通讯结束，CS拉高 */
	GD25xx_CS_HIGH();
	Delay_ms(1);
	//需要在 W25Qxx_CS_HIGH 之后，即数据传输开始之后
	BswSrv_GD25xx_Wait_for_Write_End();
}

/**
  * @brief  对FLASH写入数据，调用本函数写入数据前需要先擦除扇区
  * @param	WriteBuffer，要写入数据的指针
  * @param  WriteAddr，写入地址
  * @param  NumByteToWrite，写入数据的长度
  * @retval none
  */
void BswSrv_GD25xx_BufferWrite(uint8_t *WriteBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite)
{
	uint32_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
	
/************************* 对于写入一共四种情况 *******************/
/**			起始地址 WriteAddr 和某一页的起始地址对齐
	*1、数据小于一页
	*2、数据大于一页
	*
	*		起始地址 WriteAddr 不和某一页的起始地址对齐
	*3、数据小于一页
	*4、数据大于一页
	*/
/******************************************************************/

/*mod运算求余，若WriteAddr是W25Q64_PageSize整数倍，运算结果Addr值为0*/
  Addr = WriteAddr % GD25_PageSize;
	/*差count个字节数据，刚好可以对齐到页地址*/
	count = GD25_PageSize - Addr;
	/*计算出要写多少整数页*/
	NumOfPage = NumByteToWrite / GD25_PageSize;
	/*mod运算求余，计算出剩余不满一页的字节数*/
	NumOfSingle = NumByteToWrite % GD25_PageSize;
	
	 /* Addr=0,则起始地址WriteAddr刚好是某一页的起始地址 */
	 if(Addr == 0)
	 {
			/* 写入的数据小于一页：NumByteToWrite < W25Q64_PageSize */
			if(NumOfPage == 0)
			{
				BswSrv_GD25xx_PageWrite(WriteBuffer, WriteAddr, NumByteToWrite);
			}
			else /* 写入的数据不小于一页：NumByteToWrite >= W25Q64_PageSize */
			{
				/* 先把整数页都写了 */
				while(NumOfPage--)
				{
					BswSrv_GD25xx_PageWrite(WriteBuffer, WriteAddr, GD25_PageSize);
					WriteAddr +=  GD25_PageSize;
					WriteBuffer += GD25_PageSize;
				}
				
				/* 若有不满一页的数据，则把它写完 */
				if(NumOfSingle)
					BswSrv_GD25xx_PageWrite(WriteBuffer, WriteAddr, NumOfSingle);
			}
	 }
	 /* Addr!=0,则起始地址WriteAddr不是某一页的起始地址 */
	 else
	 {
			/* 写入的数据小于一页：NumByteToWrite < W25Q64_PageSize */
			if(NumOfPage == 0)
			{
				/* 但是尾地址在下一页 */
				if(NumOfSingle > count )
				{
					temp = NumOfSingle - count;
					
					/* 先写满当前页 */
					BswSrv_GD25xx_PageWrite(WriteBuffer, WriteAddr, count);
					WriteAddr +=  count;
					WriteBuffer += count;
					
					/* 再写下一页的剩余数据 */
					BswSrv_GD25xx_PageWrite(WriteBuffer, WriteAddr, temp);
				}
				/* 尾地址和起始地址WriteAddr在同一页 */
				else
				{
					BswSrv_GD25xx_PageWrite(WriteBuffer, WriteAddr, NumByteToWrite);
				}
			}
			else/* 写入的数据不小于一页：NumByteToWrite >= W25Q64_PageSize */
			{
				/* 把头部不对齐的部分(count)单独处理一下，就跟对齐了的是一种情况了 */
				BswSrv_GD25xx_PageWrite(WriteBuffer, WriteAddr, count);
				
				NumByteToWrite -= count;
				NumOfPage =  NumByteToWrite / GD25_PageSize;
				NumOfSingle = NumByteToWrite % GD25_PageSize;
				
				WriteAddr +=  count;
				WriteBuffer += count;
				
				/* 先把整数页都写了 */
				while(NumOfPage--)
				{
					BswSrv_GD25xx_PageWrite(WriteBuffer, WriteAddr, GD25_PageSize);
					WriteAddr +=  GD25_PageSize;
					WriteBuffer += GD25_PageSize;
				}
				/* 若有不满一页的数据，则把它写完 */
				if(NumOfSingle)
					BswSrv_GD25xx_PageWrite(WriteBuffer, WriteAddr, NumOfSingle);
			}
	 }
}

/**
  * @brief  擦除FLASH扇区
  * @param  SectorAddr：要擦除的扇区地址
  * @retval none
  */
void BswSrv_GD25xx_SectorErase(uint32_t SectorAddr)
{
	// uint8_t cmd[4];
	// cmd[0] = GD25_Sector_Erase_4KB;
	// cmd[1] = (uint8_t)(SectorAddr >> 16);
	// cmd[2] = (uint8_t)(SectorAddr >> 8);
	// cmd[3] = (uint8_t)SectorAddr;
	
	/* 发送FLASH写使能命令 */
	BswSrv_GD25xx_WriteEnable();
	
	/* 通讯开始，CS拉低 */
	GD25xx_CS_LOW();
	
	/* 发送命令和地址 */
    //BswDrv_Spi_Write_Data(SPI_EXTERAL_FLASH,cmd,1);
	SpiReadWrite(FLASH_SPI,GD25_Sector_Erase_4KB);
	SpiReadWrite(FLASH_SPI,(uint8_t)(SectorAddr >> 16));
	SpiReadWrite(FLASH_SPI,(uint8_t)(SectorAddr >> 8));
	SpiReadWrite(FLASH_SPI,(uint8_t)SectorAddr);
	
	/* 通讯结束，CS拉高 */
	GD25xx_CS_HIGH();
	Delay_ms(150);
	
	/* 需要在W25Qxx_CS_HIGH 之后，即数据传输开始之后 */
	BswSrv_GD25xx_Wait_for_Write_End();
}


/**
  * 函    数：MPU6050读取ID号
  * 参    数：MID 工厂ID，使用输出参数的形式返回
  * 参    数：DID 设备ID，使用输出参数的形式返回
  */
void BswSrv_GD25xx_ReadID(void)
{	
	uint8_t cmd = GD25_Manufacturer_OR_Device_ID;
	uint8_t id[2];
	/* 通讯开始，CS拉低 */
	GD25xx_CS_LOW();
	
  /* 发送写使能命令*/
    //BswDrv_Spi_Write_Data(SPI_EXTERAL_FLASH,&cmd,1);
	//YA_SpiWriteData(FLASH_SPI,1,&cmd);
	SpiReadWrite(FLASH_SPI,GD25_Manufacturer_OR_Device_ID);
	SpiReadWrite(FLASH_SPI,0);
	SpiReadWrite(FLASH_SPI,0);
	SpiReadWrite(FLASH_SPI,0);
	/* 读取数据 */
    // BswDrv_Spi_Read_Data(SPI_EXTERAL_FLASH,id,4);
	YA_SpiRead_NbyteData(FLASH_SPI,id,2);
	/* 通讯结束，CS拉高 */
	GD25xx_CS_HIGH();
	LOG("id :%x %x\r\n",id[0],id[1]);
	Delay_ms(1);
}


void BSP_W25Qxx_SectorErase(uint32_t SectorAddr)
{
	// uint8_t cmd[4];
	// cmd[0] = GD25_Sector_Erase_4KB;
	// cmd[1] = (uint8_t)(SectorAddr >> 16);
	// cmd[2] = (uint8_t)(SectorAddr >> 8);
	// cmd[3] = (uint8_t)SectorAddr;
	/* 发送FLASH写使能命令 */
	BswSrv_GD25xx_WriteEnable();
    BswSrv_GD25xx_Wait_for_Write_End();
	/* 通讯开始，CS拉低 */
	GD25xx_CS_LOW();
	/* 发送命令和地址 */
    //BswDrv_Spi_Write_Data(SPI_EXTERAL_FLASH,cmd,1);
	SpiReadWrite(FLASH_SPI,GD25_Sector_Erase_4KB);
	SpiReadWrite(FLASH_SPI,(uint8_t)(SectorAddr >> 16));
	SpiReadWrite(FLASH_SPI,(uint8_t)(SectorAddr >> 8));
	SpiReadWrite(FLASH_SPI,(uint8_t)SectorAddr);
	/* 通讯结束，CS拉高 */
	GD25xx_CS_HIGH();
	Delay_ms(1);

	/* 需要在W25Qxx_CS_HIGH 之后，即数据传输开始之后 */
	BswSrv_GD25xx_Wait_for_Write_End();
}

