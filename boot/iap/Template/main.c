
#include <stdio.h>
#include <string.h>	
#include <stdarg.h>


#include "gd32f30x.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "flash.h"
#include "gd32f303e_eval.h"//#include "gd32f307c_eval.h"







#define FLASH_SIZE                  (512*1024)  //GD32F303 flash��С
#define FLASH_SECTOR_SIZE           (2*1024)  //һ��������С2k
#define APP_MAIN_ADDR               FLASH_BASE
#define APP_MAIN_SIZE               (196*1024)

#define APP_BACK_ADDR               (APP_MAIN_ADDR+APP_MAIN_SIZE)
#define APP_BACK_SIZE               APP_MAIN_SIZE
#define FLASH_BUF_SIZE              (4096)

#define SYS_UPGRADE_ADDR            (APP_BACK_ADDR+APP_BACK_SIZE)
#define SYS_UPGRADE_SIZE            (4096)

#define FLASH_APP1_ADDR 	   0x08003000	   //��һ��Ӧ�ó�����ʼ��ַ(�����FLASH)
#define FLASH_APP2_ADDR		   APP_BACK_ADDR
#define APP_LOADED_ADDR                    0x08003000
typedef  void  (*pAppFunction) (void);
pAppFunction application;
uint32_t app_address;

typedef struct 
{
	uint8_t upgrade_flag;
	uint32_t size;
	uint16_t crc;
}Upgrade_Info;

Upgrade_Info upgrade_info;


uint8_t FLASH_byteBUF[2048];//�����2K�ֽ�


void iap_load_app(uint32_t appxaddr)
{
  if (((*(__IO uint32_t*)APP_LOADED_ADDR) & 0x2FFE0000) == 0x20000000) {
			app_address = *(__IO uint32_t*) (APP_LOADED_ADDR + 4);
			application = (pAppFunction) app_address;
			__set_MSP(*(__IO uint32_t*) APP_LOADED_ADDR);
			application();
	}
}


void YA_ComInit(void)
{

    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOD);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART2);

      rcu_periph_clock_enable(RCU_AF);
	  gpio_pin_remap_config(GPIO_USART2_FULL_REMAP, ENABLE);  //USART2��ӳ��
      /* connect port to USARTx_Tx */
      gpio_init(GPIOD, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);

    //   /* connect port to USARTx_Rx */
    //   gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    /* USART configure */
    usart_deinit(USART2);
    usart_baudrate_set(USART2, 115200);
		usart_parity_config(USART2, USART_PM_NONE);             //����żУ��
		usart_word_length_set(USART2, USART_WL_8BIT);           //8λ����λ

      usart_stop_bit_set(USART2, USART_STB_1BIT);             //1λֹͣλ
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);
    usart_enable(USART2);
}
void Bsp_DebugSend(uint8_t *pData, uint16_t len)
{
  uint16_t i = 0;

    for(i = 0; i < len; i++)
    {
        usart_data_transmit(USART2, (uint8_t)pData[i]);
        while(RESET == usart_flag_get(USART2, USART_FLAG_TBE));
    }
}
static volatile uint16_t Writecnt = 0;
static __IO uint8_t  PrintBuff[200];   //����������ĳ���
int fputc(int ch, FILE *f)
{
    if (Writecnt < sizeof(PrintBuff)) 
    {
        PrintBuff[Writecnt++] = ch;
        if ('\n' == ch) 
        {
            Bsp_DebugSend( (void *)PrintBuff, Writecnt);
            Writecnt = 0;
        }
    }
    else
    {                        
		    Bsp_DebugSend((void *)PrintBuff, sizeof(PrintBuff));
        Writecnt = 0;
	  }
    return ch;
}
/**********************************************************************
??3?��oGotoApp
���¨�?��o��?��a��?app  
��?��?��o?T 
��?3?��o?T 
***********************************************************************/
void jumpToApp(void)
{
//	USART_Cmd(USART6, DISABLE);//--__disable_irq()ֻ�ǽ�ֹCPUȥ��Ӧ�жϣ�û��������ȥ�����жϵĴ�����
//	TIM_Cmd(TIM3, DISABLE); 
	printf("111111111\r\n");
	if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
	{	 
		//__disable_irq();//__set_FAULTMASK(1);//�ر������ж�
		iap_load_app(FLASH_APP1_ADDR);//ִ��FLASH APP����
	}
}

//--���εĴ�����Ҫ�Լ��ο�ʵ�ֹ���

void up_program(void)
{
	uint32_t write_flash_addr = 0;
	uint16_t from_app[4],last_data;//--���from_app[4]�����������洢�Ǵ�ָ����ַ��ȡ���ݣ������ж������ģ���Ҫ��Ӧ�ó������Լ�ʵ�֡�
	uint32_t data_page;
	uint16_t ck = 0;
	uint8_t rbyte = 0;
	App_Read_Flash((void *)&upgrade_info,SYS_UPGRADE_ADDR,sizeof(Upgrade_Info));
	printf("read data :%x %d %x\r\n",upgrade_info.upgrade_flag,upgrade_info.size,upgrade_info.crc);
		if(upgrade_info.upgrade_flag == 0xa5  && upgrade_info.size > 0)
		{//-----�г���Ҫ����
			uint32_t i = 0;
			data_page = upgrade_info.size / 2048+1;//----------------------------------�����ж���ҳ
			if(upgrade_info.size >= 2048)//--------------------------------------------����������1ҳ
				last_data = upgrade_info.size - data_page*2048;//--------------------�ж��ٸ�����һҳ��Χ�ڵ�����
			else
				last_data = upgrade_info.size;//---------------------------------------����û��1ҳ
			for(i = 0;i < data_page;i++){
				App_Read_Flash(FLASH_byteBUF,APP_BACK_ADDR + i*2048,2048);//------------------�����ݶ������ŵ����鵱��
				write_flash_addr = FLASH_APP1_ADDR + i*2048;	
				App_Write_Flash(FLASH_byteBUF,write_flash_addr,2048);
				printf("updata :%d addr:%x\r\n",i,write_flash_addr);
				//iap_write_appbin(write_flash_addr,FLASH_byteBUF,2048);//-------����FLASH���� 
			}
			
			for(i = 0; i < upgrade_info.size; i++) 
			{
				App_Read_Flash(&rbyte,FLASH_APP1_ADDR+i,1);
				ck += rbyte;
			}
			printf("upgrade :%x\r\n",ck);
			if(ck == upgrade_info.crc)
			{
				memset((void *)&upgrade_info,0,sizeof(Upgrade_Info));
				App_Write_Flash((void *)&upgrade_info,SYS_UPGRADE_ADDR,sizeof(Upgrade_Info));
			}
			//memset((void *)&upgrade_info,0,sizeof(Upgrade_Info));
			//App_Write_Flash((void *)&upgrade_info,SYS_UPGRADE_ADDR,sizeof(Upgrade_Info));
		}
		jumpToApp();

}


int main(void)
{	
	systick_config();
	YA_ComInit();
	printf("boot runing \r\n");
	up_program();

	while(1)
	{
		;
	}
			
}



