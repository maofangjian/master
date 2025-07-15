#ifndef __BSW_COM_DRV_EC200S_H__
#define __BSW_COM_DRV_EC200S_H__

#include "global.h"



void BswDrv_Ec200sPowerOn(void);
void BswDrv_Ec200sPowerOff(void);
void BswDrv_Ec200sUartSend(uint8_t *data, uint32_t len);
int32_t BswDrv_Ec200s_GetCharFromFifo(uint8_t *ch);
void BswDrv_Ec200s_FifoFlush(void);
void BswDrv_Ec200sGetCell(void);

#endif
