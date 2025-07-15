#include "usart.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "semphr.h"
SemaphoreHandle_t uartMutex[COMn];
static uint32_t COM_TX_PORT[COMn] = {USART0,UART3,UART4,USART1,USART2};
static uint32_t COM_PORT[COMn] = {USART0,UART3,UART4,USART1,USART2};
static rcu_periph_enum COM_CLK[COMn] = {YA_COM0_CLK,YA_COM3_CLK, YA_COM4_CLK, YA_COM1_CLK,YA_COM2_CLK};
static uint32_t COM_TX_PIN[COMn] = {YA_COM0_TX_PIN,YA_COM3_TX_PIN, YA_COM4_TX_PIN, YA_COM1_TX_PIN,YA_COM2_TX_PIN};
static uint32_t COM_RX_PIN[COMn] = {YA_COM0_RX_PIN,YA_COM3_RX_PIN, YA_COM4_RX_PIN, YA_COM1_RX_PIN,YA_COM2_TX_PIN};
static uint32_t COM_GPIO_PORT[COMn] = {YA_COM0_GPIO_PORT,YA_COM3_GPIO_PORT, YA_COM4_GPIO_PORT, YA_COM1_GPIO_PORT,YA_COM2_GPIO_PORT};
static rcu_periph_enum COM_GPIO_CLK[COMn] = {YA_COM0_GPIO_CLK,YA_COM3_GPIO_CLK, YA_COM4_GPIO_CLK, YA_COM1_GPIO_CLK,YA_COM2_GPIO_CLK};

static uint32_t COM_BAUDRATE[COMn] = {115200,4800, 115200, 115200,115200};
uint8_t Ec200sBuffer[EC200S_BUFF_SIZE] = {0};
uint8_t UART0_RxBuff[UART0_BUF_SIZE] = {0};			//ec200
uint8_t UART1_RxBuff[UART1_BUF_SIZE] = {0};			//灯板通信
uint8_t UART3_RxBuff[UART3_BUF_SIZE] = {0};			//计量芯片
uint8_t UART4_RxBuff[UART4_BUF_SIZE] = {0};			//日志串口
FIFO_S_t Uart_Fifo[COMn];
FIFO_S_t Ec200_Fifo;
//串口发送函数
void Bsp_UartSendData(uint8_t uart_port,uint8_t *data, uint16_t length)
{
    uint16_t i = 0;
    if((xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) && (uartMutex[uart_port] != NULL))//系统已经运行
    {
      xSemaphoreTake(uartMutex[uart_port],1000);
    }
    for(i = 0; i < length; i++)
    {
        usart_data_transmit(COM_TX_PORT[uart_port], (uint8_t)data[i]);
        while(RESET == usart_flag_get(COM_TX_PORT[uart_port], USART_FLAG_TBE));
    }
    if((xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) && (uartMutex[uart_port] != NULL))//系统已经运行
    {
        xSemaphoreGive(uartMutex[uart_port]);	
    }
    
}
/*!
    \brief      configure COM port
    \param[in]  com: COM on the board
      \arg        EVAL_COM1: COM1 on the board
      \arg        EVAL_COM2: COM2 on the board
    \param[out] none
    \retval     none
*/
void YA_ComInit(uint32_t com)
{
    uint32_t com_id = 0U;
    if(M_4G_UART == com){
        com_id = 0U;
				nvic_irq_enable(USART0_IRQn, 1, 0);
    }else if(DEBUG_UART == com){
        com_id = 2U;
				nvic_irq_enable(UART4_IRQn, 1, 3);
    }else if(BL0939_UART == com){
				com_id = 1U;
        nvic_irq_enable(UART3_IRQn, 1, 2);
		}
    else if(DIS_UART == com){
				com_id = 3U;
        nvic_irq_enable(USART1_IRQn, 2, 0);
		}
    else if(CKB_UART == com){
				com_id = 4U;
        nvic_irq_enable(USART2_IRQn, 2, 1);
		}
    uartMutex[com_id] = xSemaphoreCreateMutex();
    /* enable GPIO clock */
    rcu_periph_clock_enable(COM_GPIO_CLK[com_id]);
    /* enable USART clock */
    rcu_periph_clock_enable(COM_CLK[com_id]);

    if(com == DEBUG_UART)
    {
      rcu_periph_clock_enable(RCU_GPIOD);
      rcu_periph_clock_enable(RCU_GPIOC);
      gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, COM_TX_PIN[com_id]);
      gpio_init(GPIOD, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, COM_RX_PIN[com_id]);
    }
    else if (com == CKB_UART)
    {
      rcu_periph_clock_enable(RCU_AF);
      rcu_periph_clock_enable(RCU_GPIOD);
      gpio_pin_remap_config(GPIO_USART2_FULL_REMAP, ENABLE);  //USART2重映射
      gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, COM_TX_PIN[com_id]);
    }
    else
    {
      if (DIS_UART == com)
      {
        rcu_periph_clock_enable(RCU_AF);
        gpio_pin_remap_config(GPIO_USART1_REMAP, ENABLE);  //USART1重映射
      }
      
      /* connect port to USARTx_Tx */
      gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, COM_TX_PIN[com_id]);

      /* connect port to USARTx_Rx */
      gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, COM_RX_PIN[com_id]);
    }

    /* USART configure */
    usart_deinit(COM_PORT[com]);
    usart_baudrate_set(COM_PORT[com], COM_BAUDRATE[com_id]);
		usart_parity_config(COM_PORT[com], USART_PM_NONE);             //无奇偶校验
		usart_word_length_set(COM_PORT[com], USART_WL_8BIT);           //8位数字位
    if (com == BL0939_UART)
    {
      usart_stop_bit_set(COM_PORT[com], USART_STB_1_5BIT);             //1位停止位
    }
    else
    {
      usart_stop_bit_set(COM_PORT[com], USART_STB_1BIT);             //1位停止位
    }
    
    usart_transmit_config(COM_PORT[com], USART_TRANSMIT_ENABLE);
    if (com !=CKB_UART)
    {
      usart_receive_config(COM_PORT[com], USART_RECEIVE_ENABLE);
      usart_interrupt_enable(COM_PORT[com], USART_INT_RBNE);
    }
    

		
    switch(com)
    {
        case M_4G_UART:
          FIFO_S_Init(&Ec200_Fifo,Ec200sBuffer,sizeof(Ec200sBuffer));
          FIFO_S_Init(&Uart_Fifo[com],UART0_RxBuff,sizeof(UART0_RxBuff));
          FIFO_S_Flush(&Uart_Fifo[com]);
          break;
        case DEBUG_UART:
          FIFO_S_Init(&Uart_Fifo[com],UART4_RxBuff,sizeof(UART4_RxBuff));
          FIFO_S_Flush(&Uart_Fifo[com]);
          break;
        case BL0939_UART:
          FIFO_S_Init(&Uart_Fifo[com],UART3_RxBuff,sizeof(UART3_RxBuff));
          FIFO_S_Flush(&Uart_Fifo[com]);
          break;
        case DIS_UART:
          FIFO_S_Init(&Uart_Fifo[com],UART1_RxBuff,sizeof(UART1_RxBuff));
          FIFO_S_Flush(&Uart_Fifo[com]);
          break;
    }
    
    usart_enable(COM_PORT[com]);
    if (DIS_UART != com)
    {
      usart_interrupt_flag_clear(COM_PORT[com], USART_INT_FLAG_RBNE);
    }
    
}

uint8_t App_UartGetOneData(int portindex, uint8_t *data)
{
    return FIFO_S_Get(&Uart_Fifo[portindex],data);
}


void App_UartFlushFifo(int portindex)
{
  FIFO_S_Flush(&Uart_Fifo[portindex]);
}

static __IO uint8_t  PrintBuff[200];   //最大心跳报文长度
static volatile uint16_t Writecnt = 0;
void Bsp_DebugSend(uint8_t *pData, uint16_t len)
{
  uint16_t i = 0;
	if((xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) && (uartMutex[CKB_UART] != NULL))//系统已经运行
	{
		xSemaphoreTake(uartMutex[CKB_UART],1000);
	}
    for(i = 0; i < len; i++)
    {
        usart_data_transmit(COM_TX_PORT[CKB_UART], (uint8_t)pData[i]);
        while(RESET == usart_flag_get(COM_TX_PORT[CKB_UART], USART_FLAG_TBE));
    }
  	if((xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) && (uartMutex[CKB_UART] != NULL))//系统已经运行
	{
        xSemaphoreGive(uartMutex[CKB_UART]);	
	}
}

/* retarget the C library printf function to the USART */
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

//debug通信
void UART4_IRQHandler(void)
{
    uint8_t data = 0;
    if(RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_RBNE)){
			data = usart_data_receive(UART4);
      FIFO_S_Put(&Uart_Fifo[DEBUG_UART], data);
      usart_interrupt_flag_clear(UART4, USART_INT_FLAG_RBNE);
    }
}

//计量芯片模组通信
void UART3_IRQHandler(void)
{
    uint8_t data = 0;
    if(RESET != usart_interrupt_flag_get(UART3, USART_INT_FLAG_RBNE)){
			data = usart_data_receive(UART3);
      FIFO_S_Put(&Uart_Fifo[BL0939_UART], data);
      usart_interrupt_flag_clear(UART3, USART_INT_FLAG_RBNE);
    }
}

//计量芯片模组通信
void UART2_IRQHandler(void)
{
    uint8_t data = 0;
    if(RESET != usart_interrupt_flag_get(USART2, USART_INT_FLAG_RBNE)){
      usart_interrupt_flag_clear(USART2, USART_INT_FLAG_RBNE);
    }
}

//串口4g模组通信
void USART0_IRQHandler(void)
{
  uint8_t data = 0;
  if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)){
      data = usart_data_receive(USART0);
      FIFO_S_Put(&Uart_Fifo[M_4G_UART], data);
      FIFO_S_Put(&Ec200_Fifo, data);
      usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE);
  }
}


//灯板串口通信
void USART1_IRQHandler(void)
{
  uint8_t data = 0;
  if(RESET != usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE)){
      data = usart_data_receive(USART1);
      FIFO_S_Put(&Uart_Fifo[DIS_UART], data);
      usart_interrupt_flag_clear(USART1, USART_INT_FLAG_RBNE);
  }
}
void YA_uart_init(void)
{
    YA_ComInit(M_4G_UART);  //4G通信串口初始化
    YA_ComInit(BL0939_UART);  //BL0939_UART串口初始化
    YA_ComInit(DEBUG_UART); //DEBUG串口初始化
    YA_ComInit(DIS_UART); //DEBUG串口初始化 
    YA_ComInit(CKB_UART); //DEBUG串口初始化 
}

