/*
 * @Author: chenyuan
 * @Date: 2025-03-17 14:15:29
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-04-02 19:33:03
 * @FilePath: \13.FreeRTOS实验\Hardware\Config\SPI\spi.c
 * @Description: 
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#include "spi.h"


/**
 * @description: SPI写单个数据
 * @Author: 
 * @param {SPI_TYPE} spi_x  spi类型
 * @return {*}
 */
uint8_t SpiReadWrite(uint32_t SPIx,uint8_t InData)
{
		uint8_t retry = 0;

		/* Loop while DR register in not emplty */
		while (spi_i2s_flag_get(SPIx, SPI_FLAG_TBE) == RESET)
		{
			retry ++;
			if(retry > 200) return 0;	
		}

		/* Send byte through the SPI peripheral */
		spi_i2s_data_transmit(SPIx, InData);

		retry = 0;
		/* Wait to receive a byte */
		while (spi_i2s_flag_get(SPIx, SPI_FLAG_RBNE) == RESET)
		{
			retry ++;
			if(retry > 200) return 0;			
		}

		/* Return the byte read from the SPI bus */
		return spi_i2s_data_receive(SPIx);

}

/**
 * @description: SPI写多个数据
 * @Author: 
 * @param {SPI_TYPE} spi_x  spi类型
 * @return {*}
 */
void YA_SpiWriteData(uint32_t SPIx,uint8_t byteCount,uint8_t* pData)
{
		uint8_t i;

		for(i=0;i<byteCount;i++)
		{
		 	SpiReadWrite(SPIx,pData[i]);
		}
}

/**
 * @description: SPI读取单个数据
 * @Author: 
 * @param {SPI_TYPE} spi_x  spi类型
 * @return {*}
 */
uint8_t YA_SpiReadData(uint32_t SPIx)
{
	if (SPIx == NFC_SPI)
	{
		return (SpiReadWrite(SPIx,0xA5));
	}
	else
	{	
		return (SpiReadWrite(SPIx,0xff));
	}
	
	
}


/**
 * @description: SPI读取多个数据
 * @Author: 
 * @Date: 
 * @param {SPI_TYPE} spi_x  spi类型
 * @param {uint8_t} *data  保存地址
 * @param {uint32_t} len  读取长度
 * @return {*}
 */
void YA_SpiRead_NbyteData(uint32_t spi_x, uint8_t *data, uint32_t len)
{
    uint32_t read_len = 0;
    if(len <= 0)
      return;

    for (read_len = 0; read_len < len; read_len++)
    {
        data[read_len] = YA_SpiReadData(spi_x);
    }
    
}

void YA_SpiInit(SPI_TYPE type)
{

	spi_parameter_struct spi_init_struct;

	if (type == SPI_FLASH)
	{	
		rcu_periph_clock_enable(RCU_SPI0);
		rcu_periph_clock_enable(RCU_AF);
		
		//spi0_ncc-PA4, SPI0_SCK-PA5, SPI0_MOSI-PA7, SPI0_MISO-PA6
		gpio_init(W25Q64_WP_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, W25Q64_WP_PIN);
		gpio_init(SPI0_CS_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, SPI0_CS_PIN);
		gpio_init(SPI0_CLK_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ,  SPI0_CLK_PIN | SPI0_MISO_PIN | SPI0_MOSI_PIN);

		SPI0_CS_HIGH();
		
		/* SPI parameter config */
		spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
		spi_init_struct.device_mode          = SPI_MASTER;
		spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
		spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
		spi_init_struct.nss                  = SPI_NSS_SOFT;
		spi_init_struct.prescale             = SPI_PSC_256;
		spi_init_struct.endian               = SPI_ENDIAN_MSB;
		spi_init(SPI0, &spi_init_struct);

		/* set crc polynomial */
		spi_crc_polynomial_set(SPI0,7);
		/* enable SPI0 */
		spi_enable(SPI0);
		W25Q64_WP_EN;

	}
	else if (type == SPI_NFC)
	{
		rcu_periph_clock_enable(RCU_SPI2);
		rcu_periph_clock_enable(RCU_AF);
		gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE); 
		//spi0_ncc-PA4, SPI0_SCK-PA5, SPI0_MOSI-PA7, SPI0_MISO-PA6
		gpio_init(SPI2_CS_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, SPI2_CS_PIN);
		gpio_init(SPI2_CLK_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ,  SPI2_CLK_PIN | SPI2_MISO_PIN | SPI2_MOSI_PIN);


		gpio_init(NFC_NPD_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, NFC_NPD_PIN);

		SPI2_CS_HIGH();
		
		/* SPI parameter config */
		spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
		spi_init_struct.device_mode          = SPI_MASTER;
		spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
		spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
		spi_init_struct.nss                  = SPI_NSS_SOFT;
		spi_init_struct.prescale             = SPI_PSC_256;
		spi_init_struct.endian               = SPI_ENDIAN_MSB;
		spi_init(SPI2, &spi_init_struct);

		/* set crc polynomial */
		spi_crc_polynomial_set(SPI2,7);
		/* enable SPI0 */
		spi_enable(SPI2);
	}
    
}


