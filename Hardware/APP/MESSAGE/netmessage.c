#include "netmessage.h"
#include "FreeRTOS.h"
#include "global.h"
#include "task.h"
#include "rtc.h"
#include "start.h"
#include "led.h"
#include "order.h"
#include "meter.h"
#include "relay_control.h"
#include "BswAbstr_Ec200s.h"
#include "App_Voice.h"
#include "mcard.h"
#include "App_CkbProto.h"
#include "cJSON.h"
uint8_t UnpackBuf[1024] = {0};
uint8_t heart_error_times = 0;

/* 声明外部符号 */

void print_heap_status(void) 
{
    size_t block_size = 1024; // 从小块开始尝试
    size_t total_allocated = 0;
    void *blocks[32]; // 保存分配指针
    int count = 0;

    // 逐步尝试分配
    while (count < 32 && (blocks[count] = malloc(block_size)) != NULL) {
        total_allocated += block_size;
        count++;
    }

    // 立即释放所有分配的内存
    for (int i = 0; i < count; i++) {
        free(blocks[i]);
    }

    printf("Protected Check:\n");
    printf("  Max Contiguous Free: %zu bytes\n", total_allocated);
}
void App_StartUpMessage(void)
{
    cJSON *root = NULL,*data = NULL;
    SystemPortInfo_t *pGunInfo = NULL;
    char *json_body = NULL; 
    char *data_body = NULL; 
    uint32_t crc = 0;
    uint32_t time  = App_GetRtcCount();

    root = cJSON_CreateObject();
    data = cJSON_CreateObject();

    cJSON_AddItemToObject(root,"device_id",cJSON_CreateString(SystemInfo.device_id));
    cJSON_AddNumberToObject(root,"cmd",NET_CMD_START_UP);
    cJSON_AddNumberToObject(root,"timestamp",time);
    cJSON_AddItemToObject(data,"version_mcu",cJSON_CreateString(SOFT_VERSION));
    cJSON_AddItemToObject(data,"version_4g",cJSON_CreateString(SystemInfo.ec200_version));
    cJSON_AddItemToObject(data,"iccid",cJSON_CreateString(SystemInfo.sim_id));
    cJSON_AddNumberToObject(data,"login_reason",0);
    cJSON_AddNumberToObject(data,"port_num",PORTMAX);
    cJSON_AddItemToObject(root,"args",data);
    data_body = cJSON_PrintUnformatted(data);
    CRC16(data_body,strlen(data_body),&crc);
    cJSON_AddNumberToObject(root,"crc",crc);
    json_body = cJSON_PrintUnformatted(root);
    LOG("JSON data :%s\r\n",json_body);
    Abstr_Ec200sMqttPublish((void*)json_body, strlen(json_body), 1,DEVICE_REQ);
    cJSON_Delete(root);
    if (data_body != NULL)
    {
        free(data_body);
    }
    if(json_body != NULL)
    {
        free(json_body);
    }
    
    
    
}


void App_HeartBeatSendMessage(void)
{

    cJSON *root = NULL,*data = NULL,*port_info = NULL,*ntc = NULL,*port = NULL,*port_status = NULL,*fault_code = NULL,*voltage = NULL,*current = NULL,*power = NULL,*elec = NULL,*money = NULL,*order = NULL;
    SystemPortInfo_t *pGunInfo = NULL;
    char *json_body = NULL; 
    char *data_body = NULL; 
    Port_Status_Def port_data;
    uint32_t NTC[3],portnum[PORTMAX],portstatus[PORTMAX],fault[PORTMAX],voltage_data[PORTMAX],current_data[PORTMAX],power_data[PORTMAX],elec_data[PORTMAX],money_data[PORTMAX];
    char order_data[PORTMAX][ORDER_LEN+1] = {0};
    uint32_t crc = 0;
    uint32_t time  = App_GetRtcCount();
    
    LOG("heart beat send \r\n");
    root = cJSON_CreateObject();
    data = cJSON_CreateObject();
    port_info = cJSON_CreateObject();
    for (uint8_t i = 0; i < 3; i++)
    {
        NTC[i] = Report_Data[i].Temp_Chanel_A;
    }
    for (uint8_t n = 0; n < PORTMAX; n++)
    {
        pGunInfo = &SystemOrderInfo[n];
        Bsp_GetPortStatus(n,&port_data);
        portnum[n] = n+1;
        portstatus[n]  = pGunInfo->gunState;
        fault[n] = port_data.status;
        voltage_data[n] = port_data.volatage/10;
        current_data[n] = port_data.current/100;
        power_data[n] = port_data.power/10;
        elec_data[n] = pGunInfo->chargingElec;
        money_data[n] = pGunInfo->cost_money_time;
        if (pGunInfo->gunState)
        {
            memcpy(order_data[n],pGunInfo->order,ORDER_LEN);
            order_data[n][ORDER_LEN] = '\0';
        }
        else
        {
            order_data[n][0] = '\0';
        }
        
    }
    
    cJSON_AddItemToObject(root,"device_id",cJSON_CreateString(SystemInfo.device_id));
    cJSON_AddNumberToObject(root,"cmd",NET_CMD_HEARTBEAT);
    cJSON_AddNumberToObject(root,"timestamp",time);

    ntc = cJSON_CreateIntArray(NTC,3);
    port = cJSON_CreateIntArray(portnum,PORTMAX);
    port_status = cJSON_CreateIntArray(portstatus,PORTMAX);
    fault_code = cJSON_CreateIntArray(fault,PORTMAX);
    voltage = cJSON_CreateIntArray(voltage_data,PORTMAX);
    current = cJSON_CreateIntArray(current_data,PORTMAX);
    power = cJSON_CreateIntArray(power_data,PORTMAX);
    elec = cJSON_CreateIntArray(elec_data,PORTMAX);
    money = cJSON_CreateIntArray(money_data,PORTMAX);
    order = cJSON_CreateArray();
    for (int i = 0; i < PORTMAX; i++) {
    cJSON_AddItemToArray(order, cJSON_CreateString(order_data[i]));
    }
    
    cJSON_AddNumberToObject(data,"csq",SystemInfo.csq);
    cJSON_AddItemToObject(data,"ntc_value",ntc);
    cJSON_AddItemToObject(data,"version_mcu",cJSON_CreateString(SOFT_VERSION));
    cJSON_AddItemToObject(data,"version_keyboard",cJSON_CreateString("V1.0.0"));

    cJSON_AddItemToObject(port_info,"port",port);
    cJSON_AddItemToObject(port_info,"port_status",port_status);
    cJSON_AddItemToObject(port_info,"fault_code",fault_code);
    cJSON_AddItemToObject(port_info,"voltage",voltage);
    cJSON_AddItemToObject(port_info,"current",current);
    cJSON_AddItemToObject(port_info,"power",power);
    cJSON_AddItemToObject(port_info,"elec",elec);
    cJSON_AddItemToObject(port_info,"money",money);
    cJSON_AddItemToObject(port_info,"order",order);

    cJSON_AddItemToObject(data,"port_info",port_info);
    cJSON_AddItemToObject(root,"args",data);
    data_body = cJSON_PrintUnformatted(data);
    if (data_body == NULL)
    {
        goto exit;
    }
    
    CRC16(data_body,strlen(data_body),&crc);
    cJSON_AddNumberToObject(root,"crc",crc);
    json_body = cJSON_PrintUnformatted(root);
    if (json_body == NULL)
    {
        goto exit;
    }
    
    LOG("JSON data :%s\r\n",json_body);
    Abstr_Ec200sMqttPublish((void*)json_body, strlen(json_body), 1,DEVICE_REQ);
exit:
    cJSON_Delete(root);
    
    if (data_body != NULL)
    {
        free(data_body);
    }
    if(json_body != NULL)
    {
        free(json_body);
    }
    heart_error_times++;
    if(heart_error_times >= 3)
    {
        heart_error_times = 0;
        SystemInfo.onlineflag = FALSE;
        LOG("long time no recv heart beat ack,error.\n");
        AbstrEc200sInfo.ec200sState.Bits.ec200sConnectState = 0;
		Abstr_Ec200s_SendNotify();
    }
    
}

void App_SendCardAuthReq(uint8_t flag)
{
    cJSON *root = NULL,*data = NULL;
    char *json_body = NULL; 
    char *data_body = NULL; 
    uint32_t crc = 0;
    uint32_t time  = App_GetRtcCount();

    root = cJSON_CreateObject();
    data = cJSON_CreateObject();

    cJSON_AddItemToObject(root,"device_id",cJSON_CreateString(SystemInfo.device_id));
    cJSON_AddNumberToObject(root,"cmd",NET_CMD_NFC_REQ);
    cJSON_AddNumberToObject(root,"timestamp",time);
    cJSON_AddNumberToObject(data,"port",gChgInfo.current_usr_gun_id);
    cJSON_AddNumberToObject(data,"mcard_type",flag);
    cJSON_AddItemToObject(data,"card_id",cJSON_CreateString(gChgInfo.current_usr_card_id));
    cJSON_AddItemToObject(root,"args",data);
    data_body = cJSON_PrintUnformatted(data);
    CRC16(data_body,strlen(data_body),&crc);
    cJSON_AddNumberToObject(root,"crc",crc);
    json_body = cJSON_PrintUnformatted(root);
    LOG("JSON data :%s\r\n",json_body);
    Abstr_Ec200sMqttPublish((void*)json_body, strlen(json_body), 1,DEVICE_REQ);
    cJSON_Delete(root);
    if (data_body != NULL)
    {
        free(data_body);
    }
    if(json_body != NULL)
    {
        free(json_body);
    }
}

//定位上报
void App_LocationMessage(void)
{
    cJSON *root = NULL,*data = NULL,*mcc = NULL,*lac = NULL,*mnc = NULL,*cellid = NULL,*singnal = NULL;
    SystemPortInfo_t *pGunInfo = NULL;
    char *json_body = NULL; 
    char *data_body = NULL; 
    uint32_t location_mcc[3],location_mnc[3],location_lac[3],location_cellid[3],location_signal[3];
    uint32_t crc = 0;
    uint32_t time  = App_GetRtcCount();

    root = cJSON_CreateObject();
    data = cJSON_CreateObject();
    for (uint8_t i = 0; i < 3; i++)
    {
        location_mcc[i] = location[i].mcc;
        location_mnc[i] = location[i].mnc;
        location_lac[i] = location[i].tac;
        location_cellid[i] = location[i].cell_id;
        location_signal[i] = location[i].signal;
    }
    
    cJSON_AddItemToObject(root,"device_id",cJSON_CreateString(SystemInfo.device_id));
    cJSON_AddNumberToObject(root,"cmd",NET_CMD_LOCATION);
    cJSON_AddNumberToObject(root,"timestamp",time);

    mcc = cJSON_CreateIntArray(location_mcc,3);
    mnc = cJSON_CreateIntArray(location_mnc,3);
    lac = cJSON_CreateIntArray(location_lac,3);
    cellid = cJSON_CreateIntArray(location_cellid,3);
    singnal = cJSON_CreateIntArray(location_signal,3);

    cJSON_AddItemToObject(data,"mcc",mcc);
    cJSON_AddItemToObject(data,"mnc",mnc);
    cJSON_AddItemToObject(data,"lac",lac);
    cJSON_AddItemToObject(data,"cell_id",cellid);
    cJSON_AddItemToObject(data,"singnal",singnal);

    cJSON_AddItemToObject(root,"args",data);
    data_body = cJSON_PrintUnformatted(data);
    CRC16(data_body,strlen(data_body),&crc);
    cJSON_AddNumberToObject(root,"crc",crc);
    json_body = cJSON_PrintUnformatted(root);
    LOG("JSON data :%s\r\n",json_body);
    Abstr_Ec200sMqttPublish((void*)json_body, strlen(json_body), 1,DEVICE_REQ);
    cJSON_Delete(root);
    if (data_body != NULL)
    {
        free(data_body);
    }
    if(json_body != NULL)
    {
        free(json_body);
    }
    
}


void App_RemoteStartChargingAck(SystemPortInfo_t *chargeinfo,uint32_t res,uint32_t failreason)
{

    cJSON *root = NULL,*data = NULL;
    char *json_body = NULL; 
    char *data_body = NULL; 
    uint32_t crc = 0;
    uint32_t time  = App_GetRtcCount();

    root = cJSON_CreateObject();
    data = cJSON_CreateObject();

    cJSON_AddItemToObject(root,"device_id",cJSON_CreateString(SystemInfo.device_id));
    cJSON_AddNumberToObject(root,"cmd",NET_CMD_REMOTE_CHARG);
    cJSON_AddNumberToObject(root,"timestamp",time);
    cJSON_AddNumberToObject(data,"port",chargeinfo->gunId);
    cJSON_AddItemToObject(data,"order",cJSON_CreateString(chargeinfo->order));
    cJSON_AddNumberToObject(data,"result",0);
    cJSON_AddNumberToObject(data,"fail_reason",0);
    cJSON_AddItemToObject(root,"args",data);
    data_body = cJSON_PrintUnformatted(data);
    CRC16(data_body,strlen(data_body),&crc);
    cJSON_AddNumberToObject(root,"crc",crc);
    json_body = cJSON_PrintUnformatted(root);
    LOG("JSON data :%s\r\n",json_body);
    Abstr_Ec200sMqttPublish((void*)json_body, strlen(json_body), 1,DEVICE_REQ);
    cJSON_Delete(root);
    if (data_body != NULL)
    {
        free(data_body);
    }
    if(json_body != NULL)
    {
        free(json_body);
    }
    
}

void App_ChargeStartNotify(uint8_t port)
{

}

void App_ChargeStopNotify(ORDER_INFO *order)
{
    cJSON *root = NULL,*data = NULL,*power_seg_array = NULL;
    char *json_body = NULL; 
    char *data_body = NULL; 
    uint32_t crc = 0;
    uint32_t time  = App_GetRtcCount();

    root = cJSON_CreateObject();
    data = cJSON_CreateObject();

    cJSON_AddItemToObject(root,"device_id",cJSON_CreateString(SystemInfo.device_id));
    cJSON_AddNumberToObject(root,"cmd",NET_CMD_STOP_NOTIFY);
    cJSON_AddNumberToObject(root,"timestamp",time);

    cJSON_AddNumberToObject(data,"port",order->port);
    cJSON_AddNumberToObject(data,"stop_reason",order->stopreason);
    cJSON_AddItemToObject(data,"order",cJSON_CreateString(order->orderNo));
    cJSON_AddNumberToObject(data,"start_time",order->starttime);
    cJSON_AddNumberToObject(data,"stop_time",order->stoptime);
    cJSON_AddNumberToObject(data,"cost_elec",order->stopelec - order->startelec);
    cJSON_AddNumberToObject(data,"cost_money",order->ordercost);
    cJSON_AddNumberToObject(data,"charge_mode",order->chargemode);
    cJSON_AddNumberToObject(data,"charge_para",order->chargepar);
    cJSON_AddNumberToObject(data,"charge_time",order->chargetime);
    power_seg_array = cJSON_CreateIntArray(order->chargepowersegment,SEGMENT_NUM_MAX);
    cJSON_AddItemToObject(data,"charge_power_time",power_seg_array);
    cJSON_AddItemToObject(root,"args",data);
    data_body = cJSON_PrintUnformatted(data);
    CRC16(data_body,strlen(data_body),&crc);
    cJSON_AddNumberToObject(root,"crc",crc);
    json_body = cJSON_PrintUnformatted(root);
    LOG("JSON data :%s\r\n",json_body);
    Abstr_Ec200sMqttPublish((void*)json_body, strlen(json_body), 1,DEVICE_REQ);
    cJSON_Delete(root);
    if (data_body != NULL)
    {
        free(data_body);
    }
    if(json_body != NULL)
    {
        free(json_body);
    }
    
}


void App_UpgradeMessageAck(void)
{

}

void App_RemoteStopChargingAck(uint8_t port)
{
    cJSON *root = NULL,*data = NULL;
    char *json_body = NULL; 
    char *data_body = NULL; 
    uint32_t crc = 0;
    uint32_t time  = App_GetRtcCount();

    root = cJSON_CreateObject();
    data = cJSON_CreateObject();

    cJSON_AddItemToObject(root,"device_id",cJSON_CreateString(SystemInfo.device_id));
    cJSON_AddNumberToObject(root,"cmd",NET_CMD_STOP_CHARGE);
    cJSON_AddNumberToObject(root,"timestamp",time);
    cJSON_AddNumberToObject(data,"port",port);
    cJSON_AddNumberToObject(data,"result",0);
    cJSON_AddItemToObject(root,"args",data);
    data_body = cJSON_PrintUnformatted(data);
    CRC16(data_body,strlen(data_body),&crc);
    cJSON_AddNumberToObject(root,"crc",crc);
    json_body = cJSON_PrintUnformatted(root);
    LOG("JSON data :%s\r\n",json_body);
    Abstr_Ec200sMqttPublish((void*)json_body, strlen(json_body), 1,DEVICE_REQ);
    cJSON_Delete(root);
    if (data_body != NULL)
    {
        free(data_body);
    }
    if(json_body != NULL)
    {
        free(json_body);
    }
    
}



void App_RemoteStartChargingHandler(uint8_t *data,uint16_t len)
{

    cJSON *root = cJSON_Parse(data);
    Charge_Param *chargeinfo = NULL;
    cJSON *special_data = NULL,*price_element = NULL;
    SystemPortInfo_t *portinfo = NULL;
    uint8_t ret = 0;
    special_data = cJSON_GetObjectItem(root,"port");
    portinfo = &SystemOrderInfo[special_data->valueint- 1];
    memset(portinfo,0,sizeof(SystemPortInfo_t));
    portinfo->gunId = special_data->valueint;
    // ret = App_OpenOrder_Check(portinfo->gunId); //保险丝检测
    // if(ret == FAIL)
    // {
    //     App_RemoteStartChargingAck(portinfo,1,1);  //端口故障，启动失败
    //     cJSON_Delete(root); 
    //     return;
    // }
    chargeinfo = &ChargeInfo[portinfo->gunId-1];
    memset(chargeinfo,0,sizeof(Charge_Param));
    LOG("port :%d\r\n",special_data->valueint);
    special_data = cJSON_GetObjectItem(root,"money");
    if (cJSON_IsNumber(special_data))
    {
        portinfo->money = special_data->valueint;
        LOG("money :%d\r\n",portinfo->money);
    }   
    special_data = cJSON_GetObjectItem(root,"order");
    if (cJSON_IsString(special_data))
    {
        memcpy((void *)portinfo->order,special_data->valuestring,strlen(special_data->valuestring));
        LOG("order :%s\r\n",portinfo->order);
    }  
    special_data = cJSON_GetObjectItem(root,"charge_mode");
    if (cJSON_IsNumber(special_data))
    {
        portinfo->chgMode = special_data->valueint;
        LOG("chgMode :%d\r\n",portinfo->chgMode);
    } 
    special_data = cJSON_GetObjectItem(root,"charge_para");
    if (cJSON_IsNumber(special_data))
    {
        portinfo->chgpara = special_data->valueint;
        LOG("charge_para :%d\r\n",portinfo->chgpara);
    }
    special_data = cJSON_GetObjectItem(root,"power_segment_price"); 
    int num_array = cJSON_GetArraySize(special_data);
    for (uint8_t i = 0; i < num_array; i++)
    {
        price_element = cJSON_GetArrayItem(special_data,i);
        portinfo->SegmentChargingPrice[i] = price_element->valueint;
        LOG("price :%d %d\r\n",i,portinfo->SegmentChargingPrice[i]);
    }
    special_data = cJSON_GetObjectItem(root,"elec_server_price");
    if (cJSON_IsNumber(special_data))
    {
        portinfo->elec_server_price = special_data->valueint;
        LOG("elec_server_price :%d\r\n",portinfo->elec_server_price);
    }
    portinfo->startTime = App_GetRtcCount();
    portinfo->stopReason = STOP_NULL;
    portinfo->startElec = 0;
    portinfo->realChargingTime = 0;
    portinfo->temporary_elec = 0;
    portinfo->totalpower = 0;
    portinfo->gunChgStatu = PORT_CHARGING_UNKNOW_POWER;
    portinfo->gunState = POER_PLUG_IN;
    portinfo->isSync = FIRST_START;
    chargeinfo->chargetime = App_GetRtcCount();
    Port_charge_status[portinfo->gunId-1] = PORTCHARGING;
    App_CkbProtoLedCtrol(portinfo->gunId,PORTCHARGING);
    App_ChargeInit(portinfo->gunId);
    Delay_ms(100);
    App_Relay_Control(portinfo->gunId,ON);
    App_RemoteStartChargingAck(portinfo,0,0);
    App_System_Order_Save();
    BswSrv_TtsPlayStartChgTip(portinfo->gunId);
    cJSON_Delete(root); 
    
}
void App_RemoteStopChargingHandler(uint8_t *data,uint16_t len)
{
    
    cJSON *root = cJSON_Parse(data);
    Charge_Param *chargeinfo = NULL;
    uint8_t port = 0;
    SystemPortInfo_t *portinfo = NULL;
    cJSON *analysis_data = cJSON_GetObjectItem(root,"port");

    port = analysis_data->valueint;
    if(port > PORTMAX)
    {
        if (root != NULL)
        {
            cJSON_Delete(root); 
            return;
        }
        
    }
    
    portinfo = &SystemOrderInfo[port- 1];
    LOG("port :%d\r\n",port);
    chargeinfo = &ChargeInfo[port-1];

    analysis_data = cJSON_GetObjectItem(root,"order");
    if (strcmp(portinfo->order,analysis_data->valuestring) == 0)
    {
        LOG("remote stop order success\r\n");
        portinfo->stopReason = STOP_REMOTE_CONTROP;
        App_RemoteStopChargingAck(port);
    }
    
    cJSON_Delete(root); 
    
}

void App_StartUpMessageHandler(uint8_t *data,uint16_t len)
{
    cJSON *root = cJSON_Parse(data);
    cJSON *result = cJSON_GetObjectItem(root,"result");
    if (cJSON_IsNumber(result))
    {
        LOG("result :%d\r\n",result->valueint);
        if (result->valueint == 0)
        {
            SystemInfo.onlineflag = 1;
        }
        else
        {
            SystemInfo.onlineflag = 0;
        }
        
        
    }
    cJSON_Delete(root); 
}

void App_LocationMessageHandler(uint8_t *data,uint16_t len)
{
    cJSON *root = cJSON_Parse(data);
    cJSON *result = cJSON_GetObjectItem(root,"result");
    if (cJSON_IsNumber(result))
    {
        LOG("result :%d\r\n",result->valueint);
    }
    cJSON_Delete(root); 
}


void App_McardMessageHandler(uint8_t *data,uint16_t len)
{
    uint8_t card_id[17];
    uint32_t userMomey_yuan, userMomey_jiao, userMomey_fen;
    memset(card_id,0,sizeof(card_id));
    cJSON *root = cJSON_Parse(data);
    cJSON *ID = cJSON_GetObjectItem(root,"card_id");
    memcpy(card_id,ID->valuestring,strlen(ID->valuestring));
    LOG("card_id :%s\r\n",card_id);
    cJSON *card_result = cJSON_GetObjectItem(root,"card_status");
    cJSON *money = cJSON_GetObjectItem(root,"card_money");
    if (gChgInfo.McardCheckFlag == MCARD_REQ_WAITING)
    {
        if(card_result->valueint == 1)//卡片正常
        {
            userMomey_yuan = money->valueint/100;
            userMomey_jiao = (money->valueint%100)/10;
            userMomey_fen  = (money->valueint%100)%10;
            BswSrv_TtsPlayUserMoney(userMomey_yuan,userMomey_jiao,userMomey_fen);
            gChgInfo.McardCheckFlag = MCARD_REQ_ACK;
            App_CkbProtoCardCtrol(1);

        }
        else if (card_result->valueint == 2) //余额不足
        {
            gChgInfo.McardCheckFlag = MCARD_IDLE;
        }
        else if (card_result->valueint == 3) //卡片正在使用
        {
            gChgInfo.McardCheckFlag = MCARD_IDLE;
        }
        else if (card_result->valueint == 4) //卡片不支持该站点
        {
            gChgInfo.McardCheckFlag = MCARD_IDLE;
        }

    }
    cJSON_Delete(root); 
}





void App_StopChargingNotifyHandler(uint8_t *data,uint16_t len)
{
    
    cJSON *root = cJSON_Parse(data);
    Charge_Param *chargeinfo = NULL;
    uint8_t result = 0,port = 0;
    cJSON *analysis_data = cJSON_GetObjectItem(root,"port");

    port = analysis_data->valueint;
    LOG("port :%d\r\n",port);
    chargeinfo = &ChargeInfo[port-1];

    analysis_data = cJSON_GetObjectItem(root,"result");
    result = analysis_data->valueint;
    if (result == 0 && chargeinfo->send_order_id == port)
    {
        App_RemoveFirstOrder();
        App_System_Order_Save();
        LOG("remove order success\r\n");
    }
    
    cJSON_Delete(root);
     
}

void App_UpgradeAckHandler(uint8_t result)
{
    cJSON *root = NULL,*data = NULL;
    char *json_body = NULL; 
    char *data_body = NULL; 
    uint32_t crc = 0;
    uint32_t time  = App_GetRtcCount();

    root = cJSON_CreateObject();
    data = cJSON_CreateObject();

    cJSON_AddItemToObject(root,"device_id",cJSON_CreateString(SystemInfo.device_id));
    cJSON_AddNumberToObject(root,"cmd",NET_CMD_UPGRADE);
    cJSON_AddNumberToObject(root,"timestamp",time);
    cJSON_AddNumberToObject(data,"result",result);
    cJSON_AddItemToObject(root,"args",data);
    data_body = cJSON_PrintUnformatted(data);
    CRC16(data_body,strlen(data_body),&crc);
    cJSON_AddNumberToObject(root,"crc",crc);
    json_body = cJSON_PrintUnformatted(root);
    LOG("JSON data :%s\r\n",json_body);
    Abstr_Ec200sMqttPublish((void*)json_body, strlen(json_body), 1,DEVICE_REQ);
    cJSON_Delete(root);
    if (data_body != NULL)
    {
        free(data_body);
    }
    if(json_body != NULL)
    {
        free(json_body);
    }
}

void App_RemoteUpgradeHandler(uint8_t *data)
{
    cJSON *root = cJSON_Parse(data);
    cJSON *url = NULL;
    int ret = 0;
    char ota_addr[128];
    url = cJSON_GetObjectItem(root,"url");
    memcpy((void *)ota_addr,url->valuestring,strlen(url->valuestring));
    ota_addr[strlen(url->valuestring)] = '\0';
    LOG("ota url :%s\r\n",ota_addr);
    cJSON_Delete(root); 
    ret = xTaskCreate((TaskFunction_t)App_UpgradeTask,"UpgradeTask",APP_UPGRADE_TASK_STACK_SIZE,ota_addr,APP_UPGRADE_TASK_PRIORITY,NULL);

}

void App_HeartBeatHandler(uint8_t *data,uint16_t len)
{
    cJSON *root = cJSON_Parse(data);
    cJSON *result = cJSON_GetObjectItem(root,"result");
    heart_error_times = 0;
    cJSON_Delete(root); 
}

void App_RemoteControlAck(uint8_t cmd)
{
    cJSON *root = NULL,*data = NULL;
    char *json_body = NULL; 
    char *data_body = NULL; 
    uint32_t crc = 0;
    uint32_t time  = App_GetRtcCount();

    root = cJSON_CreateObject();
    data = cJSON_CreateObject();

    cJSON_AddItemToObject(root,"device_id",cJSON_CreateString(SystemInfo.device_id));
    cJSON_AddNumberToObject(root,"cmd",NET_CMD_CONTROL);
    cJSON_AddNumberToObject(root,"timestamp",time);
    cJSON_AddNumberToObject(data,"return_cmd",cmd);
    cJSON_AddNumberToObject(data,"result",0);
    cJSON_AddItemToObject(root,"args",data);
    data_body = cJSON_PrintUnformatted(data);
    CRC16(data_body,strlen(data_body),&crc);
    cJSON_AddNumberToObject(root,"crc",crc);
    json_body = cJSON_PrintUnformatted(root);
    LOG("JSON data :%s\r\n",json_body);
    Abstr_Ec200sMqttPublish((void*)json_body, strlen(json_body), 1,DEVICE_REQ);
    cJSON_Delete(root);
    if (data_body != NULL)
    {
        free(data_body);
    }
    if(json_body != NULL)
    {
        free(json_body);
    }
}


void App_RemoteControlHandler(uint8_t *data,uint16_t len)
{
    cJSON *root = cJSON_Parse(data);
    cJSON *cmd = NULL;

    cmd = cJSON_GetObjectItem(root,"control_cmd");
    switch (cmd->valueint)
    {
    case REMOTE_CONTROL_REBOOT:  //设备重启
        NVIC_SystemReset();
        /* code */
        break;
    case REMOTE_CONTROL_SETIP:
        break;
    default:
        break;
    }
    App_RemoteControlAck(cmd->valueint);
    cJSON_Delete(root); 
    
}


void App_NetMessagePro(uint8_t *data,uint8_t cmd ,uint16_t len)
{
    switch (cmd)
    {
    case NET_CMD_START_UP:// 签到
        App_StartUpMessageHandler(data,len);
        break;
    case NET_CMD_LOCATION:// 定位 
        App_LocationMessageHandler(data,len);
        break;
    case NET_CMD_NFC_REQ:
        App_McardMessageHandler(data,len);
        break;

    case NET_CMD_REMOTE_CHARG:// 远程启动充电
        App_RemoteStartChargingHandler(data,len);
        break;
    case NET_CMD_STOP_CHARGE:// 远程结束充电 
        App_RemoteStopChargingHandler(data,len);
        break;

    case NET_CMD_STOP_NOTIFY:// 充电结束通知
        App_StopChargingNotifyHandler(data,len);
        break;

    case NET_CMD_UPGRADE:// 升级
        App_RemoteUpgradeHandler(data);
        break;
    
    case NET_CMD_HEARTBEAT:// 心跳
        App_HeartBeatHandler(data,len);
        break;

    case NET_CMD_CONTROL: //远程控制
        App_RemoteControlHandler(data,len);
        break;
    default:
        break;
    }
}

