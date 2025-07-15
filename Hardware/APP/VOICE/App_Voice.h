#ifndef _APP_VOICE_H_
#define _APP_VOICE_H_

#include "global.h"

void BswSrv_TtsPlayStartChgTip(uint8_t gunId);
void BswSrv_TtsPlayStopChgTip(uint8_t gunId);
void BswSrv_TtsPlayUserMoney(uint16_t money_yuan,uint16_t money_jiao,uint16_t money_fen);
void BswSrv_TtsPlayCardInvalid(void);
void BswSrv_TtsSelectport(void);

#endif
