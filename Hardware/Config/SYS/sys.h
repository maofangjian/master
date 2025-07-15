#ifndef _SYS_H_
#define _SYS_H_
#include "gd32f30x.h"
#include <string.h>
int  Bsp_GetId(uint8_t *uuid);

uint8_t Bsp_AccCheckSum(uint8_t *data,uint16_t len);
uint8_t Bsp_XorCheckSum(uint8_t *data,uint16_t len);
int Bsp_isArraryEmpty(uint8_t *array,int len);
int Bsp_StringToBCD(unsigned char *BCD, const char *str);
uint8_t *Bsp_BCDToString(uint8_t *dest, unsigned char *BCD, int bytes) ;
void Bsp_Quicksort(int array[], int maxlen, int begin, int end);
void Bsp_Swap(int *a, int *b);   
int  Bsp_StrToHex(uint8_t *pbDest, const char *pbSrc, int srcLen);
unsigned char Bsp_Asc2Int(char ch);
uint16_t CRC16(uint8_t *data,uint32_t size,uint16_t *crc );
void BswSrv_hexStrToChar(const uint8_t *hexStr, uint8_t *output) ;
#endif
