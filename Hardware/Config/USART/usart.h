/*** 
 * @Author: chenyuan
 * @Date: 2025-04-02 18:05:49
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-06-25 20:30:53
 * @FilePath: \FreeRtos实验0621\13.FreeRTOS实验\Hardware\Config\USART\usart.h
 * @Description: 
 * @
 * @Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#ifndef USART_H
#define USART_H

#ifdef cplusplus
 extern "C" {
#endif

#include "gd32f30x.h"
#include "fifo.h"

#define COMn                             5U
typedef enum{
    M_4G_UART=0,
    BL0939_UART,
    DEBUG_UART,
    DIS_UART,
    CKB_UART,
}Uart_Def;
#define EC200S_BUFF_SIZE               1024
#define UART0_BUF_SIZE                 1024
#define UART1_BUF_SIZE                 128
#define UART3_BUF_SIZE                 128
#define UART4_BUF_SIZE                 256
#define UART2_BUF_SIZE                 128

#define YA_COM0                        USART0
#define YA_COM0_CLK                    RCU_USART0
#define YA_COM0_TX_PIN                 GPIO_PIN_9
#define YA_COM0_RX_PIN                 GPIO_PIN_10
#define YA_COM0_GPIO_PORT              GPIOA
#define YA_COM0_GPIO_CLK               RCU_GPIOA


#define YA_COM1                        USART1
#define YA_COM1_CLK                    RCU_USART1
#define YA_COM1_TX_PIN                 GPIO_PIN_5
#define YA_COM1_RX_PIN                 GPIO_PIN_6
#define YA_COM1_GPIO_PORT              GPIOD
#define YA_COM1_GPIO_CLK               RCU_GPIOD

#define YA_COM4                        UART4
#define YA_COM4_CLK                    RCU_UART4
#define YA_COM4_TX_PIN                 GPIO_PIN_12
#define YA_COM4_RX_PIN                 GPIO_PIN_2
#define YA_COM4_GPIO_PORT              GPIOC
#define YA_COM4_GPIO_CLK               RCU_GPIOC

#define YA_COM3                        UART3
#define YA_COM3_CLK                    RCU_UART3
#define YA_COM3_TX_PIN                 GPIO_PIN_10
#define YA_COM3_RX_PIN                 GPIO_PIN_11
#define YA_COM3_GPIO_PORT              GPIOC
#define YA_COM3_GPIO_CLK               RCU_GPIOC


#define YA_COM2                        USART2
#define YA_COM2_CLK                    RCU_USART2
#define YA_COM2_TX_PIN                 GPIO_PIN_8
#define YA_COM2_RX_PIN                 GPIO_PIN_9
#define YA_COM2_GPIO_PORT              GPIOD
#define YA_COM2_GPIO_CLK               RCU_GPIOD
//DEBUG���Կ���
#define __DEBUG
#ifdef __DEBUG
#define DEBUG_PRINTF(format,...) printf("[%s]"format"\r\n", __func__, ##__VA_ARGS__)
#define DEBUG_PRINTF_ERROR(format,...) printf("FILE: "__FILE__", LINE: %d:"format"\r\n", __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_PRINTF(format,...)
#define DEBUG_PRINTF_ERROR(format,...)
#endif
extern FIFO_S_t Uart_Fifo[COMn];
extern FIFO_S_t Ec200_Fifo;
uint8_t App_UartGetOneData(int portindex, uint8_t *data);
void App_UartFlushFifo(int portindex);
void YA_ComInit(uint32_t com);
void YA_uart_init(void);
void Bsp_UartSendData(uint8_t uart_port,uint8_t *data, uint16_t length);
#ifdef cplusplus
}
#endif

#endif /* USART_H */
