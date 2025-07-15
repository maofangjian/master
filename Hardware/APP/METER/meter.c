#include "meter.h"
#include "usart.h"
#include "timer.h"
#include "global.h"
#include "charge.h"
#include "fifo.h"

FourByteDef Four_Data;
Receve_Data_Def All_Data[BL_CHANEL_MAX];
Report_Dada_Def Report_Data[BL_CHANEL_MAX];

Port_Status_Def Port_Status[PORTMAX];
TwoByteDef Voltage_K;				//电压转换系数
TwoByteDef Current_K;               //电流转换系数
TwoByteDef Power_K;                 //功率转换系数
TwoByteDef LeakCur_K;               
TwoByteDef Power_K_Leak;
TwoByteDef Energy_K;                //电量转换系数

uint32_t Energy_data[PORTMAX];
uint8_t MeterInitflag = 1;
uint8_t current_over[PORTMAX];
uint8_t voltage_over[PORTMAX];

void Bsp_SetPortStatus(int port,Port_Status_Def *PortStatus)
{
    memcpy(&Port_Status[port],PortStatus,sizeof(Port_Status_Def));
}

void Bsp_GetPortStatus(int port,Port_Status_Def *PortStatus)
{
    memcpy(PortStatus,&Port_Status[port],sizeof(Port_Status_Def));
}

//计量芯片返回数据读取 0：返回失败  1:返回成功
uint8_t Bsp_Bl0939_Read(BL0939_ADDR_DEF addr,uint8_t cmd)
{
    uint8_t send_addr = 0;
    uint8_t check_sum = 0;
    uint8_t read_num;
    uint8_t data[4];
    uint8_t ret = 0;
    FIFO_S_Flush(&Uart_Fifo[BL0939_UART]);
    send_addr = 0x50|addr;
    Bsp_UartSendData(BL0939_UART,&send_addr,1);
    Bsp_UartSendData(BL0939_UART,&cmd,1);
    check_sum = send_addr + cmd;

    Delay_ms(20);

    read_num = FIFO_S_CountUsed(&Uart_Fifo[BL0939_UART]);

    if(read_num > 4)
    {
        for(uint8_t n = 0; n < 4; n++)
        {
            FIFO_S_Get(&Uart_Fifo[BL0939_UART],&data[n]);
            if(n < 3)
            {
                check_sum = check_sum + data[n];
            }
            else
            {
                if(check_sum == data[3])
                {
                    ret = TRUE;
                }
                else
                {
                    ret = FALSE;
                }
            }
        }
    }
    return ret;

}



void Bsp_Bl0939_Write(BL0939_ADDR_DEF addr,uint8_t cmd)
{
    uint8_t send_count = 0,circle_count = 0;
    uint8_t send_addr;
    for(circle_count = 0; circle_count < 6;circle_count++)
    {
        send_count = 3;

        send_addr = (0x0A<<4)|addr;

        Bsp_UartSendData(BL0939_UART,&send_addr,1);

        Bsp_UartSendData(BL0939_UART,&cmd,1);

        send_addr = send_addr + cmd;

        while (send_count--)
        {
            send_addr = send_addr + Four_Data.byte_data[send_count+1];

            Bsp_UartSendData(BL0939_UART,&Four_Data.byte_data[send_count+1],1);
        }

        send_addr = ~send_addr;
        Bsp_UartSendData(BL0939_UART,&send_addr,1);

        Delay_ms(20);

        Bsp_Bl0939_Read(addr,cmd);
        LOG("%X %X %X\r\n",Uart_Fifo[BL0939_UART].pStartAddr[0],Uart_Fifo[BL0939_UART].pStartAddr[1],Uart_Fifo[BL0939_UART].pStartAddr[2]);
        if(Uart_Fifo[BL0939_UART].pStartAddr[0] == Four_Data.byte_data[3]&&Uart_Fifo[BL0939_UART].pStartAddr[1] == Four_Data.byte_data[2]&&Uart_Fifo[BL0939_UART].pStartAddr[2] == Four_Data.byte_data[1])
        {
            LOG("emu :%d write cmd :%x success\r\n",addr,cmd);
            break;
        }
        FEED_WDG();
    }
}

//获取计量芯片的数据
uint8_t Bsp_Read_Data(BL0939_ADDR_DEF addr)
{
    uint8_t send_addr = 0;
    uint8_t check_sum = 0;
    uint8_t read_num;
    uint8_t data;
    uint8_t ret = 0;
    uint8_t read_cmd = 0xAA;
    FIFO_S_Flush(&Uart_Fifo[BL0939_UART]);
    send_addr = (0x05<<4) | addr;	

    Bsp_UartSendData(BL0939_UART,&send_addr,1);
    Bsp_UartSendData(BL0939_UART,&read_cmd,1);
    check_sum = send_addr;

    Delay_ms(150);

    read_num = FIFO_S_CountUsed(&Uart_Fifo[BL0939_UART]);
    if(read_num >= 35)
    {
        for(uint8_t n = 0; n < 35; n++)
        {
            FIFO_S_Get(&Uart_Fifo[BL0939_UART],&data);
            if(n < 34)
            {
                All_Data[addr].all_data[n] = data;
                check_sum = check_sum + data;
            }
            else
            {
                check_sum = ~check_sum;
                if(check_sum == data)
                {
                    //LOG("recv data  check success\r\n ");
                    ret = TRUE;
                }
                else
                {
                    //LOG("recv data  check fail\r\n ");
                    ret = FALSE;
                }
            }
        }
    }
    return ret;
}

//解析计量芯片数据
void App_Deal_Bl0939_Data(BL0939_ADDR_DEF addr)
{
    uint8_t ret = 0;
    FourByteDef Date_tmp;
    Port_Status_Def port_data;
    static uint32_t Energy_A = 0,Energy_B = 0;
    ret = Bsp_Read_Data(addr);
    if(ret == FALSE)
    {
        LOG("read %d bl0939 data failed\r\n",addr);
        return;
    }
    Date_tmp.byte_data[3] = 0x00;
    Date_tmp.byte_data[2] = All_Data[addr].Flame.Ia_RMS[2];
    Date_tmp.byte_data[1] = All_Data[addr].Flame.Ia_RMS[1];
    Date_tmp.byte_data[0] = All_Data[addr].Flame.Ia_RMS[0];

    Date_tmp.total_data = Date_tmp.total_data*100/Current_K.int_data;
    Report_Data[addr].Current_Chanel_A = Date_tmp.total_data;  //得到电流数据A
    Date_tmp.byte_data[3] = 0x00;
    Date_tmp.byte_data[2] = All_Data[addr].Flame.Ib_RMS[2];
    Date_tmp.byte_data[1] = All_Data[addr].Flame.Ib_RMS[1];
    Date_tmp.byte_data[0] = All_Data[addr].Flame.Ib_RMS[0];
    Date_tmp.total_data = Date_tmp.total_data*100/LeakCur_K.int_data;
    Report_Data[addr].Current_Chanel_B = Date_tmp.total_data;  //得到电流数据B

    Date_tmp.byte_data[3] = 0x00;
    Date_tmp.byte_data[2] = All_Data[addr].Flame.VRMS[2];
    Date_tmp.byte_data[1] = All_Data[addr].Flame.VRMS[1];
    Date_tmp.byte_data[0] = All_Data[addr].Flame.VRMS[0];
    Date_tmp.total_data = Date_tmp.total_data*100/Voltage_K.int_data;
    Report_Data[addr].Voltage_Chanel_A = Date_tmp.total_data;  //得到电压值

    Date_tmp.byte_data[3] = 0x00;
    Date_tmp.byte_data[2] = All_Data[addr].Flame.AWATT[2];
    Date_tmp.byte_data[1] = All_Data[addr].Flame.AWATT[1];
    Date_tmp.byte_data[0] = All_Data[addr].Flame.AWATT[0]; 
    if(Date_tmp.total_data > 0x7fffff)   //无功功率
    {
        Date_tmp.total_data = 0x1000000 - Date_tmp.total_data;
    }
    Date_tmp.total_data = Date_tmp.total_data*100/Power_K.int_data;
    Report_Data[addr].Power_Chanel_A = Date_tmp.total_data;  //得到A通道功率

    Date_tmp.byte_data[3] = 0x00;
    Date_tmp.byte_data[2] = All_Data[addr].Flame.BWATT[2];
    Date_tmp.byte_data[1] = All_Data[addr].Flame.BWATT[1];
    Date_tmp.byte_data[0] = All_Data[addr].Flame.BWATT[0];   
    if(Date_tmp.total_data > 0x7fffff)   //无功功率
    {
        Date_tmp.total_data = 0x1000000 - Date_tmp.total_data;
    }
    Date_tmp.total_data = Date_tmp.total_data*100/Power_K_Leak.int_data;
    Report_Data[addr].Power_Chanel_B = Date_tmp.total_data;  //得到B通道功率

    // Date_tmp.byte_data[3] = 0x00;
    // Date_tmp.byte_data[2] = 0x00;
    // Date_tmp.byte_data[1] = All_Data[addr].Flame.CFACNT[1];
    // Date_tmp.byte_data[0] = All_Data[addr].Flame.CFACNT[0];  

    Date_tmp.byte_data[3] = 0x00;
    Date_tmp.byte_data[2] = 0x00;
    Date_tmp.byte_data[1] = All_Data[addr].Flame.TPS_1[1];
    Date_tmp.byte_data[0] = All_Data[addr].Flame.TPS_1[0];			//TPS1  内部测温值
    Date_tmp.total_data = (Date_tmp.total_data/2-32)*170/450-50;	
    Report_Data[addr].Temp_Chanel_A = Date_tmp.total_data;			//转换为实际温度值


    if(ABS((signed)(Date_tmp.total_data - Report_Data[addr].Mid_CFA_COUNT)) < 0xff0000)
    {
        Report_Data[addr].Mid_CFA_COUNT = Report_Data[addr].Mid_CFA_COUNT + ABS((signed)(Date_tmp.total_data - Report_Data[addr].Fir_CFA_COUNT))*1000;
    }
    else
    {
        Report_Data[addr].Mid_CFA_COUNT = Report_Data[addr].Mid_CFA_COUNT + (0xffffff - ABS((signed)(Date_tmp.total_data - Report_Data[addr].Fir_CFA_COUNT)))*1000;
    }
    Report_Data[addr].Fir_CFA_COUNT = Date_tmp.total_data;
    Report_Data[addr].Energy_Chanel_A += Report_Data[addr].Fir_CFA_COUNT/Energy_K.int_data; //A通道电量值计算
    if(Energy_A != 0 && (Report_Data[addr].Energy_Chanel_A - Energy_A) >= 300)
    {
        Report_Data[addr].Energy_Chanel_A = Energy_A;
    }
    Energy_A = Report_Data[addr].Energy_Chanel_A;

    Date_tmp.byte_data[3] = 0x00;
    Date_tmp.byte_data[2] = All_Data[addr].Flame.CFBCNT[2];
    Date_tmp.byte_data[1] = All_Data[addr].Flame.CFBCNT[1];
    Date_tmp.byte_data[0] = All_Data[addr].Flame.CFBCNT[0];  
    if(ABS((signed)(Date_tmp.total_data - Report_Data[addr].Mid_CFB_COUNT)) < 0xff0000)
    {
        Report_Data[addr].Mid_CFB_COUNT = Report_Data[addr].Mid_CFB_COUNT + ABS((signed)(Date_tmp.total_data - Report_Data[addr].Fir_CFB_COUNT))*1000;
    }
    else
    {
        Report_Data[addr].Mid_CFB_COUNT = Report_Data[addr].Mid_CFB_COUNT + (0xffffff - ABS((signed)(Date_tmp.total_data - Report_Data[addr].Fir_CFB_COUNT)))*1000;
    }
    Report_Data[addr].Fir_CFB_COUNT = Date_tmp.total_data;
    Report_Data[addr].Energy_Chanel_B += Report_Data[addr].Fir_CFB_COUNT/Energy_K.int_data;
    if(Energy_B != 0 && (Report_Data[addr].Energy_Chanel_B - Energy_B) >= 300)
    {
        Report_Data[addr].Energy_Chanel_B = Energy_B;
    }
    Energy_B = Report_Data[addr].Energy_Chanel_B;

    if(addr == BL0939_ADDR1)
    {
        Bsp_GetPortStatus(0,&port_data);
        port_data.volatage = Report_Data[addr].Voltage_Chanel_A/10;
        port_data.current = Report_Data[addr].Current_Chanel_A;
        port_data.power = Report_Data[addr].Power_Chanel_A;
        port_data.energy =Energy_data[0] +  Report_Data[addr].Energy_Chanel_A/10;
        Bsp_SetPortStatus(0,&port_data);
        Bsp_GetPortStatus(1,&port_data);
        port_data.volatage = Report_Data[addr].Voltage_Chanel_A/10;
        port_data.current = Report_Data[addr].Current_Chanel_B;
        port_data.power = Report_Data[addr].Power_Chanel_B;
        port_data.energy = Energy_data[1] + Report_Data[addr].Energy_Chanel_B/10;
        Bsp_SetPortStatus(1,&port_data);
    }
    else if(addr == BL0939_ADDR2)
    {
        Bsp_GetPortStatus(2,&port_data);
        port_data.volatage = Report_Data[addr].Voltage_Chanel_A/10;
        port_data.current = Report_Data[addr].Current_Chanel_A;
        port_data.power = Report_Data[addr].Power_Chanel_A;
        port_data.energy = Energy_data[2] + Report_Data[addr].Energy_Chanel_A/10;
        Bsp_SetPortStatus(2,&port_data);
        Bsp_GetPortStatus(3,&port_data);
        port_data.volatage = Report_Data[addr].Voltage_Chanel_A/10;
        port_data.current = Report_Data[addr].Current_Chanel_B;
        port_data.power = Report_Data[addr].Power_Chanel_B;
        port_data.energy = Energy_data[3] + Report_Data[addr].Energy_Chanel_B/10;
        Bsp_SetPortStatus(3,&port_data);   
    }
    else if(addr == BL0939_ADDR3)
    {
        Bsp_GetPortStatus(4,&port_data);
        port_data.volatage = Report_Data[addr].Voltage_Chanel_A/10;
        port_data.current = Report_Data[addr].Current_Chanel_A;
        port_data.power = Report_Data[addr].Power_Chanel_A;
        port_data.energy = Energy_data[4] + Report_Data[addr].Energy_Chanel_A/10;
        Bsp_SetPortStatus(4,&port_data);
        Bsp_GetPortStatus(5,&port_data);
        port_data.volatage = Report_Data[addr].Voltage_Chanel_A/10;
        port_data.current = Report_Data[addr].Current_Chanel_B;
        port_data.power = Report_Data[addr].Power_Chanel_B;
        port_data.energy = Energy_data[5] + Report_Data[addr].Energy_Chanel_B/10;
        Bsp_SetPortStatus(5,&port_data);   
    }
    else if(addr == BL0939_ADDR4)
    {
        Bsp_GetPortStatus(6,&port_data);
        port_data.volatage = Report_Data[addr].Voltage_Chanel_A/10;
        port_data.current = Report_Data[addr].Current_Chanel_A;
        port_data.power = Report_Data[addr].Power_Chanel_A;
        port_data.energy = Energy_data[6] + Report_Data[addr].Energy_Chanel_A/10;
        Bsp_SetPortStatus(6,&port_data);
        Bsp_GetPortStatus(7,&port_data);
        port_data.volatage = Report_Data[addr].Voltage_Chanel_A/10;
        port_data.current = Report_Data[addr].Current_Chanel_B;
        port_data.power = Report_Data[addr].Power_Chanel_B;
        port_data.energy = Energy_data[7] + Report_Data[addr].Energy_Chanel_B/10;
        Bsp_SetPortStatus(7,&port_data);   
    }
    else if(addr == BL0939_ADDR5)
    {
        Bsp_GetPortStatus(8,&port_data);
        port_data.volatage = Report_Data[addr].Voltage_Chanel_A/10;
        port_data.current = Report_Data[addr].Current_Chanel_A;
        port_data.power = Report_Data[addr].Power_Chanel_A;
        port_data.energy = Energy_data[9] + Report_Data[addr].Energy_Chanel_A/10;
        Bsp_SetPortStatus(8,&port_data);
        Bsp_GetPortStatus(9,&port_data);
        port_data.volatage = Report_Data[addr].Voltage_Chanel_A/10;
        port_data.current = Report_Data[addr].Current_Chanel_B;
        port_data.power = Report_Data[addr].Power_Chanel_B;
        port_data.energy = Energy_data[8] + Report_Data[addr].Energy_Chanel_B/10;
        Bsp_SetPortStatus(9,&port_data);   
    }
    else if(addr == BL0939_ADDR6)
    {
        Bsp_GetPortStatus(10,&port_data);
        port_data.volatage = Report_Data[addr].Voltage_Chanel_A/10;
        port_data.current = Report_Data[addr].Current_Chanel_A;
        port_data.power = Report_Data[addr].Power_Chanel_A;
        port_data.energy = Energy_data[11] + Report_Data[addr].Energy_Chanel_A/10;
        Bsp_SetPortStatus(10,&port_data);
        Bsp_GetPortStatus(11,&port_data);
        port_data.volatage = Report_Data[addr].Voltage_Chanel_A/10;
        port_data.current = Report_Data[addr].Current_Chanel_B;
        port_data.power = Report_Data[addr].Power_Chanel_B;
        port_data.energy = Energy_data[10] + Report_Data[addr].Energy_Chanel_B/10;
        Bsp_SetPortStatus(11,&port_data);   
    }
    for(uint8_t i = 0; i < PORTMAX; i++)
    {
        Bsp_GetPortStatus(i,&port_data);
        if(port_data.volatage > VOLTAGE_OVER)
        {
            voltage_over[i]++;
            if(voltage_over[i] > 10)
            {
                port_data.status |= 0x02;
            }
        }
        else
        {
            port_data.status &= ~0x02;
            voltage_over[i] = 0;
        }

        if(port_data.current > CURRENT_OVER)
        {
            current_over[i]++;
            if(current_over[i] > 10)
            {
                port_data.status |= 0x01;
            }
        }
        else
        {
            port_data.status &= ~0x01;
            current_over[i] = 0;
        }
        Bsp_SetPortStatus(i,&port_data);
    }

}

void App_Energy_Load(void)
{
    for(uint8_t i = 0; i < PORTMAX; i++)
    {
        Energy_data[i]  =  SystemOrderInfo[i].chargingElec;
    }
}

void App_Bl0939_Clear(uint8_t door)
{
    uint8_t addr = (door-1)/2;

    Energy_data[door-1] = 0;
    if((door - 1)%2 == 0)
    {
        Report_Data[addr].Energy_Chanel_B = 0;
        Report_Data[addr].Fir_CFB_COUNT = 0;
        Report_Data[addr].Mid_CFB_COUNT = 0;
        Report_Data[addr].Power_Chanel_B = 0;
        Report_Data[addr].Current_Chanel_B = 0;
        Report_Data[addr].Voltage_Chanel_B = 0;
    }
    else
    {
        Report_Data[addr].Energy_Chanel_A = 0;
        Report_Data[addr].Fir_CFA_COUNT = 0;
        Report_Data[addr].Mid_CFA_COUNT = 0;
        Report_Data[addr].Power_Chanel_A = 0;
        Report_Data[addr].Current_Chanel_A = 0;
        Report_Data[addr].Voltage_Chanel_A = 0;    
    }
    memset(&Report_Data[addr],0,sizeof(Report_Dada_Def));

    Four_Data.byte_data[0] = 0x00;
    Four_Data.byte_data[1] = 0x00;
    Four_Data.byte_data[2] = 0x00;
    Four_Data.byte_data[3] = 0x55;  

    Bsp_Bl0939_Write((BL0939_ADDR_DEF)addr,USR_WRPROT);//打开写保护指令

    Four_Data.byte_data[0] = 0x00;
    Four_Data.byte_data[1] = 0x00;
    Four_Data.byte_data[2] = 0x04;
    Four_Data.byte_data[3] = 0x00;  

    Bsp_Bl0939_Write((BL0939_ADDR_DEF)addr,MODE);//设置读取后清零

    if(((door - 1) % 2)  == 0)
    {
        Bsp_Bl0939_Read((BL0939_ADDR_DEF)addr,CFB_CNT);
    }
    else
    {
        Bsp_Bl0939_Read((BL0939_ADDR_DEF)addr,CFA_CNT); 
    }

    Four_Data.byte_data[0] = 0x00;
    Four_Data.byte_data[1] = 0x00;
    Four_Data.byte_data[2] = 0x00;
    Four_Data.byte_data[3] = 0x00;  
    Bsp_Bl0939_Write((BL0939_ADDR_DEF)addr,MODE);  //模式设置，关闭读后清零功能
    Four_Data.byte_data[0] = 0x00;
    Four_Data.byte_data[1] = 0x00;
    Four_Data.byte_data[2] = 0x10;
    Four_Data.byte_data[3] = 0x00;  
    Bsp_Bl0939_Write((BL0939_ADDR_DEF)addr,MODE);  //A通道漏电选择
    Four_Data.byte_data[0] = 0x00;
    Four_Data.byte_data[1] = 0x00;
    Four_Data.byte_data[2] = 0x40;
    Four_Data.byte_data[3] = 0x00;  
    Bsp_Bl0939_Write((BL0939_ADDR_DEF)addr,TPS_CTRL);  //A通道漏电选择
	Four_Data.byte_data[0] = 0x00;
	Four_Data.byte_data[1] = 0x00; 
	Four_Data.byte_data[2] = 0x49;
	Four_Data.byte_data[3] = 0x0C; 
    Bsp_Bl0939_Write((BL0939_ADDR_DEF)addr,A_F_RMS_CTL);//漏电过流检测阈值设置5A过流阈值=5*324004*0.001*1000*0.72/(1.218*512)
    Bsp_Bl0939_Write((BL0939_ADDR_DEF)addr,B_F_RMS_CTL);
    Four_Data.byte_data[0] = 0x00;
    Four_Data.byte_data[1] = 0x00;
    Four_Data.byte_data[2] = 0x00;
    Four_Data.byte_data[3] = 0x00;  
    Bsp_Bl0939_Write((BL0939_ADDR_DEF)addr,MODE);
}

void App_Bl0939_Init(void)
{
    Power_K.int_data=8276; 		
    Current_K.int_data=27486; 		
    Voltage_K.int_data=17127; 		
    LeakCur_K.int_data=27490;
    Power_K_Leak.int_data=8276;
    Energy_K.int_data=6315;	

    Four_Data.byte_data[0] = 0x00;
    Four_Data.byte_data[1] = 0x00;
    Four_Data.byte_data[2] = 0x00;
    Four_Data.byte_data[3] = 0x55;  

    Bsp_Bl0939_Write(BL0939_ADDR1,USR_WRPROT); //关闭写保护指令
    Bsp_Bl0939_Write(BL0939_ADDR2,USR_WRPROT);
    Bsp_Bl0939_Write(BL0939_ADDR3,USR_WRPROT); //关闭写保护指令
    Bsp_Bl0939_Write(BL0939_ADDR4,USR_WRPROT);
    Bsp_Bl0939_Write(BL0939_ADDR5,USR_WRPROT); //关闭写保护指令
    Bsp_Bl0939_Write(BL0939_ADDR6,USR_WRPROT);

    Four_Data.byte_data[0] = 0x00;
    Four_Data.byte_data[1] = 0x00;
    Four_Data.byte_data[2] = 0x00;
    Four_Data.byte_data[3] = 0x39;  

    Bsp_Bl0939_Write(BL0939_ADDR1,WA_CREEP);//设置有功功率防潜寄存器，用于噪声功率切除
    Bsp_Bl0939_Write(BL0939_ADDR2,WA_CREEP); 
    Bsp_Bl0939_Write(BL0939_ADDR3,WA_CREEP);//设置有功功率防潜寄存器，用于噪声功率切除
    Bsp_Bl0939_Write(BL0939_ADDR4,WA_CREEP); 
    Bsp_Bl0939_Write(BL0939_ADDR5,WA_CREEP);//设置有功功率防潜寄存器，用于噪声功率切除
    Bsp_Bl0939_Write(BL0939_ADDR6,WA_CREEP); 

	Four_Data.byte_data[0]=0x00;
	Four_Data.byte_data[1]=0x00; 
    Four_Data.byte_data[2]=0x00;
    Four_Data.byte_data[3]=0x00;//关闭写保护
    Bsp_Bl0939_Write(BL0939_ADDR1,USR_WRPROT);
    Bsp_Bl0939_Write(BL0939_ADDR2,USR_WRPROT);
    Bsp_Bl0939_Write(BL0939_ADDR3,USR_WRPROT);
    Bsp_Bl0939_Write(BL0939_ADDR4,USR_WRPROT);
    Bsp_Bl0939_Write(BL0939_ADDR5,USR_WRPROT);
    Bsp_Bl0939_Write(BL0939_ADDR6,USR_WRPROT);
    for (uint8_t i = 0; i < PORTMAX; i++)
    {
        App_Bl0939_Clear(i+1);
    }
    MeterInitflag = 1;
    
}
