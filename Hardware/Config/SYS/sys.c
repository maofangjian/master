/*
 * @Author: chenyuan
 * @Date: 2025-04-08 17:19:29
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-04-10 15:20:48
 * @FilePath: \13.FreeRTOS实验\Hardware\Config\SYS\sys.c
 * @Description: 
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#include "sys.h"
#include "stdio.h"
#include "global.h"
/**
 * 获取芯片唯一id
 * 12个字节 96位
 */

const uint16_t crctab[256] =
{
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

int Bsp_GetId(uint8_t *uuid)
{
	if(uuid == NULL) return FALSE;

	uint8_t id[12];
	memset(uuid, 0x30, 32);

    //获取MCU唯一ID
//    uint32_t Unique_ID1 = *(uint32_t *)(0x1FFFF7AC);        //UNIQUE_ID[31: 0]
//    uint32_t Unique_ID2 = *(uint32_t *)(0x1FFFF7B0);        //UNIQUE_ID[63:32]
//    uint32_t Unique_ID3 = *(uint32_t *)(0x1FFFF7B4);        //UNIQUE_ID[95:63]
    uint32_t Unique_ID1 = *(uint32_t *)(0x1FFFF7E8);        //UNIQUE_ID[31: 0]
    uint32_t Unique_ID2 = *(uint32_t *)(0x1FFFF7EC);        //UNIQUE_ID[63:32]
    uint32_t Unique_ID3 = *(uint32_t *)(0x1FFFF7F0);        //UNIQUE_ID[95:63]

	id[0] = (Unique_ID1 >> 24) & 0xFF;
	id[1] = (Unique_ID1 >> 16) & 0xFF;
	id[2] = (Unique_ID1 >> 8) & 0xFF;
	id[3] = (Unique_ID1) & 0xFF;

	id[4] = (Unique_ID2 >> 24) & 0xFF;
	id[5] = (Unique_ID2 >> 16) & 0xFF;
	id[6] = (Unique_ID2 >> 8) & 0xFF;
	id[7] = (Unique_ID2) & 0xFF;

	id[8] = (Unique_ID3 >> 24) & 0xFF;
	id[9] = (Unique_ID3 >> 16) & 0xFF;
	id[10] = (Unique_ID3 >> 8) & 0xFF;
	id[11] = (Unique_ID3) & 0xFF;

	Bsp_BCDToString(&uuid[8], id, 12);

	return TRUE;
}


/**
 * 累加校验
 */
uint8_t Bsp_AccCheckSum(uint8_t *data, uint16_t len)
{
    uint8_t sum = 0;

    for(uint16_t i = 0; i < len; i++)
    {
        sum += data[i];
    }

    return sum;
}

/**
 * 异或校验
 */
uint8_t Bsp_XorCheckSum(uint8_t *data,uint16_t len)
{
    uint8_t sum = 0;

    for(uint16_t i = 0; i < len; i++)
    {
        sum ^= data[i];
    }

    return sum;
}

/**
 * 判断数组是否为空
 */
int Bsp_isArraryEmpty(uint8_t *array, int len)
{
	for(int i = 0;i<len ; i++){
		if(array[i] != 0x00)
        {
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * ASCII码转化为整形
 */
unsigned char Bsp_Asc2Int(char ch)
{
    unsigned char val = 0;
    if ((ch >= '0') && (ch <= '9')) {
        val = ch - '0';
    } else if ((ch >= 'A') && (ch <= 'F')) {
        val = ch - 'A' + 10;
    } else if ((ch >= 'a') && (ch <= 'f')) {
        val = ch - 'a' + 10;
    }
    return val;
}

/**
 * 字符串转化BCD码
 */
int Bsp_StringToBCD(unsigned char *BCD, const char *str)
{
    unsigned char chh, chl;
    int length = strlen(str);
    int index = 0;

    for (index = 0; index < length; index += 2) {
        chh = Bsp_Asc2Int(str[index]);
        chl = Bsp_Asc2Int(str[index + 1]);

        BCD[index / 2] = (chh << 4) | chl;
    }
    return (length / 2);
}

/**
 * BCD码转化字符串
 */
uint8_t *Bsp_BCDToString(uint8_t *dest, unsigned char *BCD, int bytes)
{
    char  temp[] = "0123456789ABCDEF";
    int index = 0;
    int length = 0;
    if (BCD == NULL || bytes <= 0)
        return NULL;

    for (index = 0; index < bytes; index++) {
        dest[length++] = temp[(BCD[index] >> 4) & 0x0F];
        dest[length++] = temp[BCD[index] & 0x0F];
    }
    dest[length] = '\0';
    return dest;
}

/**
 * 交换变量
 */
void Bsp_Swap(int *a, int *b)    
{  
    int temp;  
  
    temp = *a;  
    *a = *b;  
    *b = temp;  
  
    return ;  
}
/**
 * 快速排序
 */
void Bsp_Quicksort(int array[], int maxlen, int begin, int end)  
{  
    int i, j;  
  
    if(begin < end)  
    {  
        i = begin + 1;  
        j = end;        
            
        while(i < j)  
        {  
            if(array[i] > array[begin])  
            {  
                Bsp_Swap(&array[i], &array[j]);  
                j--;  
            }  
            else  
            {  
                i++;  
            }  
        }  
        if(array[i] >= array[begin])  
        {  
            i--;  
        }  
        Bsp_Swap(&array[begin], &array[i]);  
          
        Bsp_Quicksort(array, maxlen, begin, i);  
        Bsp_Quicksort(array, maxlen, j, end);  
    }  
}  

// 0x30 -> 0; a -> 0x0a; A -> 0x0a
uint8_t Bsp_Val(uint8_t ch)
{
    uint8_t val = (uint8_t)-1;

    if ((ch >= '0') && (ch <='9')) {
        val = ch - '0';
        return val;
    }
    if ((ch >= 'A') && (ch <='F')) {
        val = ch - 'A' + 10;
        return val;
    }
    if ((ch >= 'a') && (ch <='f')) {
        val = ch - 'a' + 10;
        return val;
    }
    return val;
}

// 0x12  <= {0x31,0x32}   0x1210 <= {0x31,0x32,0x31} 0x1210 <= {0x31,0x32,0x31,0x30}
// srcLen : pbSrc的长度
int Bsp_StrToHex(uint8_t *pbDest, const char *pbSrc, int srcLen)
{
    int32_t i=0,j=0;
    uint8_t chl,chh;

    memset(pbDest, 0, srcLen/2);
    while (i < srcLen) {
        if ((i+1) == srcLen) {  //最后一个,是奇数
            chl = Bsp_Val(pbSrc[i]);
            pbDest[j] = chl<<4;
            j ++;
        }else{
            chh = Bsp_Val(pbSrc[i]);
            chl = Bsp_Val(pbSrc[i+1]);
            pbDest[j] = (chh << 4) | chl;
            j ++;
        }
        i = i+2;
    }
    return j;
}


uint16_t CRC16(uint8_t *data,uint32_t size,uint16_t *crc )
{
	int tmp = 0;
	int i;
	for(i=0;i<size;i++)
	{
		tmp=(*crc>>8)^data[i];
		*crc=(*crc<<8)^crctab[tmp];
	}
	return 0;
}



void BswSrv_hexStrToChar(const uint8_t *hexStr, uint8_t *output) 
{
    int len = strlen(hexStr);
    for (int i = 0; i < len; i += 2) {
        unsigned int val;
        sscanf(hexStr + i, "%2x", &val); // 解析两个字符为十六进制数
        output[i/2] = val;
    }
    output[len/2] = '\0'; // 结束符
}