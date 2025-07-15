
#ifndef __BSW_ABSTR_EC200_H
#define __BSW_ABSTR_EC200_H


// ------------------------------------------------------------------------
// @Project Includes
#include "global.h"


//#define EC200S_USE_TCP                    1       //使用TCP
//#define USE_FTP_DOWNLOAD                  1       //使用FTP下载
//#define USE_TEST_ANTENNA					1       //天线测试

#ifdef  USE_FTP_DOWNLOAD
#define EC200S_FTP_LEN						512		//FTP下载长度
#endif

#define EC200S_HTTP_LEN						512		//HTTP下载长度

#define USE_EC200S_GPS                      1
#define ICCID_LEN                       	20		//SIM卡卡号的长度
#define GPS_LOCATION_LEN					12		//GPS定位信息的长度



typedef struct
{
	int8_t *cmd;
	int8_t *res;
	uint32_t wait;
	uint32_t nwait;
	int32_t (*process)(int8_t ok, uint8_t retry);
}ABS_EC200S_INIT_T;


typedef struct 
{
	uint8_t upgrade_flag;
	uint32_t size;
	uint16_t crc;
}Upgrade_Info;

//EC200S初始化状态
typedef enum
{
    EC200S_RESET = 0,
    EC200S_ATV1,
    EC200S_ATE,                                 
	EC200S_ATI,                                 
    EC200S_CPIN,                                
	EC200S_CCID, 
	//EC200S_CNUM,						//询问电话号码
	EC200S_GSM,							//模块IMEI
#if defined(USE_TEST_ANTENNA)
	EC200S_ATS0,
    EC200S_ATW,
#else
    EC200S_CREG,
    EC200S_CSQ,
    EC200S_COPS,
    EC200S_QNWINFO,
    EC200S_QICSGP,
    EC200S_QIDEACT,
    EC200S_QIACT,
    EC200S_QIACT_CK,
    EC200S_QMTCFG_VERSION,
    EC200S_QMTCFG_DATAFORMAT,
    EC200S_QMTCFG_VIEWMODE,
    EC200S_QMTCFG_RECVMODE,
    EC200S_QMTCONN,
    //EC200S_IPOPEN,
#endif
    EC200S_STATE_NUM
}ABS_EC200S_INIT_STATE;

//MQTT连接中的状态
enum {
    MQTT_STATE_OPEN = 0,
    MQTT_STATE_OPEN_CHK,
    MQTT_STATE_CLOSE,
    MQTT_STATE_CONNECT,
    MQTT_STATE_DISCONNECT,
    MQTT_STATE_SUB,
    MQTT_STATE_UNSUB,
    MQTT_STATE_READY,
    MQTT_STATE_FAIL,
};

typedef enum {
    QOS_LEVEL_0 = 0,        //最多发送一次
    QOS_LEVEL_1,            //最少发送一次
    QOS_LEVEL_2,            //只发送一次
}QOS_LEVEL;

//TCP socket连接中的状态
enum{
    NET_STATE_SOCKET0_OPEN=0,
    NET_STATE_READY,
    NET_STATE_FAILURE,
    NET_NULL,
};

//EC200S模块的状态
enum
{
	EC200S_TCPCONNECING   		= 0,        //连接中，无网络
	EC200S_TCPCONNECTED   		= 1,        //连接成功
	EC200S_TCPSENDDTAT			= 2,        //发送数据
	EC200S_FTPDOWN				= 3,        //ftp下载
};

//标识的EC200S模块的相关状态
typedef union
{
	uint32_t  totalState;
	struct
	{
		uint8_t ec200sPowerState			:1;			//模块开机状态
		uint8_t ec200sConnectState			:1;			//GPRS联网状态
		uint8_t ec200sFtpState				:1;			//FTP状态
		uint8_t ec200sStopFtpState			:1;			//关闭FTP下载
		uint8_t ec200sFtpDownLoadState		:3;			//FTP升级状态，0空闲状态 1:收到升级请求   2:镜像下载到本地   3:文件不存在    4:升级请求取消
		uint8_t ec200sIsFirstPowerOnState	:1;			//模块是否第一次开机
		uint8_t ec200sSendDataState			:1;			//模块发送数据状态
		
		uint8_t ec200sHttpState				:1;			//HTTP状态
		uint8_t ec200sStopHttpState			:1;			//关闭HTTP下载
		uint8_t ec200sHttpDownLoadState		:3;			//HTTP升级状态，0空闲状态 1:收到升级请求   2:镜像下载到本地   3:文件不存在    4:升级请求取消
		

	}Bits;
}ABS_EC200S_STATE;


//标识EC200S的相关信息情况
typedef struct
{
	ABS_EC200S_STATE ec200sState;								//标识ec200s的相关状态
	uint8_t ec200sIccid[ICCID_LEN+1];							//SIM卡的ICCID
	uint8_t ec200sRssi;											//信号值
	uint8_t ec200sImei[MODULE_IMEI_LEN+1];
//    int8_t TTS_Status;			//TTS当前状态：0空闲   1播放中
}ABS_EC200S_INFO;


enum
{
	EC200S_STATE_IS_POWERON 	= 0x01,		//GPRS 上电
	EC200S_STATE_IS_ONLINE		= 0x02,		//GPRS联网状态
	EC200S_STATE_IS_FTP_DOWNLOAD= 0x10,		//FTP下载固件
    EC200S_STATE_IS_HTTP_DOWNLOAD= 0x20,    //HTTP下载固件
};

enum
{
	EC200S_FTP_SUCCESS			= 0,	//FTP下载成功
	EC200S_FTP_NONE_FILE		= 1,	//FTP下载没有文件
	EC200S_FTP_PACK_HEAD_ERROR	= 2,	//FTP下载总包头解析错误
	EC200S_FTP_PACK_NAME_ERROR  = 3,	//FTP下载，固件名称错误
	EC200S_FTP_FIRMWARE_WRITE_ERROR= 4, //FTP下载，写入固件到flash错误
	EC200S_FTP_FIRMWARE_LEN_ERROR  =5,	//FTP下载，从EC200S中获取的固件长度错误
	EC200S_FTP_FIRMWARE_CHECK_ERROR=6,	//FTP下载，检验失败
	EC200S_FTP_GET_TIMEOUT		= 7,	//FTP Get数据超时
	EC200S_FTP_STOP				= 8,	//停止FTP下载
};


enum
{
	EC200S_FTP_GET_HEAD_STEP	= 0,	//FTP下载获取头部信息
	EC200S_FTP_GET_HEAD_INFO_STEP= 1,	//FTP下载获取固件信息
	EC200S_FTP_GET_FIRMWARE		= 2,	//FTP下载获取固件

};


enum
{
	EC200S_HTTP_SUCCESS			    = 0,	    //HTTP下载成功
	EC200S_HTTP_NONE_FILE		    = 1,	    //HTTP下载没有文件
	EC200S_HTTP_PACK_HEAD_ERROR	    = 2,	    //HTTP下载总包头解析错误
	EC200S_HTTP_PACK_NAME_ERROR     = 3,	    //HTTP下载，固件名称错误
	EC200S_HTTP_FIRMWARE_WRITE_ERROR= 4,        //HTTP下载，写入固件到flash错误
	EC200S_HTTP_FIRMWARE_LEN_ERROR  = 5,	    //FHTTP下载，从EC200S中获取的固件长度错误
	EC200S_HTTP_FIRMWARE_CHECK_ERROR= 6,	    //HTTP下载，检验失败
	EC200S_HTTP_GET_TIMEOUT		    = 7,		//HTTP Get数据超时
	EC200S_HTTP_STOP				= 8,	    //停止HTTP下载
    EC200S_HTTP_BUSY				= 9,	    //停止FTP下载
};


enum
{
	EC200S_HTTP_GET_HEAD_STEP	= 0,	//HTTP下载获取头部信息
	EC200S_HTTP_GET_HEAD_INFO_STEP= 1,	//HTTP下载获取固件信息
	EC200S_HTTP_GET_FIRMWARE	= 2,	//HTTP下载获取固件

};



#pragma pack(1)

typedef struct {
    uint8_t client_idx;         //mqtt客户端标识符，0~5
    uint8_t topic_pub_req[128];      //发布主题  请求
    uint8_t topic_sub_req[128];      //订阅主题  请求
    uint8_t topic_pub_res[128];      //发布主题  应答
    uint8_t topic_sub_res[128];      //订阅主题  应答
    uint8_t client_id[128];      //服务器客户端标识符
    uint8_t host_name[128];      //服务器域名
    int     port;               //端口号
    uint8_t username[128];       //用户名
    uint8_t password[128];       //密码
}MQTT_INFO_t;


//FTP获取到的固件头部信息，进行存储Flash给上层应用读取
typedef struct
{
	uint16_t updateFlag;
    uint8_t  fwVer;             //固件版本
	uint16_t checkSum;          //校验和
	uint32_t fsize;             //固件大小
    uint16_t segmentNum;        //分片的数目s
	uint16_t SuccessNum;        //升级成功次数
	uint16_t FailNum;           //升级失败次数
}UPGRADE_FW_HEAD_INFO;


//FTP下载数据，获取固件总包头信息
typedef struct
{
	uint8_t  aa;
    uint8_t  five;
    uint8_t  fwCnt;
    uint8_t  fwVer1;
}ABSTR_PACK_HEADER;

typedef struct{
    uint32_t size;
    uint16_t checkSum;
    uint8_t  name[10];
}FW_HEAD_INFO_STR;

//HTTP下载数据，获取固件总包头信息
typedef struct
{
	uint8_t  aa;
    uint8_t  five;
    uint16_t  checkSum;
	uint32_t  size;
}FW_HEAD_INFO;

typedef struct _SCELL
{
    uint32_t  mcc;
    uint32_t  mnc;
    uint32_t  tac; 
    uint32_t  cell_id;
    int       signal;      
}SCELL;

#pragma pack()
extern uint8_t upgrade_flag;
extern uint8_t Location_status;
extern SCELL location[];
extern uint32_t	location_cnt;
extern int8_t Abstr_Ec200sQcell(void);
int32_t Abstr_Ec200sQcellSendCmd(int8_t *cmd, int8_t *ack, uint16_t waittime, int32_t flag);

extern volatile ABS_EC200S_INIT_STATE AbstrEc200sGprsInitState;
extern ABS_EC200S_INFO AbstrEc200sInfo;





int32_t Abstr_Ec200sSendCmd(int8_t *cmd, int8_t *ack, uint16_t waittime, int32_t flag);

#ifdef USE_FTP_DOWNLOAD
void Abstr_Ec200sSetFtpDownStatus(uint8_t status);
uint8_t Abstr_Ec200sStatus(void);
int8_t Abstr_Ec200sFtpGetFrimware(int8_t* server, int8_t* username, int8_t* passward, int8_t* file);
void Abstr_Ec200sSetFtpStop(void);
#endif //USE_FTP_DOWNLOAD

#ifdef EC200S_USE_TCP
int32_t Abstr_Ec200sTcpSend(uint8_t* data, uint16_t len);
#endif //EC200S_USE_TCP

int32_t Abstr_Ec200sReconnect(void);
int32_t Abstr_Ec200sMqttPublish(uint8_t *data, uint16_t len, QOS_LEVEL qos,uint8_t req);

void Abstr_Ec200sTask(void);
uint8_t Abstr_Ec200s_GetState(uint8_t status);
void Abstr_Ec200s_SendNotify(void);
void Abstr_Ec200s_SendNotifyFromIsr(void);
int32_t Abstr_Ec200sCheckRes(int8_t *cmd, int8_t *res, uint16_t tmo);

extern int BswAbstr_Ec200sTtsPlay(uint8_t *text);      //语音播报
extern int8_t Abstr_Ec200sHttpGetFrimware(int8_t* url_file);
void App_UpgradeTask(void *pvParameters);
void GetCsq(int8_t *pStr);

#endif













