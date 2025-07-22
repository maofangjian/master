/*!
    \file    gd32f303e_eval.c
    \brief   firmware functions to manage leds, keys, COM ports

    \version 2017-02-10, V1.0.0, firmware for GD32F30x
    \version 2018-10-10, V1.1.0, firmware for GD32F30x
    \version 2018-12-25, V2.0.0, firmware for GD32F30x
    \version 2020-09-30, V2.1.0, firmware for GD32F30x 
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include <gd32f30x.h>
#include "gd32f303e_eval.h"
//#include "i2c.h"
#include "systick.h"

#define   DELAY_IIC_US          1//5
#define I2C_SDA_IN()  gpio_init(GPIOB, GPIO_MODE_IPD, GPIO_OSPEED_10MHZ,GPIO_PIN_9);//{GPIO_CTL0(GPIOB)&=~GPIO_MODE_MASK(9);GPIO_CTL0(GPIOB)|=GPIO_MODE_SET(9,0x8);}//PB9输入模式
#define I2C_SDA_OUT() gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ,GPIO_PIN_9);//{GPIO_CTL0(GPIOB)&=~GPIO_MODE_MASK(9);GPIO_CTL0(GPIOB)|=GPIO_MODE_SET(9,0x3);}//PB9输出模式
#define I2C_SDA_READ	gpio_input_bit_get(GPIOB,GPIO_PIN_9)




#define IIC_SCL(n) (n?gpio_bit_set(GPIOB, GPIO_PIN_8):gpio_bit_reset(GPIOB, GPIO_PIN_8))
#define IIC_SDA(n) (n?gpio_bit_set(GPIOB, GPIO_PIN_9):gpio_bit_reset(GPIOB, GPIO_PIN_9))




//extern void delay_us(unsigned int nus);
extern void delay_us_x(uint32_t nus);
/* private variables */
static uint32_t GPIO_PORT[LEDn] = {LED2_GPIO_PORT, IIC_SDA_GPIO_PORT,
                                   IIC_SCL_GPIO_PORT, LED5_GPIO_PORT};
static uint32_t GPIO_PIN[LEDn] = {LED2_PIN, IIC_SDA_PIN, IIC_SCL_PIN, LED5_PIN};

static rcu_periph_enum COM_CLK[COMn] = {EVAL_COM1_CLK, EVAL_COM2_CLK};
static uint32_t COM_TX_PIN[COMn] = {EVAL_COM1_TX_PIN, EVAL_COM2_TX_PIN};
static uint32_t COM_RX_PIN[COMn] = {EVAL_COM1_RX_PIN, EVAL_COM2_RX_PIN};
static uint32_t COM_GPIO_PORT[COMn] = {EVAL_COM1_GPIO_PORT, EVAL_COM2_GPIO_PORT};
static rcu_periph_enum COM_GPIO_CLK[COMn] = {EVAL_COM1_GPIO_CLK, EVAL_COM2_GPIO_CLK};

static rcu_periph_enum GPIO_CLK[LEDn] = {GPIOA_CLK_ENAABLE, GPIOB_CLK_ENAABLE, 
                                         IIC_SCL_GPIO_CLK, GPIOF_CLK_ENAABLE};//---端口时钟使能

static uint32_t KEY_PORT[KEYn] = {WAKEUP_KEY_GPIO_PORT, 
                                  TAMPER_KEY_GPIO_PORT,
                                  USER_KEY1_GPIO_PORT,
                                  USER_KEY2_GPIO_PORT};
static uint32_t KEY_PIN[KEYn] = {WAKEUP_KEY_PIN, TAMPER_KEY_PIN,USER_KEY1_PIN,USER_KEY2_PIN};
static rcu_periph_enum KEY_CLK[KEYn] = {WAKEUP_KEY_GPIO_CLK, 
                                        TAMPER_KEY_GPIO_CLK,
                                        USER_KEY1_GPIO_CLK,
                                        USER_KEY2_GPIO_CLK};
static exti_line_enum KEY_EXTI_LINE[KEYn] = {WAKEUP_KEY_EXTI_LINE,
                                             TAMPER_KEY_EXTI_LINE,
                                             USER_KEY1_EXTI_LINE,
                                             USER_KEY2_EXTI_LINE};
static uint8_t KEY_PORT_SOURCE[KEYn] = {WAKEUP_KEY_EXTI_PORT_SOURCE,
                                        TAMPER_KEY_EXTI_PORT_SOURCE,
                                        USER_KEY1_EXTI_PORT_SOURCE,
                                        USER_KEY2_EXTI_PORT_SOURCE};
static uint8_t KEY_PIN_SOURCE[KEYn] = {WAKEUP_KEY_EXTI_PIN_SOURCE,
                                       TAMPER_KEY_EXTI_PIN_SOURCE,
                                       USER_KEY1_EXTI_PIN_SOURCE,
                                       USER_KEY2_EXTI_PIN_SOURCE};
static uint8_t KEY_IRQn[KEYn] = {WAKEUP_KEY_EXTI_IRQn, 
                                 TAMPER_KEY_EXTI_IRQn,
                                 USER_KEY1_EXTI_IRQn,
                                 USER_KEY2_EXTI_IRQn};

#define   PartAdd          0x7c     //[]


#define AT24C64	  8191
#define EE_TYPE   AT24C64
#define AT24C16		2047


/*!
    \brief      configure led GPIO
    \param[in]  lednum: specify the led to be configured
      \arg        LED2
      \arg        LED3
      \arg        LED4
      \arg        LED5
    \param[out] none
    \retval     none
*/
void  gd_eval_led_init (led_typedef_enum lednum)
{
    /* enable the led clock */
    rcu_periph_clock_enable(GPIO_CLK[lednum]);
    //rcu_periph_clock_enable(RCU_GPIOA);
    /* configure led GPIO port */ 
    gpio_init(GPIO_PORT[lednum], GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,GPIO_PIN[lednum]);
	//gpio_init(GPIOA,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_9);
    //GPIO_BC(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}

/*!
    \brief      turn on selected led
    \param[in]  lednum: specify the led to be turned on
      \arg        LED2
      \arg        LED3
      \arg        LED4
      \arg        LED5
    \param[out] none
    \retval     none
*/
void gd_eval_led_on(led_typedef_enum lednum)
{
    GPIO_BOP(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
	//GPIO_BOP(RCU_GPIOA) = GPIO_PIN_9;
}

//void gd_IO_on(led_typedef_enum lednum)
//{
 //   GPIO_BOP(GPIOB) = GPIO_PIN_3;
	//GPIO_BOP(RCU_GPIOA) = GPIO_PIN_9;
//}

//void gd_IO_off(led_typedef_enum lednum)
//{
   // GPIO_BC(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
	//GPIO_BC(RCU_GPIOA) = GPIO_PIN_9;
//}

/*!
    \brief      turn off selected led
    \param[in]  lednum: specify the led to be turned off
      \arg        LED2
      \arg        LED3
      \arg        LED4
      \arg        LED5
    \param[out] none
    \retval     none
*/
void gd_eval_led_off(led_typedef_enum lednum)
{
    GPIO_BC(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
	//GPIO_BC(RCU_GPIOA) = GPIO_PIN_9;
}

/*!
    \brief      toggle selected led
    \param[in]  lednum: specify the led to be toggled
      \arg        LED2
      \arg        LED3
      \arg        LED4
      \arg        LED5
    \param[out] none
    \retval     none
*/
void gd_eval_led_toggle(led_typedef_enum lednum)
{
    gpio_bit_write(GPIO_PORT[lednum], GPIO_PIN[lednum], 
                    (bit_status)(1-gpio_input_bit_get(GPIO_PORT[lednum], GPIO_PIN[lednum])));
}

/*!
    \brief      configure key
    \param[in]  key_num: specify the key to be configured
      \arg        KEY_TAMPER: tamper key
      \arg        KEY_WAKEUP: wakeup key
      \arg        KEY_USER1: user key1
      \arg        KEY_USER2: user key2
    \param[in]  key_mode: specify button mode
      \arg        KEY_MODE_GPIO: key will be used as simple IO
      \arg        KEY_MODE_EXTI: key will be connected to EXTI line with interrupt
    \param[out] none
    \retval     none
*/
void gd_eval_key_init(key_typedef_enum key_num, keymode_typedef_enum key_mode)
{
    /* enable the key clock */
    rcu_periph_clock_enable(KEY_CLK[key_num]);
    rcu_periph_clock_enable(RCU_AF);

    /* configure button pin as input */
    gpio_init(KEY_PORT[key_num], GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, KEY_PIN[key_num]);

    if (key_mode == KEY_MODE_EXTI) {
        /* enable and set key EXTI interrupt to the lowest priority */
        nvic_irq_enable(KEY_IRQn[key_num], 2U, 0U);

        /* connect key EXTI line to key GPIO pin */
        gpio_exti_source_select(KEY_PORT_SOURCE[key_num], KEY_PIN_SOURCE[key_num]);

        /* configure key EXTI line */
        exti_init(KEY_EXTI_LINE[key_num], EXTI_INTERRUPT, EXTI_TRIG_FALLING);
        exti_interrupt_flag_clear(KEY_EXTI_LINE[key_num]);
    }
}

/*!
    \brief      return the selected key state
    \param[in]  key: specify the key to be checked
      \arg        KEY_TAMPER: tamper key
      \arg        KEY_WAKEUP: wakeup key
      \arg        KEY_USER1: user key
      \arg        KEY_USER2: user key2
    \param[out] none
    \retval     the key's GPIO pin value
*/
uint8_t gd_eval_key_state_get(key_typedef_enum key)
{
    return gpio_input_bit_get(KEY_PORT[key], KEY_PIN[key]);
}

/*!
    \brief      configure COM port
    \param[in]  com: COM on the board
      \arg        EVAL_COM1: COM1 on the board
      \arg        EVAL_COM2: COM2 on the board
    \param[out] none
    \retval     none
*/
void gd_eval_com_init(uint32_t com)
{
    uint32_t com_id = 0U;
    if(EVAL_COM1 == com){
        com_id = 0U;
    }else if(EVAL_COM2 == com){
        com_id = 1U;
    }
    
    /* enable GPIO clock */
    rcu_periph_clock_enable(COM_GPIO_CLK[com_id]);

    /* enable USART clock */
    rcu_periph_clock_enable(COM_CLK[com_id]);

    /* connect port to USARTx_Tx */
    gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, COM_TX_PIN[com_id]);

    /* connect port to USARTx_Rx */
    gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, COM_RX_PIN[com_id]);

    /* USART configure */
    usart_deinit(com);
    usart_baudrate_set(com, 115200U);
    usart_receive_config(com, USART_RECEIVE_ENABLE);
    usart_transmit_config(com, USART_TRANSMIT_ENABLE);
    usart_enable(com);
}
//-------------------------------------------------------------------------------------------------------------------------------------------
 void iic_io_init(void)
  {
	 /* enable the led clock */
	 rcu_periph_clock_enable(RCU_GPIOB);
	 gpio_init(GPIOB, GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ,GPIO_PIN_9);
	 
	 gpio_init(GPIOB, GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ,GPIO_PIN_8);	 
	 gpio_bit_set(GPIOB, GPIO_PIN_8);
	 gpio_bit_set(GPIOB, GPIO_PIN_9);
 }


void APTTouchStart(void)//------IIC起始信号，SCL为高电平期间，SDA由高到低产生一个下降沿
{
 I2C_SDA_OUT();//------------------------把SDA设置成输出，可发送数据

 IIC_SDA(1);//--SDA拉高
 //delay_1us();
 IIC_SCL(1);//--SCL拉高
 delay_us_x(DELAY_IIC_US);//--scl高电平的持续时间不能短与5us
 IIC_SDA(0);//--SDA拉低
 delay_us_x(DELAY_IIC_US);
 IIC_SCL(0);//--SCL拉低
 delay_us_x(DELAY_IIC_US);
}

void APTTouchStop(void)//------IIC停止信号，SCL为高电平期间，SDA由低到高产生一个上升沿
{
 
 IIC_SDA(0);//--SDA拉低
 delay_us_x(DELAY_IIC_US);
 IIC_SCL(1);//--SCL拉高
 delay_us_x(DELAY_IIC_US);
 IIC_SDA(1);//--SDA拉高
 delay_us_x(DELAY_IIC_US);
 IIC_SCL(0);//--通讯完后scl拉低
 //20190416--delay_1us();
 IIC_SDA(0);//--通讯完后sda拉低
}

void  APTTouchWaitAck(void)//--主机等待从机应答"0"
{
 unsigned char WaitTimeCnt;
 I2C_SDA_IN();//-------------------------SDA引脚配置成输入
 IIC_SCL(1);//------------------------------SCL拉高，读取SDA信号,保持5us
 WaitTimeCnt = 7;//WaitTimeCnt=10;
do{
	//__wait_nop();
   delay_us_x(DELAY_IIC_US);
   WaitTimeCnt--;
   }while(I2C_SDA_READ&&WaitTimeCnt);//--等待从机应答信号，如果为"0",表示从机收到了信号
 IIC_SCL(0);//-------------------------------SCL拉低保持信号
 I2C_SDA_OUT();//------------------------把SDA设置成输出，可发送数据
}

void APTTouchWriteData(volatile unsigned char WrData)//---IIC写入一个字节，SCL低的时候允许数据变化，SCL高的时候从机读取数据
{
	unsigned char	WrDataCnt;
	
	for(WrDataCnt=0;WrDataCnt<8;WrDataCnt++)
	{
		if(WrData & 0x80)//-------------主机先写高字节
			IIC_SDA(1);
		else
			IIC_SDA(0);

		 delay_us_x(DELAY_IIC_US);//delay_1us();////delay_1us();
		IIC_SCL(1);//------------------SCL拉高让从机读取数据
		//delay_1us();
		 delay_us_x(DELAY_IIC_US);
		IIC_SCL(0);//-------------------SCL拉低允许主机改变数据
		 delay_us_x(DELAY_IIC_US);
		//__wait_nop();//delay_1us();
		WrData=WrData<<1;
	}
}

unsigned char APTTouchReadData(void)		//从iic里读出数据
{
 unsigned char	RdDataCnt,RdData;
 I2C_SDA_IN();//-----------------------SDA引脚配置成输入
 RdData=0x00;
 for(RdDataCnt=0;RdDataCnt<8;RdDataCnt++)
   {
	RdData=RdData<<1;
	IIC_SCL(1);//-------------------------SCL拉高，读取数据
	delay_us_x(DELAY_IIC_US);
	if(I2C_SDA_READ)
	RdData=RdData|0x01;
	IIC_SCL(0);//-------------------------SCL拉低，允许SDA变化
	delay_us_x(DELAY_IIC_US);
   }
 I2C_SDA_OUT();//---------------------一个字节读完SDA引脚配置成输出

  IIC_SDA(1);
	  delay_us_x(DELAY_IIC_US);

 IIC_SCL(1);
	delay_us_x(DELAY_IIC_US);

 IIC_SCL(0);
 return(RdData);
}

void ht16c22_write_command(unsigned char command,unsigned char data)
{
 APTTouchStart();
 APTTouchWriteData(PartAdd);//--AC
 APTTouchWaitAck();
 APTTouchWriteData(command);//--地址
 APTTouchWaitAck();
 APTTouchWriteData(data);//--值
 APTTouchWaitAck();
 APTTouchStop();
// WDT_Reset();
}
//-------------------读一个字节
unsigned char ht1382_receuve_IT(unsigned char devAddr)
{
	unsigned char readdata=00;
 
	APTTouchStart();
	APTTouchWriteData(0xD0);
	APTTouchWaitAck();

	APTTouchWriteData(devAddr);
	APTTouchWaitAck();

	APTTouchStop();
	APTTouchStart();

	APTTouchWriteData(0xD1);
	APTTouchWaitAck();

	readdata= APTTouchReadData();

	APTTouchStop();


	return readdata;
}

void ht16c22_write_data(unsigned char addr,  volatile unsigned char data[],unsigned char len)
{
 unsigned char i;

 APTTouchStart();
 APTTouchWriteData(PartAdd);//--AC
 APTTouchWaitAck();
 APTTouchWriteData(0x80);//--写命令
 APTTouchWaitAck();
 APTTouchWriteData(addr);//--写地址
 APTTouchWaitAck();
 for(i = 0;i<len;i++){
	 APTTouchWriteData(data[i]);//--值
	 APTTouchWaitAck();
 }
 APTTouchStop();
 //WDT_Reset();
}

void ht1382_transmit_IT( unsigned char data[], unsigned char len)
{
 uint8_t i;

 APTTouchStart();
 for(i = 0;i<len;i++){
	 APTTouchWriteData(data[i]);//--值
	 APTTouchWaitAck();
 }
 APTTouchStop();
// WDT_Reset();
}

//{
 //unsigned char i;

// APTTouchStart();
// for(i = 0;i<len;i++){
//	 APTTouchWriteData(data[i]);//--值
//	 APTTouchWaitAck();
// }
// APTTouchStop();
// WDT_Reset();
//}
//void Soft_I2C_Init(void)
//{
    //rcu_periph_clock_enable(RCU_GPIOB);	/* 使能GPIOB时钟 */

	/* 配置 IIC_SCL --> PB6 引脚为推挽输出 */ 
    //gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
	/* 配置 IIC_SDA --> PB7 引脚为推挽输出 */ 
   // gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

  //  gpio_bit_set(GPIOB, GPIO_PIN_8);
//	gpio_bit_set(GPIOB, GPIO_PIN_9);
//}



//--------------------------------------------------------------------------


