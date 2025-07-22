#include "flash.h"
#include "string.h"
uint8_t flash_buf[FLASH_BUF_SIZE];
uint8_t iapbuf[FLASH_BUF_SIZE];
//按2字节写入
uint8_t Bsp_WriteWord(uint32_t addr, uint8_t *pBuffer, uint16_t len)
{
    uint16_t i;
    uint32_t data = 0;
    uint32_t writeAddr = addr;

    if((addr < FLASH_BASE) || ((addr + len) > (FLASH_BASE + FLASH_SIZE)))
    {
        return 1;
    }
	
	
	for(i = 0;i < len;i++)
    {
        memcpy(&data,pBuffer + i*4,4);
        fmc_word_program(writeAddr, data);
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
        writeAddr += 4;
        
    } 
	
	
    return 0; 
}


//读flash数据
uint8_t App_Read_Flash(uint8_t *read_data,uint32_t address, uint32_t size)
{
    uint32_t i = 0;
    for (i = 0; i < size; i++)
    {
        read_data[i] = REG8(address);
        address++;
    }
    return 0;
    
}


//flash擦除数据
uint8_t App_Flash_Erase(uint32_t address, uint32_t size)
{
	uint32_t offaddr = 0;   
	uint32_t secpos = 0;
    uint32_t falshPageSize = 0;
    uint32_t eraseAddress = 0;
    if(address < FLASH_BASE || ((address+size) >= (FLASH_BASE+FLASH_SIZE)))
    {
        return 0;
    }
    fmc_unlock();

    while(1)
    {
        falshPageSize = FLASH_SECTOR_SIZE;
        offaddr = address - FLASH_BASE;
        secpos = offaddr/falshPageSize;
        eraseAddress = FLASH_BASE + secpos * falshPageSize;

		
        fmc_page_erase(eraseAddress);//擦除扇区
    
        fmc_flag_clear(FMC_FLAG_BANK0_END);  
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
		
        
        address = eraseAddress + falshPageSize;
        size -= falshPageSize;
        if(size == 0)
        {
            break;
        }  
    }

	fmc_lock();
    return 0;
}

//flash写入数据
uint8_t App_Write_Flash(uint8_t *write_data,uint32_t address, uint32_t size)
{
    uint32_t secpos = 0;
	uint16_t secoff = 0;
	uint16_t secremain = 0;
 	uint16_t i = 0;
	uint32_t offaddr = 0;
    uint32_t falshPageSize = 0;
    uint32_t writePageAddr = 0;
    if(address < FLASH_BASE || ((address+size) >= (FLASH_BASE+FLASH_SIZE))) 
    {
        return 0;
    }
    fmc_unlock(); 
    while(1)
	{

        write_data += secremain;  	                       
        address += secremain;	                        
        size -= secremain;	              
        
        offaddr = address - FLASH_BASE;		   
        falshPageSize = FLASH_SECTOR_SIZE;
        
        secpos = offaddr/falshPageSize;			
        secoff = (offaddr%falshPageSize);		
        secremain = falshPageSize - secoff;		
        writePageAddr = FLASH_BASE + secpos*falshPageSize;
        
        if(size <= secremain)           
        {
            secremain = size;
        }
		App_Read_Flash(flash_buf,writePageAddr,falshPageSize);
		for(i = 0;i < falshPageSize;i++)      
		{
			if(flash_buf[i] != 0XFF)             
			{
				break;
			}
		}
		if(i < falshPageSize)                       //需要擦除
		{
			fmc_page_erase(writePageAddr);          //擦除这个扇区
            fmc_flag_clear(FMC_FLAG_BANK0_END);               //清除所有标志位
            fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
            fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
			for(i=0;i < secremain;i++)    
			{
				flash_buf[i+secoff]=write_data[i];
			}

			Bsp_WriteWord(writePageAddr,(void *)flash_buf,falshPageSize/4);//写入整个扇区
		}
        else
		{
            for(i=0;i < secremain;i++)        
			{
				flash_buf[i+secoff]=write_data[i];
			}

			Bsp_WriteWord(writePageAddr,(void *)flash_buf,falshPageSize/4);//写入整个扇区
		}

		if(size == secremain)              
		{
			break;
		}
	}

    fmc_lock();
    return 1;
}

