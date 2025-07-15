#ifndef _MERTER_H_
#define _MERTER_H_
#include "gd32f30x.h"

//只读寄存器  名称   地址          功能                     默认值
#define IA_FAST_RMS 0x00 //A 通道快速有效值，无符号        0x000000
#define IA_WAVE     0x01 //A 通道电流波形寄存器，有符号      0x000000
#define IB_WAVE     0x02//B 通道电流波形寄存器，有符号       0x000000
#define V_WAVE      0x03//电压波形寄存器，有符号           0x000000
#define IA_RMS      0x04//A 通道电流有效值寄存器，无符号      0x000000
#define IB_RMS      0x05//B 通道电流有效值寄存器，无符号      0x000000
#define V_RMS       0x06//电压有效值寄存器，无符号          0x000000
#define IB_FAST_RMS 0x07//B 通道快速有效值，无符号         0x000000
#define A_WATT      0x08//A 通道有功功率寄存器，有符号       0x000000
#define B_WATT      0x09//B 通道有功功率寄存器，有符号       0x000000
#define CFA_CNT     0x0A//A 通道有功电能脉冲计数，无符号      0x000000
#define CFB_CNT     0x0B//B 通道有功电能脉冲计数，无符号      0x000000
#define A_CORNER    0x0C//A 通道电流电压波形相角寄存器       0x0000      
#define B_CORNER    0x0D//B 通道电流电压波形相角寄存器       0x0000
#define TPS1        0x0E//B 通道电流电压波形相角寄存器       0x000
#define TPS2        0x0F//外部温度检测寄存器，无符号         0x000
//读写寄存器
#define A_F_RMS_CTL 0x10//A 通道快速有效值控制寄存器        0xFFFF
#define IA_RMSOS    0x13//电流 A 通道有效值小信号校正寄存器    0x00
#define IB_RMSOS    0x14//电流 B 通道有效值小信号校正寄存器    0x00
#define A_WATTOS    0x15//A 通道有功功率小信号校正寄存器      0x00
#define B_WATTOS    0x16//B 通道有功功率小信号校正寄存器      0x00
#define WA_CREEP    0x17//有功功率防潜寄存器                 0x0B
#define MODE        0x18//用户模式选择寄存器                 0x0000
#define SOFT_RESET  0x19//写入 0x5A5A5A时用户区寄存器复位      0x000000
#define USR_WRPROT  0x1A//写入 0x55后用户操作寄存器可以写入；  0x00
#define TPS_CTRL    0x1B//温度模式控制寄存器                 0x07FF
#define TPS2_A      0x1C//外部温度传感器增益系数校正寄存器      0x0000
#define TPS2_B      0x1D//外部温度传感器偏移系数校正寄存器      0x0000
#define B_F_RMS_CTL 0x1E//B 通道快速有效值控制寄存器        0xFFFF


#define BL0939_ADDR_WRITE   0xA8     //写地址
#define BL0939_ADDR_READ    0x58     //读地址


#define BL_CHANEL_MAX         6
#pragma pack(1)
typedef enum{
    BL0939_ADDR1 = 0,
    BL0939_ADDR2,
    BL0939_ADDR3,
    BL0939_ADDR4,
    BL0939_ADDR5,
    BL0939_ADDR6,
}BL0939_ADDR_DEF;


typedef union
{
    uint8_t byte_data[4];/* data */
    uint16_t int_data[2];
    uint32_t  total_data;
}FourByteDef;

typedef union
{
    uint8_t byte_data[2];/* data */
    uint16_t int_data;
}TwoByteDef;

typedef struct 
{
    uint8_t status;   //端口状态  bit 0: 过流 bit 1：过压 bit 2:短路 bit 3:保险丝故障 bit 4:设备过温
    uint16_t power;  //功率
    uint16_t volatage; //电压
    uint16_t current;   //电流
    uint32_t energy;   //电量
}Port_Status_Def;


typedef struct 
{
    uint32_t Energy_Chanel_A;   //通道A的用电量
    uint32_t Energy_Chanel_B;   //通道B的用电量
	uint32_t Fir_CFA_COUNT;		//CFA_CNT寄存器
	uint32_t Fir_CFB_COUNT;		//CFB_CNT寄存器
	uint32_t Mid_CFA_COUNT;		//A通道电能中间变量
	uint32_t Mid_CFB_COUNT;		//B通道电能中间变量
    uint32_t Power_Chanel_A;    //通道A的功率
    uint32_t Power_Chanel_B;    //通道B的功率
    uint32_t Current_Chanel_A;  //通道A的电流
    uint32_t Current_Chanel_B;  //通道B的电流
    uint32_t Voltage_Chanel_A;  //通道A的电压
    uint32_t Voltage_Chanel_B;  //通道B的电压
    uint32_t Temp_Chanel_A;     //通道A的温度
    uint32_t Temp_Chanel_B;     //通道B的温度
}Report_Dada_Def;


//全电参数数据包格式
typedef struct 
{
	uint8_t	head;                       //包头
	uint8_t Ia_FAST_RMS[3];             //电流A快速有效值
	uint8_t Ia_RMS[3];                  //电流A有效值
	uint8_t Ib_RMS[3];                  //电流B有效值
	uint8_t VRMS[3];                   //电压有效值
	uint8_t FAST_RMS[3];                //电流B快速有效值
	uint8_t AWATT[3];                  //A通道功率值
	uint8_t BWATT[3];                  //B通道功率值
	uint8_t CFACNT[3];                 //A通道脉冲计数值
	uint8_t CFBCNT[3];                 //B通道脉冲计数值
	uint8_t TPS_1[3];                    //内部温度计量值
	uint8_t TPS_2[3];                    //外部温度传感器测量值
	uint8_t	CHKSUM;                     //校验和值
}All_Data_Def;

typedef union 
{
    uint8_t all_data[35];/* data */
    All_Data_Def Flame;
}Receve_Data_Def;

#pragma pack()
extern Report_Dada_Def Report_Data[];

extern uint8_t MeterInitflag;
void Bsp_SetPortStatus(int port,Port_Status_Def *PortStatus);
void Bsp_GetPortStatus(int port,Port_Status_Def *PortStatus);
void App_Deal_Bl0939_Data(BL0939_ADDR_DEF addr);
void App_Bl0939_Init(void);
void App_Energy_Load(void);
#endif
