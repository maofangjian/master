/*
 * @Author: 
 * @Date: 2025-05-12 19:35:36
 * @LastEditors: chenyuan
 * @LastEditTime: 2025-05-15 18:46:54
 * @FilePath: \13.FreeRTOS实验\Hardware\APP\VOICE\App_Voice.c
 * @Description: 
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#include "App_Voice.h"
#include "i2s.h"

/**
 * @description: 扫码充电提示语
 * @Author: 
 * @Date: 
 * @param {uint8_t} gunId
 * @return {*}
 */
void BswSrv_TtsPlayStartChgTip(uint8_t gunId)
{
    play_item[0] = gunId+2;
    play_item[1] = AUDIO_17;
}

/**
 * @description: 结束充电提示语
 * @Author: 
 * @Date: 
 * @param {uint8_t} gunId
 * @return {*}
 */
void BswSrv_TtsPlayStopChgTip(uint8_t gunId)
{
    play_item[0] = gunId+2;
    play_item[1] = AUDIO_18;
}

/**
 * @description: 刷卡余额播报
 * @Author: 
 * @Date: 
 * @param {uint8_t} money_yuan：元  money_jiao：角 money_fen：分
 * @return {*}
 */
void BswSrv_TtsPlayUserMoney(uint16_t money_yuan,uint16_t money_jiao,uint16_t money_fen)
{
    if(money_yuan <= 10)
    {
        play_item[0] = AUDIO_15;
        play_item[1] = money_yuan+2;
        play_item[2] = AUDIO_13;
        play_item[3] = money_jiao+2;
        play_item[4] = money_fen+2;
        play_item[5] = AUDIO_14;
        play_item[6] = AUDIO_19;
        
    }
    else if(money_yuan < 100)
    {
        play_item[0] = AUDIO_15;
        play_item[1] = money_yuan/10+2;
        play_item[2] = AUDIO_11;
        play_item[3] = money_yuan%10+2;
        play_item[4] = AUDIO_13;
        play_item[5] = money_jiao+2;
        play_item[6] = money_fen+2;
        play_item[7] = AUDIO_14;
        play_item[8] = AUDIO_19;
    }
    else if(money_yuan <= 1000)
    {
        play_item[0] = AUDIO_15;
        play_item[1] = money_yuan/100+2;
        play_item[2] = AUDIO_12;
        if(money_yuan%100/10 == 0)
        {
            play_item[3] = AUDIO_01;
            play_item[4] = money_yuan%10+2;
            play_item[5] = AUDIO_13;
            play_item[6] = money_jiao+2;
            play_item[7] = money_fen+2;
            play_item[8] = AUDIO_14;
            play_item[9] = AUDIO_19;
        }
        else
        {
            play_item[3] = money_yuan%100/10+2;
            play_item[4] = AUDIO_11;
            play_item[5] = money_yuan%10+2;
            play_item[6] = AUDIO_13;
            play_item[7] = money_jiao+2;
            play_item[8] = money_fen+2;
            play_item[9] = AUDIO_14;
            play_item[10] = AUDIO_19;
        }

    }
}



/**
 * @description: 卡片无效
 * @Author: 
 * @Date: 
 * @param {uint8_t} 
 * @return {*}
 */
void BswSrv_TtsPlayCardInvalid(void)
{
    play_item[0] = AUDIO_16;
}

/**
 * @description: 选择端口充电
 * @Author: 
 * @Date: 
 * @param {uint8_t} 
 * @return {*}
 */
void BswSrv_TtsSelectport(void)
{
    play_item[0] = AUDIO_19;
}



