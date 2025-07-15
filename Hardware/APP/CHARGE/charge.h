#ifndef _CHARGE_H_
#define _CHARGE_H_

#include "gd32f30x.h"
#include "global.h"
#define CHARGE_POWER_STAGE          30 //充电功率段数
enum{
    CHARGING_TIME_MODE = 0, //固定时长充电模式
    CHARGING_ELEC_POWER_MODE = 1, //电价分离模式
    CHARGING_POWER_MODE = 2,//按功率段收取
    CHARGING_ELEC_MODE = 3, //服务费+成本都按电量算
};

typedef enum{
    PORTIDLE = 0,
    PORTCHARGING = 1,
    PORTFULL = 2,
    PORTFAULT = 3,
}PORT_STATUS;

#pragma pack(1)
//充电参数
typedef struct 
{
    uint8_t charge_power[CHARGE_POWER_STAGE];  //充电功率段
    uint32_t chargetime; //充电时间
    uint32_t starttime; //计时开始时间
    uint16_t limittime; //系统限制时间
    uint8_t  istest;
    uint16_t chargepower; //充电功率
    uint8_t  fullflag; //充满标志
    uint8_t  power_index; //功率索引
    uint8_t  check_count; //功率检测次数
    uint8_t  tempovercnt; //高温检测次数
    uint16_t chargefullpower; //充满功率阈值
    uint16_t chargefulltime; //充满时间阈值
    uint8_t pullporttimer; //拔出检测时间
    uint8_t sendchargenotifycnt;//发送充电通知次数
    uint32_t fulltime; //充满时间
    uint8_t pullchecktimes; //拔出检测
    uint32_t sendstartchargetime; //发送充电开始时间
    uint32_t sendstopchargetime; //发送结束充电时间
    uint8_t  send_order_id; //发送订单ID
}Charge_Param;
typedef enum
{
    PORT_IDLE, //枪头空闲
    POER_PLUG_IN, //枪头插入
    PORT_CHARGE, //枪头充电
}PORT_STATE;


typedef enum
{
	PORT_CHARGING_IDLE=0,    		    //空闲
    PORT_CHARGING_UNKNOW_POWER=1,        //充电中功率未检测
	PORT_CHARGING_GUN_PULL=2,		    //拔枪状态
	PORT_CHARGING_FULL=3,		        //充满状态
	PORT_CHARGING_WORK,                  //充电中
}PORT_CHARGING_STATU;

typedef struct {
    uint8_t gunId;                                  //枪头号
    PORT_STATE gunState;                             //枪头状态
    PORT_CHARGING_STATU gunChgStatu;                 /*枪头充电状态        (0:没有启动充电,非0:启动充电状态) 
                                                      1:充电中功率未检测; 2:功率小于1.5w; 3:1.5w<功率<30w; 4:30w<功率   
                                                    */
    uint8_t  maxPower;					            //充电过程中最大功率 0.1w 大于10w是10w
    uint32_t startTime;                             //订单开始时间
    uint32_t stopTime;                              //订单结束时间
    uint8_t  stopReason;                            //停止充电原因
    uint32_t realChargingTime;                      //实际充电时间
    uint32_t startElec;                             //订单开始电量
    uint16_t chargingElec;                          //累计充电电量，0.01kwh 
    
    uint8_t  userAccount[20];                       //用户账号
    uint32_t cost_money_time;                       //累计消费金额(分) 或者累计消费充电时间(分钟)    //modify by liutao
	uint8_t  order[ORDER_LEN+1];              //订单号
    uint16_t chgpara;                               //当前订单用户选择的充电参数
    
    uint8_t  chgMode;                               /* 充电类型 1:一口价 2:功率阶梯 */

    uint8_t  powerSegmentIndex;                     //功率段指示,指示当前充电功率是在哪段功率段区间
    uint8_t  isSync;                                //订单开启同步状态 0-不需要同步 1-第一次启动 2--网络重连 3--重新上电 
	
	uint8_t  chargesendcnt;                         //充电通知发送次数
	uint16_t SegmentChargingTime[SEGMENT_NUM_MAX];  //10段功率段充电时间
    uint16_t SegmentChargingPrice[SEGMENT_NUM_MAX]; //10段功率段充电价格（分）
	uint32_t total_money_time;                      //功率段总消费金额(分)
	uint32_t money;                                 //用户余额
    uint8_t  sendOrderGunId;                        //发送的订单号
    uint32_t totalpower;                            //总功率
    uint32_t elec_server_price;                     //电量服务费价格
    float    temporary_elec;
    uint16_t crc;                                   //枪头所有参数的crc校验
}SystemPortInfo_t;

enum{
    STOP_REMOTE_CONTROP = 1, //用户远程结束
    STOP_PULL_PORT,  //枪头断开
    STOP_CHARGE_FULL,   //充满停止/*  */
    STOP_NO_MONEY,  //余额不足
    STOP_OVER_CURRENT, //过流停止
    STOP_OVER_SHORT, //过流停止
    STOP_OVER_VOLTAGE, //过压停止
    STOP_LOW_VOLTAGE, //欠压停止
    STOP_OVER_TOTAL_POWER, //整桩功率过高
    STOP_OVER_TEMP, //过温停止
    STOP_FUSE_BREAK, //保险丝熔断
    STOP_OVER_12H, //充电超过12小时
    STOP_OVER_PORT_POWER, //单口功率过高
    STOP_NULL,
};

typedef struct {                          
    uint8_t  ChargeFullExitFlag;                           	//判满假充电标志: 0 无     1 假充电判断
	uint32_t FullbeginTime;                                 //假充电计时开始时间
	uint32_t BeginTimebackup;                    
}PORT_CHARGING_FULL_TO_EXIT_STR;

#pragma pack()
extern SystemPortInfo_t SystemOrderInfo[PORTMAX];
extern Charge_Param ChargeInfo[PORTMAX];
extern PORT_STATUS Port_charge_status[PORTMAX];
void App_InsertCheck(void);
void App_ChargeInit(uint8_t port);
void App_StopChargingPro(uint8_t port);
void App_Restore_Charge_State(void);
void App_ChargeTask(void *pvParameters);
void App_ChgCtrlProcNetStatus(void);
void App_ChgCtrlMoneyCount(void);
#endif


