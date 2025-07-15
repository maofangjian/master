#ifndef __BSWDRV_CARD_H__
#define __BSWDRV_CARD_H__

#include "global.h"
#include "fm175xx.h"


#pragma pack(1)

/*鉴权卡储存块结构体*/
typedef struct {
	uint8_t  cardStatu;             //卡使用标志,0x00–正常 0x01-锁定 0x02-冻结
                                    //正常：卡片可以正常使用
                                    //锁定：锁定中，有未结算的订单
                                    //冻结：卡片不可使用（黑名单、注销等）
    uint8_t  chargePileNum[6];      //充电桩号 低位在前，高位在后 BCD 码
    uint32_t cardBalance;         	//卡余额 单位：分,低位在前，高位在后 ，有符号 16 进制数
    uint32_t recentlyUseTime;       //最近使用时间 UNIX 时间戳
    uint8_t  cs;                    //校验和
}AUTH_CARD_SECTOR12_BLOCK0_INFO;

typedef struct {
	uint8_t  cardType;          	//0x11-支付卡 0x12-鉴权卡 0x13-未知
    uint8_t  cardId[8];       	    //用户卡号，低位在前，高位在后 BCD 码
    uint8_t  depositFlag;      	    //0x01-有押金,0x00-无押金
    uint32_t validTime;          	//有效期,UNIX 时间戳
    uint8_t  userPwEnFlag;          //是否启用用户密码标志             0x01 启用， 0x00 不启用
    uint8_t  cs;           	        //校验和
}AUTH_CARD_SECTOR12_BLOCK1_INFO;


typedef enum {
	MCARD_IDLE = 0,
	MCARD_REQ = 1,
	MCARD_REQ_WAITING = 2,
	MCARD_REQ_ACK = 3,
	MCARD_CHANGER = 4,
	MCARD_SENDING = 5,
	MCARD_ACK = 6,
}MCARD_STEP;

typedef struct {
	uint8_t  sendAuthCnt;                       //发送刷卡鉴权次数
	uint8_t  current_usr_gun_id;                //1~12
    uint8_t  mode;                              //充电模式
	uint8_t  current_usr_card_id[17];
	uint8_t  McardCheckFlag;
	uint32_t McardCheckTime;
	
	int32_t  result;
    int32_t  userMomey;                //用户卡余额 从刷卡鉴权应答得到  分
}CHG_INFO_STR;


#pragma pack()

extern CHG_INFO_STR gChgInfo;
extern uint8_t cardwrong; 
extern TaskHandle_t Mcard_TaskHandler;

extern unsigned char TypeA_Request(unsigned char *pTagType);
extern unsigned char TypeA_WakeUp(unsigned char *pTagType);
extern unsigned char TypeA_Anticollision(unsigned char selcode,unsigned char *pSnr);
extern unsigned char TypeA_Select(unsigned char selcode,unsigned char *pSnr,unsigned char *pSak);
extern unsigned char TypeA_Halt(void);
extern unsigned char TypeA_CardActivate(unsigned char *pTagType,unsigned char *pSnr,unsigned char *pSak);
void TypeA_Set_NVB(unsigned char collpos,unsigned char *nvb,unsigned char *row,unsigned char *col);
void TypeA_Set_Bit_Framing(unsigned char collpos,unsigned char *bit_framing);

extern unsigned char Mifare_Transfer(unsigned char block);
extern unsigned char Mifare_Restore(unsigned char block);
extern unsigned char Mifare_Blockset(unsigned char block,unsigned char *buff);
extern unsigned char Mifare_Blockinc(unsigned char block,unsigned char *buff);
extern unsigned char Mifare_Blockdec(unsigned char block,unsigned char *buff);
extern unsigned char Mifare_Blockwrite(unsigned char block,unsigned char *buff);
extern unsigned char Mifare_Blockread(unsigned char block,unsigned char *buff);
extern unsigned char Mifare_Auth(unsigned char mode,unsigned char sector,unsigned char *mifare_key,unsigned char *card_uid);

void App_McardProcess(void);
void BswSrv_McardTask(void *param);
#endif   //__BSWDRV_CARD_H__

