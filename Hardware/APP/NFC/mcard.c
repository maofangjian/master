#include "mcard.h"
#include "fm175xx.h"
#include "rtc.h"
#include "start.h"
#include "netmessage.h"

TaskHandle_t Mcard_TaskHandler;
/****************************************************************************************/
/*名称：TypeA_Request																	*/
/*功能：TypeA_Request卡片寻卡															*/
/*输入：																				*/
/*       			    			     												*/
/*	       								 												*/
/*输出：																			 	*/
/*	       	pTagType[0],pTagType[1] =ATQA                                         		*/
/*       	OK: 应答正确                                                              	*/
/*	 		FM17520_ERROR: 应答错误																*/
/****************************************************************************************/
unsigned char TypeA_Request(unsigned char *pTagType) 
{
    unsigned char   result, send_buff[1], rece_buff[2];
    unsigned int   rece_bitlen = 0;
    
    Clear_BitMask(TxModeReg, 0x80);//关闭TX CRC
    Clear_BitMask(RxModeReg, 0x80);//关闭RX CRC
    Set_BitMask(RxModeReg, 0x08);//关闭位接收
    Write_Reg(BitFramingReg, 0x07);
    send_buff[0] = 0x26;
    Pcd_SetTimer(1);

    result = Pcd_Comm(Transceive, send_buff, 1, rece_buff, &rece_bitlen);  //发送REQA进行轮询
    //PrintfData("TypeA_Request", rece_buff, 16);
    if ((result == OK) && (rece_bitlen == 2 * 8)) 
    {
        *pTagType = rece_buff[0];
        *(pTagType + 1) = rece_buff[1];
        return OK;
    }
    return FM17520_ERROR;
}

/****************************************************************************************/
/*名称：TypeA_Anticollision																*/
/*功能：TypeA_Anticollision卡片防冲突													*/
/*输入：selcode =0x93，0x95，0x97														*/
/*       			    			     												*/
/*	       								 												*/
/*输出：																			 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID                            		*/
/*       	OK: 应答正确                                                              	*/
/*	 		FM17520_ERROR: 应答错误																*/
/****************************************************************************************/
unsigned char TypeA_Anticollision(unsigned char selcode, unsigned char *pSnr) {
    unsigned char   result, send_buff[7], rece_buff[5], CollPos, NVB, Row, Col, Bit_Framing, Send_Len, Snr[5], i, Rece_Len;
    unsigned int   rece_bitlen = 0;

    memset(rece_buff, 0x00, 5);
    memset(Snr, 0x00, 5);//临时UID
    memset(pSnr, 0x00, 5);
    NVB = 0x20;//NVB初始值
    Bit_Framing = 0x00;//Bit_Framing初始值

    Row = 0;//行，0为无冲突，bit1~bit8
    Col = 0;//列，0为无冲突，1~4
    CollPos = 0;
    //4行8列，共32个位置，对应CollPos数值 0~31
    Clear_BitMask(TxModeReg, 0x80);
    Clear_BitMask(RxModeReg, 0x80);

    Write_Reg(CollReg, 0x80);//冲突位后的接收数据为0

    Write_Reg(MfRxReg, 0x00);//使能奇偶校验


    while (1) {
        send_buff[0] = selcode;

        send_buff[1] = NVB;

        Write_Reg(BitFramingReg, Bit_Framing);//设置发送、接收的bit个数

        memcpy(send_buff + 2, pSnr, Col);

        Send_Len = Col + 2;//设置发送字节数	

        Pcd_SetTimer(1);

        result = Pcd_Comm(Transceive, send_buff, Send_Len, rece_buff, &rece_bitlen);
        //PrintfData("quqian101TypeA_Anticollision", rece_buff, 16);
        if (result == TIMEOUT_Err) {
            return FM17520_ERROR;
        }


        if (result & 0x08)//CollErr
        {
            //发现冲突错误
            CollPos = Read_Reg(CollReg) & 0x1F;//读取冲突位置

            TypeA_Set_NVB(CollPos, &NVB, &Row, &Col);//计算下次交互需要的NVB，保存本次收到的bit行数、列数

            TypeA_Set_Bit_Framing(CollPos, &Bit_Framing);//计算下次交互需要发送、接收的bit个数

            memcpy(Snr, rece_buff, Col);//保存接收到的部分UID到Snr

            Snr[Col - 1] = Snr[Col - 1] & Row;//保存接收到的正确bit

            for (i = 0; i < Col; i++) {
                pSnr[i] = pSnr[i] | Snr[i];//组合Snr与pSnr，保存到pSnr
            }
        } else {
            //没有发现冲突
            if (rece_bitlen > 8)
                Rece_Len = rece_bitlen / 8;
            else
                Rece_Len = 1;

            if (Col == 0) {
                //上次交互无冲突
                if ((Rece_Len + Col) != 5) 
				{
                    return FM17520_ERROR;
                }
                memcpy(pSnr, rece_buff, Rece_Len);//保存rece buff到 pSnr
            } else {
                //上次交互有冲突	
                if ((Rece_Len + Col) != 6) {

                    return FM17520_ERROR;
                }


                memcpy(Snr + Col - 1, rece_buff, Rece_Len);//保存rece buff到 Snr

                for (i = 0; i < Rece_Len; i++) {
                    pSnr[i + Col - 1] = pSnr[i + Col - 1] | Snr[i + Col - 1];//组合Snr与pSnr，保存到pSnr
                }
            }
            if (pSnr[4] == (pSnr[0] ^ pSnr[1] ^ pSnr[2] ^ pSnr[3])) {
                return OK;
            } else {
                return FM17520_ERROR;
            }
        }

    }
   // return result;
}


void TypeA_Set_NVB(unsigned char collpos, unsigned char *nvb, unsigned char *row, unsigned char *col) {
    if (collpos == 0) {
        *nvb = 0x58;
        *col = 4;
    }
    if ((collpos > 0) && (collpos < 9)) //1~8
    {
        *nvb = 0x20 + (collpos); //21~28
        *col = 1;
    }
    if ((collpos > 8) && (collpos < 17))//9~16
    {
        *nvb = 0x30 + (collpos - 8);//31~38
        *col = 2;
    }
    if ((collpos > 16) && (collpos < 25))//17~24
    {
        *nvb = 0x40 + (collpos - 16);//41~48
        *col = 3;
    }
    if ((collpos > 24) && (collpos < 32))//25~31
    {
        *nvb = 0x50 + (collpos - 24);//51~57
        *col = 4;
    }

    switch ((*nvb) & 0x07) {
        case 1:
            *row = 0x01;
            break;
        case 2:
            *row = 0x03;
            break;
        case 3:
            *row = 0x07;
            break;
        case 4:
            *row = 0x0F;
            break;
        case 5:
            *row = 0x1F;
            break;
        case 6:
            *row = 0x3F;
            break;
        case 7:
            *row = 0x7F;
            break;
        case 8:
            *row = 0xFF;
            break;
    }
    return;
}

void TypeA_Set_Bit_Framing(unsigned char collpos, unsigned char *bit_framing) {
    if (collpos == 0) {
        *bit_framing = 0x00;
        return;
    }
    if ((collpos > 0) && (collpos < 9)) //1~8
    {
        *bit_framing = ((collpos) << 4) + (collpos);
        return;
    }
    if ((collpos > 8) && (collpos < 17))//9~16
    {
        *bit_framing = (((collpos - 8)) << 4) + (collpos - 8);
        return;
    }
    if ((collpos > 16) && (collpos < 25))//17~24
    {
        *bit_framing = (((collpos - 16)) << 4) + (collpos - 16);

        return;
    }
    if ((collpos > 24) && (collpos < 32))//25~31
    {
        *bit_framing = (((collpos - 24)) << 4) + (collpos - 24);
        return;
    }
    return;
}


/****************************************************************************************/
/*名称：TypeA_Select																	*/
/*功能：TypeA_Select卡片选卡															*/
/*输入：selcode =0x93，0x95，0x97														*/
/*      pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 			    			     	*/
/*	       								 												*/
/*输出：																			 	*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/
/*       	OK: 应答正确                                                              	*/
/*	 		FM17520_ERROR: 应答错误																*/
/****************************************************************************************/
unsigned char TypeA_Select(unsigned char selcode, unsigned char *pSnr, unsigned char *pSak) {
    unsigned char   result, i, send_buff[7], rece_buff[5];
    unsigned int   rece_bitlen;
    Write_Reg(BitFramingReg, 0x00);
    Set_BitMask(TxModeReg, 0x80);
    Set_BitMask(RxModeReg, 0x80);

    send_buff[0] = selcode;
    send_buff[1] = 0x70;

    for (i = 0; i < 5; i++) {
        send_buff[i + 2] = *(pSnr + i);
    }

    Pcd_SetTimer(1);
    result = Pcd_Comm(Transceive, send_buff, 7, rece_buff, &rece_bitlen);
    //PrintfData("TypeA_Select", rece_buff, 16);
    if (result == OK) {
        *pSak = rece_buff[0];
    }
    return result;
}
/****************************************************************************************/
/*名称：TypeA_Halt																		*/
/*功能：TypeA_Halt卡片停止																*/
/*输入：																				*/
/*       			    			     												*/
/*	       								 												*/
/*输出：																			 	*/
/*	       											                            		*/
/*       	OK: 应答正确                                                              	*/
/*	 		FM17520_ERROR: 应答错误																*/
/****************************************************************************************/
unsigned char TypeA_Halt(void) {
    unsigned char   result, send_buff[2], rece_buff[1];
    unsigned int   rece_bitlen;
    send_buff[0] = 0x50;
    send_buff[1] = 0x00;

    Write_Reg(BitFramingReg, 0x00);
    Set_BitMask(TxModeReg, 0x80);
    Set_BitMask(RxModeReg, 0x80);
    Clear_BitMask(Status2Reg, 0x08);
    Pcd_SetTimer(1);
    result = Pcd_Comm(Transceive, send_buff, 2, rece_buff, &rece_bitlen);
    return result;
}
/****************************************************************************************/
/*名称：TypeA_CardActivate																*/
/*功能：TypeA_CardActivate卡片激活														*/
/*输入：																				*/
/*       			    			     												*/
/*	       								 												*/
/*输出：	pTagType[0],pTagType[1] =ATQA 											 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 		                   		*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/
/*       	OK: 应答正确                                                              	*/
/*	 		FM17520_ERROR: 应答错误																*/
/****************************************************************************************/
unsigned char TypeA_CardActivate(unsigned char *pTagType, unsigned char *pSnr, unsigned char *pSak) 
{
    uint8_t result;

    result = TypeA_Request(pTagType);//寻卡 Standard	 send request command Standard mode
    
    //PrintfData("pTagType:", pTagType, 2);
    if (result != OK) 
    {
    	//LOG("FM17520_ERROR.\n");
        return FM17520_ERROR;
    }
    if (pTagType[1] != 0x00) 
	{
		//LOG("FM17520_ERROR.\n");
        return FM17520_ERROR;
    }
    if ((pTagType[0] & 0xC0) == 0x00)   //M1卡
    {
        result = TypeA_Anticollision(0x93, pSnr);//防冲突验证
        if (result != OK) {
            return FM17520_ERROR;
        }
        result = TypeA_Select(0x93, pSnr, pSak);//选卡并继续防冲突验证
        if (result != OK) {
            return FM17520_ERROR;
        }
    }
    return result;
}


/*****************************************************************************************/
/*名称：Mifare_Auth																		 */
/*功能：Mifare_Auth卡片认证																 */
/*输入：mode，认证模式（0：key A认证，1：key B认证）；sector，认证的扇区号（0~15）		 */
/*		*mifare_key，6字节认证密钥数组；*card_uid，4字节卡片UID数组						 */
/*输出:																					 */
/*		OK    :认证成功																	 */
/*		FM17520_ERROR :认证失败																  	 */
/*****************************************************************************************/
 unsigned char Mifare_Auth(unsigned char mode,unsigned char sector,unsigned char *mifare_key,unsigned char *card_uid)
{
	unsigned char   send_buff[12],rece_buff[1],result;
	unsigned int   rece_bitlen;
	if(mode==0x0)
		send_buff[0]=0x60;//kayA认证
	if(mode==0x1)
		send_buff[0]=0x61;//keyB认证
  	send_buff[1]=sector*4;
	send_buff[2]=mifare_key[0];
	send_buff[3]=mifare_key[1];
	send_buff[4]=mifare_key[2];
	send_buff[5]=mifare_key[3];
	send_buff[6]=mifare_key[4];
	send_buff[7]=mifare_key[5];
	send_buff[8]=card_uid[0];
	send_buff[9]=card_uid[1];
	send_buff[10]=card_uid[2];
	send_buff[11]=card_uid[3];

	Pcd_SetTimer(1);
	Clear_FIFO();
	result =Pcd_Comm(MFAuthent,send_buff,12,rece_buff,&rece_bitlen);//Authent认证
    //PrintfData("Mifare_Auth", rece_buff, 16);

	if (result==OK)
    {
        if(Read_Reg(Status2Reg) & 0x08)//判断加密标志位，确认认证结果
            return OK;
        else
            return FM17520_ERROR;
    }
	return FM17520_ERROR;
}
/*****************************************************************************************/
/*名称：Mifare_Blockset																	 */
/*功能：Mifare_Blockset卡片数值设置														 */
/*输入：block，块号；*buff，需要设置的4字节数值数组										 */
/*																						 */
/*输出:																					 */
/*		OK    :设置成功																	 */
/*		FM17520_ERROR :设置失败																	 */
/*****************************************************************************************/
 unsigned char Mifare_Blockset(unsigned char block,unsigned char *buff)
 {
  unsigned char   block_data[16],result;
	block_data[0]=buff[3];
	block_data[1]=buff[2];
	block_data[2]=buff[1];
	block_data[3]=buff[0];
	block_data[4]=~buff[3];
	block_data[5]=~buff[2];
	block_data[6]=~buff[1];
	block_data[7]=~buff[0];
   	block_data[8]=buff[3];
	block_data[9]=buff[2];
	block_data[10]=buff[1];
	block_data[11]=buff[0];
	block_data[12]=block;
	block_data[13]=~block;
	block_data[14]=block;
	block_data[15]=~block;
  result= Mifare_Blockwrite(block,block_data);
  return result;
 }

/*****************************************************************************************/
/*名称：Mifare_Blockread																 */
/*功能：Mifare_Blockread卡片读块操作													 */
/*输入：block，块号（0x00~0x3F）；buff，16字节读块数据数组								 */
/*输出:																					 */
/*		OK    :成功																		 */
/*		FM17520_ERROR :失败																		 */
/*****************************************************************************************/
unsigned char Mifare_Blockread(unsigned char block,unsigned char *buff)
{	
	unsigned char   send_buff[2],result;
	unsigned int   rece_bitlen;
	Pcd_SetTimer(1);
	send_buff[0]=0x30;//30 读块
	send_buff[1]=block;//块地址
	Clear_FIFO();
	result =Pcd_Comm(Transceive,send_buff,2,buff,&rece_bitlen);//
 //   printf("rece_bitlen: %d ,result = %d \n", rece_bitlen, result);
	if ((result!=OK )||(rece_bitlen!=16*8)) //接收到的数据长度为16
	{
		return FM17520_ERROR;
	}

	return OK;
}

/*****************************************************************************************/
/*名称：mifare_blockwrite																 */
/*功能：Mifare卡片写块操作																 */
/*输入：block，块号（0x00~0x3F）；buff，16字节写块数据数组								 */
/*输出:																					 */
/*		OK    :成功																		 */
/*		FM17520_ERROR :失败																		 */
/*****************************************************************************************/
unsigned char Mifare_Blockwrite(unsigned char block,unsigned char *buff)
{
	unsigned char  send_buff[16],rece_buff[16];
	unsigned int   rece_bitlen;
	Pcd_SetTimer(1);
	send_buff[0] = 0xa0;//a0 写块
	send_buff[1] = block;//块地址

	Pcd_Comm(Transceive, send_buff, 2, rece_buff, &rece_bitlen);//
 //   LOG("rece_bitlen: %d ,result = %d ,rece_buff[0] = %d\n", rece_bitlen, result, rece_buff[0]);
	
	if ((rece_bitlen != 4) | ((rece_buff[0] & 0x0F)!=0x0A)) 	//如果未接收到4bit 1010，表示无ACK
    {
        LOG("Mifare_Blockwrite 错误1:\n");
        return(FM17520_ERROR);
    }
	
	Pcd_SetTimer(5);
	Pcd_Comm(Transceive, buff, 16, rece_buff, &rece_bitlen);//
 //   LOG("qa rece_bitlen: %d ,result = %d ,rece_buff[0] = %d\n", rece_bitlen, result, rece_buff[0]);
    if ((rece_bitlen != 4)|((rece_buff[0] & 0x0F)!=0x0A)) 	//如果未接收到4bit 1010，表示无ACK
    {
        LOG("Mifare_Blockwrite 错误2:\n");
        return(FM17520_ERROR);
    }
	
	return OK;
}
/*****************************************************************************************/
/*名称：																				 */
/*功能：Mifare 卡片增值操作																 */
/*输入：block，块号（0x00~0x3F）；buff，4字节增值数据数组								 */
/*输出:																					 */
/*		OK    :成功																		 */
/*		FM17520_ERROR :失败																		 */
/*****************************************************************************************/

unsigned char Mifare_Blockinc(unsigned char block,unsigned char *buff)
{	
	unsigned char   result,send_buff[2],rece_buff[1];
	unsigned int   rece_bitlen;
	Pcd_SetTimer(5);
	send_buff[0]=0xc1;//
	send_buff[1]=block;//块地址
	Clear_FIFO();
	result=Pcd_Comm(Transceive,send_buff,2,rece_buff,&rece_bitlen);
    //PrintfData("Mifare_Blockinc1", rece_buff, 16);
	if ((result!=OK )|((rece_buff[0]&0x0F)!=0x0A))	//如果未接收到0x0A，表示无ACK
		return FM17520_ERROR;
	Pcd_SetTimer(5);
	Clear_FIFO();
	Pcd_Comm(Transceive,buff,4,rece_buff,&rece_bitlen);
    //PrintfData("Mifare_Blockinc2", rece_buff, 16);
	return result;
}

/*****************************************************************************************/
/*名称：mifare_blockdec																	 */
/*功能：Mifare 卡片减值操作																 */
/*输入：block，块号（0x00~0x3F）；buff，4字节减值数据数组								 */
/*输出:																					 */
/*		OK    :成功																		 */
/*		FM17520_ERROR :失败																		 */
/*****************************************************************************************/

unsigned char Mifare_Blockdec(unsigned char block,unsigned char *buff)
{	
	unsigned char   result,send_buff[2],rece_buff[1];
	unsigned int   rece_bitlen;
	Pcd_SetTimer(5);
	send_buff[0]=0xc0;//
	send_buff[1]=block;//块地址
	Clear_FIFO();
	result=Pcd_Comm(Transceive,send_buff,2,rece_buff,&rece_bitlen);
    //PrintfData("Mifare_Blockdec1", rece_buff, 16);
	if ((result!=OK )|((rece_buff[0]&0x0F)!=0x0A))	//如果未接收到0x0A，表示无ACK
		return FM17520_ERROR;
	Pcd_SetTimer(5);
	Clear_FIFO();
	Pcd_Comm(Transceive,buff,4,rece_buff,&rece_bitlen);
    //PrintfData("Mifare_Blockdec2", rece_buff, 16);
	return result;
}

/*****************************************************************************************/
/*名称：mifare_transfer																	 */
/*功能：Mifare 卡片transfer操作															 */
/*输入：block，块号（0x00~0x3F）														 */
/*输出:																					 */
/*		OK    :成功																		 */
/*		FM17520_ERROR :失败																		 */
/*****************************************************************************************/

unsigned char Mifare_Transfer(unsigned char block)
{	
	unsigned char    result,send_buff[2],rece_buff[1];
	unsigned int    rece_bitlen;
	Pcd_SetTimer(5);
	send_buff[0]=0xb0;//
	send_buff[1]=block;//块地址
	Clear_FIFO();
	result=Pcd_Comm(Transceive,send_buff,2,rece_buff,&rece_bitlen);
    //PrintfData("Mifare_Transfer", rece_buff, 16);
	if ((result!=OK )|((rece_buff[0]&0x0F)!=0x0A))	//如果未接收到0x0A，表示无ACK
		return FM17520_ERROR;
	return result;
}

/*****************************************************************************************/
/*名称：mifare_restore																	 */
/*功能：Mifare 卡片restore操作															 */
/*输入：block，块号（0x00~0x3F）														 */
/*输出:																					 */
/*		OK    :成功																		 */
/*		FM17520_ERROR :失败																		 */
/*****************************************************************************************/

unsigned char Mifare_Restore(unsigned char block)
{	
	unsigned char    result,send_buff[4],rece_buff[1];
	unsigned int    rece_bitlen;
	Pcd_SetTimer(5);
	send_buff[0]=0xc2;//
	send_buff[1]=block;//块地址
	Clear_FIFO();
	result=Pcd_Comm(Transceive,send_buff,2,rece_buff,&rece_bitlen);
    //PrintfData("Mifare_Restore1", rece_buff, 16);
	if ((result!=OK )|((rece_buff[0]&0x0F)!=0x0A))	//如果未接收到0x0A，表示无ACK
		return FM17520_ERROR;
	Pcd_SetTimer(5);
	send_buff[0]=0x00;
	send_buff[1]=0x00;
	send_buff[2]=0x00;
	send_buff[3]=0x00;
	Clear_FIFO();
	Pcd_Comm(Transceive,send_buff,4,rece_buff,&rece_bitlen);
    //PrintfData("Mifare_Restore2", rece_buff, 16);
	return result;
}



//#define AUTH_CARD_BLOCK_INDEX           49  
uint8_t KeyASeed[33] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
#define AUTH_CARD_BLOCK_INDEX           37
#define AUTH_CARD_BLOCK_CODE            39
#define AUTH_CARD_SANQU                 9
uint8_t DefaultKeyASeed[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
CHG_INFO_STR gChgInfo;
static uint8_t cardnum = 0;
static uint32_t card_oldtime = 0;
uint8_t cardwrong = 0;     //卡片： 0 有效； 1 无效
static uint8_t cardstep = 1;
static uint32_t card_id = 0;
void BswSrv_McardCheck(void)
{
    uint8_t PICC_SAK[3] = {0};
    uint8_t PICC_ATQA[2] = {0};
    uint8_t PICC_UID[32] = {0};
    uint8_t cardKeyA[32];
	uint8_t AES_UID[32] = {0};
    uint32_t card_picc_uid = 0;
    //寻卡
    if (TypeA_CardActivate(PICC_ATQA, PICC_UID, PICC_SAK) == OK) 
    {
        sprintf(cardKeyA,"%02x%02x%02x%02x",PICC_UID[0], PICC_UID[1], PICC_UID[2], PICC_UID[3]);
        //sprintf(cardKeyA,"%s","67ceb2a6");
		LOG("find card:%s  %02x%02x%02x%02x.\n",cardKeyA, PICC_UID[0], PICC_UID[1], PICC_UID[2], PICC_UID[3]);		
		memcpy(AES_UID,PICC_UID,32);
		card_picc_uid =  (PICC_UID[0]<<24) | (PICC_UID[1]<<16) | (PICC_UID[2]<<8) | (PICC_UID[3]<<0);
		if ((App_GetRtcCount() - card_oldtime) > 2)
        {
            card_oldtime = App_GetRtcCount();

			//通过keyA种子KeyASeed和物理卡号为明文加密生成该卡的秘钥keyA,即cardKeyA
			//BswSrv_SeAesEncrypt((void*)AES_UID, 16, (void*)AES, KeyASeed);
			// memcpy(cardKeyA,AES,6);
			// cardKeyA[6]= 0xFF;
			// cardKeyA[7]= 0x07;
			// cardKeyA[8]= 0x80;
			// cardKeyA[9]= 0x69;
			// memcpy((void*)&cardKeyA[10], AES,6);
			
			// //KEYA密码认证扇区9
			// if((Mifare_Auth(0, AUTH_CARD_SANQU, cardKeyA, PICC_UID) != OK))
			// {
			// 	cardwrong = 1;
			// 	LOG("card cardKeyA auth fail.\r\n");
			// 	TypeA_Halt();
			// 	return;
			// }
			// LOG("card cardKeyA auth sucess.\r\n");
			
			//读鉴权卡卡号存储的所在数据块
			// if(Mifare_Blockread(AUTH_CARD_BLOCK_INDEX, blockData) != OK)
			// {
			// 	LOG("read block data fail.\r\n");
			// 	TypeA_Halt();
			// 	return;
			// }
			// blockData[16] = '\0';
			// LOG("read block data sucess: %s.\r\n",blockData);
			
			TypeA_Halt();
				
			LOG("card McardCheckFlag:%d.\n",gChgInfo.McardCheckFlag);		
			if(gChgInfo.McardCheckFlag == MCARD_IDLE)
			{
				memcpy((void*)gChgInfo.current_usr_card_id, cardKeyA, strlen(cardKeyA));
				LOG("Mcardstring = %s.\r\n", gChgInfo.current_usr_card_id);
				card_id = card_picc_uid;
				gChgInfo.McardCheckFlag = MCARD_REQ;
			}

		}
    }
}

/**
 * @description: 刷卡任务处理
 * @Author: 
 * @Date: 2025-05-22 17:19:40
 * @return {*}
 */
void App_McardProcess(void)
{
	SystemPortInfo_t *pGunInfo = NULL;
	uint8_t i;
	
	static uint32_t McardCheckTime = 0;
	if(gChgInfo.McardCheckFlag == MCARD_REQ)   //刷卡请求
	{
		gChgInfo.current_usr_gun_id = 0;
		App_SendCardAuthReq(0); //刷卡查余额
		gChgInfo.McardCheckFlag = MCARD_REQ_WAITING;	
		McardCheckTime = App_GetRtcCount();
	}
    else if (gChgInfo.McardCheckFlag == MCARD_SENDING)
	{
		//App_CkbProtoCardCtrol(0);
		App_SendCardAuthReq(1); //刷卡鉴权
		gChgInfo.McardCheckFlag = MCARD_ACK;
	}
	

	if ((App_GetRtcCount() - McardCheckTime > 15) && (gChgInfo.McardCheckFlag >= MCARD_REQ))
	{
		gChgInfo.McardCheckFlag = MCARD_IDLE;
        App_CkbProtoCardCtrol(0);
        printf("card idle status\r\n");
	}
}

void BswSrv_McardTask(void *param)
{
    FM175XX_Init();
    Mcard_TaskHandler = xTaskGetCurrentTaskHandle(); // 获取当前任务句柄
	memset((void*)&gChgInfo, 0, sizeof(CHG_INFO_STR));
    while (1)
	{
		FM175XX_HardReset();
//        Uart_Send_Msg("-> FM175XX Reset OK\r\n");//FM175XX¸´Î»³É¹¦
        Pcd_ConfigISOType(0);
		Set_Rf(0);   
        Set_Rf(3);   
		
        if (SystemInfo.nfcCardState == 1)
        {
            BswSrv_McardCheck();
        }
        vTaskDelay(50);
    }
}
