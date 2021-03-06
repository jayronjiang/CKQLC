
/***************************************************************************
Modula:    XY-LC-301
Version:   4.1
Revisor:   Liu XingZhou
StartDate: 2004/04/05
LastUpdate:2008/06/08
***************************************************************************/

//#define TEST
//#define _DEBUG
//#pragma ot(0,speed)

//==============================================================================================
#include <Reg52.h>
#include <intrins.h>
#include "XY-LC301.h"
//=============================================================================================
#ifdef _DEBUG
char code VerInfo[]="LC-301-V4.1-debug";
#else
char code VerInfo[]="LC-301-V4.1-160321";
#endif
char code *CodeMark[]={
    "CODEMARK[Lane Control for general purpose use.",
    __DATE__,
    "]"
};
//---------------------------------------------------------------------------------------------
bit bdata buffer_star;	// buffer starts to receive the data
bit bdata buffer_sto;	// Messages are finished.
bit bdata message_right;	// Message received is OK.
bit bdata message_wrong;	// Message received is not OK.
bit bdata dete_change;	// indacate status of P2 changed.
bit bdata bTTL_ALG_Wrong;

bit bdata bEnBarAutoDown;
bit bdata con_speak_fee;
bit bdata dete_bit_recd;

bit bdata alarm_flag;

bit bdata TTL_working_wrong;	//TongXing Lamp
bit bdata ALG_working_wrong;	//Auto-LanGan

bit bdata ALG_up_flag_bit;		// 状态指示, 1: 已经抬杠, 0:未抬杠
bit bdata ALG_down_flag_bit;	// 状态指示, 1: 已经降杠, 0:未降杠
bit bdata LastLaneLampState;
bit bdata watching_car_passing;	//用于防砸功能:在车辆从线圈离开、降竿的过程置1，启动防砸监视,栏杆彻底降下或则接收到抬杆命令，置0，停止监视

bit bdata bLastLaneRedGreenOperateState;
bit bdata bALGTTLAutoDetect;
bit bdata bFeeCleared;
bit bdata bTXok;		// Sending is finished OK.

unsigned char data COMM_buff[COMM_LENTH];
unsigned char data send_addr;
unsigned char data tmp_use1;
unsigned char data alarm_time_counter;
unsigned char data detect_time_counter; 

unsigned char data ReceiveIndicateCount; // Used to light off the LED when receiving, ~20ms
unsigned char data ReceiveTimeoutCount;

unsigned char data WatchingDelayCount=0;
//-----------------------------------------------------------------------------
unsigned char bdata control_char1;         // 	control_char1=COMM_buff[3]; 
sbit  ALG_up_bit      =control_char1^0; 
sbit  ALG_down_bit    =control_char1^1;
sbit  TTL_green_bit   =control_char1^2;
sbit  VOX_alarm_bit   =control_char1^3;
sbit  Light_alarm_bit =control_char1^4;
sbit  Lane_lamp_bit   =control_char1^5;		//CheDao TongXing Lamp
sbit  control_bak1_bit=control_char1^6;
sbit  control_bak2_bit=control_char1^7;

unsigned char bdata control_char_bak2;          	//'control_char_bak2=COMM_buff[20];
sbit  ALG_control_mode	=control_char_bak2^0;	//Mode: Auto LG or Manual LG
sbit  FEE_display_bit 	=control_char_bak2^1;
sbit  SPEAK_control_bit	=control_char_bak2^2;
sbit  FEE_displaykeep_bit=control_char_bak2^3;	//用于功能：绿灯后，费显仍然保持

sbit  bFeeDispSelection	=control_char_bak2^4;	//费显选择，1-无保持功能的广东用版本，0-有保持功能的山西版本
sbit  bControlOption	=control_char_bak2^5;	//车控器使用属性: 1-出口, 0-入口
sbit  bFeeDispReserved2	=control_char_bak2^6;

//-----------------------------------------------------------------------------
unsigned char bdata  device_status_char1;       //device_status_char1 = P2
sbit  TTL_detect_bit   =device_status_char1^0;  // 雨棚灯状态
sbit  LG_detect_bit    =device_status_char1^1;
sbit  ALarm_detect_bit =device_status_char1^2;
sbit  ALG_detect_bit   =device_status_char1^3;  // 栏杆状态, 0:降杠到位 1: 抬杠到位
sbit  FRONT_coil_bit   =device_status_char1^4;
sbit  BACK_coil_bit    =device_status_char1^5;
sbit  bakup_detect1_bit=device_status_char1^6;
sbit  bakup_detect2_bit=device_status_char1^7;

 
unsigned char bdata device_status_bakused;      //'B' type message 
sbit  ALG_work_status=device_status_bakused^2;	 // 抬杠过程是否完成?
sbit  TTL_work_status=device_status_bakused^6;

unsigned char bdata sys_tmep;                 //auto detect system used, sys_tmep = P2
sbit sys_tmep_b7=sys_tmep^7;		     //It is also the low level, if ACTIVE, the signal is LOW.
sbit sys_tmep_b6=sys_tmep^6;
sbit sys_tmep_b5=sys_tmep^5;
sbit sys_tmep_b4=sys_tmep^4;
sbit sys_tmep_b3=sys_tmep^3;
sbit sys_tmep_b2=sys_tmep^2;
sbit sys_tmep_b1=sys_tmep^1;
sbit sys_tmep_b0=sys_tmep^0;

char data PowerOnFlag;		//this byte do NOT to initialize
//==============================================================================================
void InitSys(void);
void PrintVerInfo(void);
void DumpMemory(void);
void Uart1IntEntry(void);
void FeedDog(void);
void ddl_200us(int x);
void send_mess(unsigned char);
void judge_rec_message(void);
void detect_device(void);
void control_device(void);
void control_speaker(void);
void device_keep(void);
void system_auto_detect(void);

void delay_10s(void);
void ONE_FEEdisplay_Red(void);
void ONE_FEEdisplay_Green(void);
void ClearFEEdisplay(void);

bit  detect_ALG_TTL_working(void);
void send_test();
void InitApp(void);
char GetControlChar(void);
//--------------------------------- additional -------------------------------------------------
void UpdateFEEdisplay(void);
void BarOpRectifyLaneLamp(void);
bit  AutoBarDowningDetect(void);
//--------------------------------- extenal ----------------------------------------------------
extern void TestForLC301(void);	//added 0405
//==============================================================================================
void main(void)
{
	char FlashTimes=3;
	ddl_200us(200);	// Dealy for about 100ms?
	// Jerry: start sequence, communicaiton LED blinks 3times.
	do
	{
		COMMFLASH=1;	// Jerry: used to indicate if communication happens.
		ddl_200us(64);
		COMMFLASH=0;
		ddl_200us(64);
		FlashTimes--;
	}while(FlashTimes>0);

	InitSys();
	// If it is the first time powering on.
	// 没有什么意义，因为没有写入e2prom
	if(PowerOnFlag!=(char)0xAA)
	{
		PowerOnFlag=(char)0xAA;
		system_auto_detect();
		InitApp();

		ALG_down_bit=1;
		ALG_up_bit=0;
		TTL_green_bit=0;
		VOX_alarm_bit=0;
		Light_alarm_bit=0;
		Lane_lamp_bit=0;
		control_bak1_bit=0;
		control_bak2_bit=0;
	}
	else
		PrintVerInfo();		//poweron info, used for debugging

	control_device();	 	//power on ,control TTL red and ALG down


	TestForLC301();	//用于老化测试

	while(1)
	{
		WDT=!WDT;

		//------------处理接收命令 部分 ------------------
		if(buffer_sto==1)
		{
			buffer_sto=0;
			FeedDog();
			judge_rec_message();
			if(message_right==1)
			{
				message_right=0;
				FeedDog();
				if(COMM_buff[2]!='1') continue;	//只有是本机地址才对设备进行控制
				// 如果是C 类信息, 判断是什么命令
				// 起始码  + 通信类 + 设备地址 + 调试命令 + 命令参数 + 结束码 + 校验码
				// R' - 复位， 'V' - 取版本信息，'D' - 将车控器内存信息导出(.bin格式)
				if((COMM_buff[1]=='C')&&(COMM_buff[3]=='R')) {while(1);} // Reset the system?
				if((COMM_buff[1]=='C')&&(COMM_buff[3]=='V')) {PrintVerInfo(); continue;}
				if((COMM_buff[1]=='C')&&(COMM_buff[3]=='D')) {DumpMemory(); continue;}
				//if(COMM_buff[1]!='A') continue;	//以下只处理B类信息

				detect_ALG_TTL_working();

				// Message B response
				// 起始码 + 通信类 + 设备地址 + 设备状态字节1 + 设备状态字节2 + 结束码 + 校验码 (共7个字节)
				send_mess(0x02);
				send_mess('B');
				send_mess('1');
				// device_status_char1:设备状态字节
				// 0位: 雨棚灯状态      --0为红灯，1为绿灯
				// 1位: 封道栏杆状态--0为打开，1为关闭
				// 2位: 脚踏报警状态--0为有报警，1为无报警
				// 3位: 自动栏杆状态--0为降杆，1为抬杆
				// 4位: 前线圈                 --0为有车，1为无车
				// 5位: 后线圈                 --0为有车，1为无车
				// 6位: 车道通行灯      --0为红灯，1为绿灯
				// 7位: 备用                       --0为启动，1为无
				send_mess(device_status_char1);
				// device_status_bakused: 其中设备状态字节2:
				// 0位: 备用
				// 1位: 备用
				// 2位: 栏杆       --('0'表示正常, '1'表示出错)
				// 3位: 备用
				// 4位: 备用
				// 5位: 备用
				// 6位: 顶棚灯 --('0'表示正常, '1'表示出错)
				// 7位: 备用
				send_mess(device_status_bakused);
				send_mess(0x03);
				// 计算结束码, 是前面字节的依次异或
				tmp_use1=('B')^('1');
				tmp_use1=tmp_use1^(device_status_bakused);
				tmp_use1=tmp_use1^device_status_char1;              
				send_mess(tmp_use1);


				//外设控制(注意保持在其他控制之前)
				// Message A receiving
				// 起始码+通信类+设备地址+控制字节1+金额(4字节)+余额(4字节)+入口站(6字节)
				// +车型+车种+控制字节2+通讯序号+结束码+校验码 (共24个 字节)

				// 控制字节2:
				// 0位: 栏杆控制位  --1:车辆过后自动降杆模式，0:车辆过后手动降杆模式
				// 1位: 费额显示位  --1:费额显示，            0:费额不显示
				// 2位: 语音报价位  --1:报价，                0:不报价
				// 3位: 费额显示保持--1:保持，            0:刷新，当为1时，忽略bit1
				// 4位: 费显选择位  --1:广东版本，无保持功能，0:山西版本有保持功能，本控制需要费显支持
			       // 5位: 车控器属性  --1:用于出口，            0:用于入口
				control_char_bak2=COMM_buff[20];
				// 控制字节1:
				// 0位: 起栏杆--1为抬杆，0为保持
				// 1位: 落栏杆--1为降杆，0为保持
				// 2位: 顶棚灯--1为绿灯，0为红灯
				// 3位: 声报警--1为声报警，0为声报警取消
				// 4位: 光报警--1为光报警，0为光报警取消
				// 5位: 通行灯--1为绿灯，0为红灯
				// 6位: 未定义--备用
				// 7位: 未定义--备用
				control_char1=COMM_buff[3];  // 控制字节
				if(ALG_up_bit)
				{
					bEnBarAutoDown=ALG_control_mode;	//自动降杆模式
					ALG_control_mode=0;					//清除该设置位, 汕芬潮州站军警车道遥控栏杆机需求发起
					watching_car_passing=0;
/*					if(BACK_coil_bit==0){
						dete_bit_recd=1;                //抬杆后开始计后线圈有无车过 P20100603＋
					}else dete_bit_recd=0;
*/
					dete_bit_recd=0;     //抬杆后开始计后线圈有无车过 P20100603＋
				}
				BarOpRectifyLaneLamp();					//根据现场被控设备逻辑情况，调整控制位
				control_device();


				//扬声器操作
				if(SPEAK_control_bit==1)
				{
					control_speaker();
				}
				else ddl_200us(250);		//该处延时约等于if内的时间，保证连续对费显的操作的间隔时间

				
				//费额显示操作
				if(FEE_displaykeep_bit==0)
				{
					if(FEE_display_bit==1)		
						UpdateFEEdisplay();		//显示费用余额
					else ClearFEEdisplay();		//清除费额，打红灯(有些打绿灯也可以)就清除费额显示
				}
			}

			// Send to PC 0x15 'E' 'E' 'E' 'E ' 0x03 0x00'
			if(message_wrong==1)
			{
				message_wrong=0;
				send_mess(0x15);
				for(send_addr=0;send_addr<=3;send_addr++)
					send_mess('E');
				send_mess(0x03);
				send_mess(0x00);
			}
		}

      	
		//------------检测状态部分 ------------------
		WDT=!WDT;
		detect_device();

		#ifdef TEST
		if(TTL_detect_bit==0)	//这两行代码在山西太旧中使用，但不知道用于什么目的
		 	send_test();
		#endif

		if(dete_change)
		{
			dete_change=0;
			detect_ALG_TTL_working();
			send_mess(0x02); send_mess('B'); send_mess('1');
			send_mess(device_status_char1);
			send_mess(device_status_bakused);
			send_mess(0x03);
			tmp_use1='B'^'1';
			tmp_use1=tmp_use1^device_status_bakused;
			tmp_use1=tmp_use1^device_status_char1;              
			send_mess(tmp_use1);
                                     
			if(BACK_coil_bit==0)	 // 如果有车
				dete_bit_recd=1;	// if car on coil, record set
		}


		//防止砸车功能，以上注释部分的改进
		WDT=!WDT;
		// 降杠过程中发现后线圈有车, 抬杠,并再次进入自动降杠过程
		if(watching_car_passing){
		if(BACK_coil_bit==0)	//后线圈有车
		{
			if(ALG_down_flag_bit&ALG_detect_bit)	//正在降落...
			{
				ALG_up_bit=1;
				ALG_down_bit=0;
				control_device();
				bEnBarAutoDown=1; 
			}
		}}

		// Alarming for 10sec
		if(ALarm_detect_bit==0)	//valid is 0
		{
			alarm_time_counter=ALARM_TIME;
			VOX_alarm_bit=1;
			Light_alarm_bit=1;
			device_keep();
		}

		//自动降栏杆
		WDT=!WDT;
		if(bEnBarAutoDown==1)		// 自动降杠模式	
		{
			// 如果上次有车, 这次无车,表示车已经离开
			if(BACK_coil_bit==1)		//Back Coil Flag =1, No Car
			{
				if(dete_bit_recd==1)	//Car ever on Coil last time, this 2 line indicate Car leave Coil
				{
					dete_bit_recd=0;
					bEnBarAutoDown=0;	//复位为手动降栏杆模式
					ALG_down_bit=1;		// 开启降杠过程
					ALG_up_bit=0;
					Lane_lamp_bit=0;	//red
					control_device();
					watching_car_passing=1; 
				}
			}
		}
			
		WDT=!WDT;
		// 设备每隔3s自动检测下状态
		if(bALGTTLAutoDetect)		//设备错误状态自动检测部分
		{
			bALGTTLAutoDetect=0;
    			if(detect_ALG_TTL_working())
			{
				bTTL_ALG_Wrong=1;			//如果是错误状态，每当该标志置位时向上位机汇报
				dete_change=1;
			}
			else
			{
				if(bTTL_ALG_Wrong) dete_change=1;	//从错误状态返回到正常状态时，及时向上位机汇报
				bTTL_ALG_Wrong=0;
			}
		}
		WDT=!WDT;
	}
}
//==============================================================================================
//SysInit.c

/******************************************************
XY-LC-301 BASIC INITIALIZATION SECTION
******************************************************/
//复位时，车控器向上位机发版本信息，如：XY-LC-301-V4.1-40618
void PrintVerInfo(void)
{
	char i=0;
	char chr;
	chr=VerInfo[i];

	FeedDog();
	send_mess('X');
	send_mess('Y');
	send_mess('-');
	while(chr!=0)
	{
		send_mess(chr);
		i++;
		chr=VerInfo[i];
	}
	send_mess(0x0D);
	send_mess(0x0A);
}
void DumpMemory(void)
{
	unsigned char *pMem=0;	


	FeedDog();
	send_mess('X');
	send_mess('Y');
	send_mess('_');
	while(pMem<0x80)
	{
		send_mess((char)*pMem);
		pMem++;
	}
	send_mess(0x0D);
	send_mess(0x0A);
}
//==============================================================================================
// System configures, enable the interrupts.
void InitSys(void)
{
	// SM0:SM1 = 0:1, 10位UART 模式
	SCON=0x40;
	// TMOD = 0b0010 0001, 即TM1与TM0都工作于定时器模式
	// TM1: 模式2,自动重载. TM0: 模式1,16位定时器
	TMOD=0x21;
	// TM1 8位重载定时器,波特率计算公式:
	// TH1=TL1= 256-(Fosc/(baud*12*32*2^SMOD))  SMOD 取0和1
	TH1=0xFD;	//9600bps when in 11.0592Mhz
	TL1=0xFD;
	// 开启定时器1
	TR1=1;
	// 使能串口中断
	ES=1;
	// 使能接收
	REN=1;

	/* 开启定时器0 */
	TR0=1;
	/* 使能定时器0中断*/
	ET0=1;
	/* 设置串口中断为高优先级*/
	PS=1;

	/*声音报警、费显和上位机通信共用一个串口*/
	TOVOX=0;
	TOFEE=0;
	TOCPU=1;

	/* 开总中断*/
	EA=1;
}
//==============================================================================================
void InitApp(void)
{  
	char i;    

	FeedDog();
	// All the devices are Low level to drive.
	BAR_UP=1;
	BAR_DOWN=1;
	TTL_GREEN=1;
	VOX_ALM=1;
	LIGHT_ALM=1;
	LAN_LAMP=1;
	BAK1_USED=1;
	BAK2_USED=1;
    
	alarm_time_counter=0;
	tmp_use1=0;
	send_addr=0;
	sys_tmep=0;
	control_char_bak2=0;
	control_char1=0;
	device_status_char1=0x00;		//all device disconnected
	device_status_bakused=0xbb;		//system default, all device are good
	detect_time_counter=AUTO_DETCET_TIME;
	for(i=0; i<COMM_LENTH; i++)
		COMM_buff[i]=0;

	ReceiveIndicateCount=0;
	ReceiveTimeoutCount=RECEIVE_TIMEOUT;

	buffer_star=0;
	buffer_sto=0;
	message_right=0;
	message_wrong=0;
	dete_change=0;

	bEnBarAutoDown=0;
	con_speak_fee=0;
	dete_bit_recd=0;

	alarm_flag=0;
	TTL_working_wrong=0;	//TongXing Lamp
	ALG_working_wrong=0;	//Auto-LanGan
	ALG_up_flag_bit=0;
	ALG_down_flag_bit=0;
	LastLaneLampState=0;
	watching_car_passing=0;	//用于防砸功能:在车辆从线圈离开、降竿的过程置1，启动防砸监视,栏杆彻底降下或则接收到抬杆命令，置0，停止监视
	bLastLaneRedGreenOperateState=0;
	bALGTTLAutoDetect=0; 
	bFeeCleared=0;
	bTTL_ALG_Wrong=0;
	bTXok=0;
}

//==============================================================================================
void init_uart1() interrupt 4 using 3
{
	static unsigned char data tmp,rec_counter;

  	// Receiving is finished.
	if(RI)
	{
		RI=0;
		WDT=!WDT;
		tmp=SBUF;
		if(tmp==0x02&&buffer_star==0)
		{
			rec_counter=0;
			COMM_buff[rec_counter]=tmp; 
			buffer_star=1;	// The first byte.
			rec_counter++;
			COMMFLASH=1; // When receiving, flash the led.
			ReceiveIndicateCount=1;	//CommLedFlash Delay Count
			ReceiveTimeoutCount=RECEIVE_TIMEOUT; 
		}                      
		else
		{
			if(rec_counter<=(COMM_LENTH-1)&&buffer_star==1)
			{
				COMMFLASH=1;
				ReceiveIndicateCount=1;
				ReceiveTimeoutCount=RECEIVE_TIMEOUT;
				COMM_buff[rec_counter]=tmp; 
				if(rec_counter==(COMM_LENTH-1))
				{
					rec_counter=0;
					buffer_star=0;
					ReceiveTimeoutCount=0;	//数据帧接收完，清除超时设定值
					buffer_sto=1;
				}
				else
					rec_counter++;
			}
		} 
		WDT=!WDT;
	}
	else
	{
		// If not the RX INT, it is the TX INT
		TI=0;
		bTXok=1;  // Sending is OK.
	}
}
//==============================================================================================
//Common.c
//==============================================================================================
// Calculate the BCC and compare them if it is OK.
void judge_rec_message()
{
	unsigned char i,j;
	FeedDog();
	if(COMM_buff[0]==0x02&&COMM_buff[22]==0x03)
	{
		j=COMM_buff[1];
		for(i=2;i<=(COMM_LENTH-3);i++)
			j=COMM_buff[i]^j;
		if(j==COMM_buff[COMM_LENTH-1])
			message_right=1;
		else
			message_wrong=1;
	}
	else
	{
		message_wrong=1;
	}
}
//==============================================================================================
// Send 1 single byte process
void send_mess(unsigned char n)
{
	unsigned int addtmp=1000;
	WDT=!WDT;
	COMMFLASH=1;
	bTXok=0;
	SBUF=n;
	while(1)
	{
		if(bTXok) break;
		addtmp--;
		if(addtmp==0) break;
	}
	COMMFLASH=0;
	WDT=!WDT;
}
//==============================================================================================
/********************************************************************************************************
** 函数名称: ddl_200us
** 功能描述: delay for about 500us:(how to calculate?)
**				51单片机一个机器周期为12个时钟周期，那么11.0592M的时钟一个机器周期为:
**				机器周期= 1*12/11.0592 = 1.085us.
**				在KEIL里面查看函数的汇编: 从0x0521开始计算循环: 指令周期等效为机器周期为:
**				C:0x051F    E4       CLR      A
**				C:0x0520    FD       MOV      R5,A
**				C:0x0521    ED       MOV      A,R5			+1
**				C:0x0522    C3       CLR      C                       +1
**				C:0x0523    9421     SUBB     A,#device_status_bakused(0x21)     +2
**				C:0x0525    50EE     JNC      ddl_200us(C:0515)				+2
**				   558: void ddl_200us(int x) 
**				C:0x0527    B294     CPL      WDT(0x90.4)		+2
**				C:0x0529    00       NOP      					+1
**				C:0x052A    00       NOP      					+1
**				C:0x052B    00       NOP      					+1
**				C:0x052C    00       NOP      					+1
**				C:0x052D    0D       INC      R5				+1
**				C:0x052E    80F1     SJMP     C:0521			+2
**				   559: { 
**				   560:         unsigned char j; 
**				   561:         while(x--) 
**				   562:         { 
**				C:0x0530    22       RET      
**				共15*33 = 495, 共495*1.085 = 537us.
** 输 　 入:
**
** 输　  出:
**
** 全局变量:
** 调用模块:
**
** 作　  者: Jerry
** 日　  期: 20180920
**------------------------------------------------------------------------------------------------------
** 修 改 人:
** 日　  期:
**------------------------------------------------------------------------------------------------------
*******************************************************************************************************/
void ddl_200us(int x)
{
	unsigned char j;
	while(x--)
	{
		for(j=0;j<33;j++)
	 	{WDT=!WDT; _nop_(); _nop_(); _nop_(); _nop_();}
	}
}

//==============================================================================================
// 20ms interrupt?
void timer0_int_entry() interrupt 1 using 1
{
	static char DidaCount=0;
	static bit bReEntry=0;
	TH0=0xB8;TL0=0x12;		  //20ms
	if(bReEntry) return;	//重入处理，好象系统本身具有这

	WDT=!WDT;
	bReEntry=1;
	DidaCount++;			//微调
	// It runs 200ms after the power on.
	if(DidaCount<10) {bReEntry=0; return;}

	DidaCount=0;

	//codes below execute every 200ms!!!!
	// 降杠时监控时间, 超过此时间则不监视后线圈
	if(WatchingDelayCount>0)
	{
		WatchingDelayCount--;
		if(WatchingDelayCount==0) watching_car_passing=0;
	}
	// 接收时闪灯间隔
	if(ReceiveIndicateCount>0)
	{
		ReceiveIndicateCount--;
		if(ReceiveIndicateCount==0)	COMMFLASH=0;	//Close LED
	}

	// 接收超时, 数据帧断续
	if(ReceiveTimeoutCount>0)
	{
		ReceiveTimeoutCount--;
		if(ReceiveTimeoutCount==0)
		 	{buffer_star=0; buffer_sto=0;}	//接收超过时间复位
	}

	// 系统检测状态和控制位是否一致,每3s检测一次
	if(detect_time_counter>0)
		detect_time_counter--;
	else
	{
		bALGTTLAutoDetect=1;
		detect_time_counter=AUTO_DETCET_TIME;
	}

	// 报警长度,10S
	if(alarm_time_counter>0)
	{
		alarm_time_counter--;
		if(alarm_time_counter==0)
		{
			VOX_ALM=1;	//STOP Alarm
			LIGHT_ALM=1;
		}
	}

	bReEntry=0;
}
//==============================================================================================
void detect_device(void)
{
	unsigned char outside_tmp,outside_tmp1;

	FeedDog();
	outside_tmp=P2;
	if(outside_tmp!=device_status_char1)
	{
		outside_tmp1=outside_tmp;
		ddl_200us(60);	//80-40ms ;//40-20ms ;//60-30ms ;//20-10ms ;//100-50ms
		outside_tmp=P2;
		if(outside_tmp==outside_tmp1)	// 30ms防抖结果
		{
			dete_change=1;
			device_status_char1=outside_tmp;
			if(ALG_detect_bit==0) watching_car_passing=0; 
		}
	}
}

//==============================================================================================
// Detect if the action is done or not.
bit  detect_ALG_TTL_working(void)
{
		FeedDog();
		//detect_device();

		if(ALG_up_flag_bit==1)
		{
					if(ALG_detect_bit==1)	ALG_work_status=0;	//抬杆状态
					else 				    ALG_work_status=1;	//正在抬杆或出错
		}
		
		if(ALG_down_flag_bit==1)
		{
					if(ALG_detect_bit==0)	ALG_work_status=0;	//降杆状态
					else					ALG_work_status=1;	//正在降杆或出错
		}
		
		if(TTL_green_bit==TTL_detect_bit)
		{
					 TTL_work_status=0;
		}
		else TTL_work_status=1;



		if(TTL_work_status==1||ALG_work_status==1)	// ALG/TTL_work_status=0, working normal
					return 1;
		else 		return 0;

}
//==============================================================================================
/********************************************************************************************************
** 函数名称: system_auto_detect
** 功能描述: Test all extern device if work normal
** 输 　 入: First output a ACTIVE signal, wait 1or 2 sec, read the feedback
**			:	Then output a INACTIVE signal, wait 1or 2 sec, read the feedback again.
** 输　  出:
**
** 全局变量:
** 调用模块:
**
** 作　  者: Jerry
** 日　  期: 20180925
**------------------------------------------------------------------------------------------------------
** 修 改 人:
** 日　  期:
**------------------------------------------------------------------------------------------------------
*******************************************************************************************************/
void system_auto_detect()
{
	//  unsigned char i,j;
	FeedDog();

	BAK2_USED=0;	// Output a signal.
	COMMFLASH=1;
	ddl_200us(505);WDT=0;WDT=1;
	ddl_200us(505);WDT=0;WDT=1;
	ddl_200us(505);WDT=0;WDT=1;
	ddl_200us(505);WDT=0;WDT=1;
	ddl_200us(505);WDT=0;WDT=1;
	sys_tmep=P2;				//(8-chanel input-detect legcy to P2)

	if(sys_tmep_b7==0)	// Read the feedback
	{
		BAK2_USED=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		sys_tmep=P2;

		if(sys_tmep_b7==1)
			COMMFLASH=0;
		else
		{
			BAK2_USED=0;
			COMMFLASH=0;
		}

		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;

		// BAK1_USED signal testing begins.
		COMMFLASH=1;
		BAK1_USED=0;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;

		sys_tmep=P2;
		if(sys_tmep_b6==0)
		{
			BAK1_USED=1;
			ddl_200us(505);WDT=0;WDT=1;
			ddl_200us(505);WDT=0;WDT=1;
			ddl_200us(505);WDT=0;WDT=1;
			sys_tmep=P2;
			if(sys_tmep_b6==1){COMMFLASH=0;}
			else{BAK1_USED=0;COMMFLASH=0;}
		}

		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;

		// LAN_LAMP signal testing begins.
		COMMFLASH=1;				//start test lamp
		LAN_LAMP=0;				//Open Lamp
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;	//delay ~0.75s total
		ddl_200us(505);WDT=0;WDT=1;

		sys_tmep=P2;				//read status
		if(sys_tmep_b5==0)			//if open Lamp success
		{
			LAN_LAMP=1;			//close lamp
			ddl_200us(505);WDT=0;WDT=1;
			ddl_200us(505);WDT=0;WDT=1;
			ddl_200us(505);WDT=0;WDT=1;
			sys_tmep=P2;			//read lamp status again
                                           
			if(sys_tmep_b5==1)		//if close lamp suceess
				COMMFLASH=0;		//finish test lamp
			else
			{
				LAN_LAMP=0;		//if close lamp fail, LED indicate fail chanel
				COMMFLASH=0;		//finish test lamp
			}
		}
                       
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		COMMFLASH=1;

		// LIGHT_ALM signal testing begins.
		LIGHT_ALM=0;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		
		sys_tmep=P2;                   
		if(sys_tmep_b4==0)
		{
			LIGHT_ALM=1;
			ddl_200us(505);WDT=0;WDT=1;
			ddl_200us(505);WDT=0;WDT=1;
			ddl_200us(505);WDT=0;WDT=1;
			sys_tmep=P2;
			if(sys_tmep_b4==1)
				COMMFLASH=0;
			else
			{
				LIGHT_ALM=0;
				COMMFLASH=0;
			}
		}
                                           
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;

		// VOX_ALM signal testing begins.
		COMMFLASH=1;
		VOX_ALM=0;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;

		sys_tmep=P2;                   
		if(sys_tmep_b3==0)
		{
			VOX_ALM=1;
			ddl_200us(505);WDT=0;WDT=1;
			ddl_200us(505);WDT=0;WDT=1;
			ddl_200us(505);WDT=0;WDT=1;
			sys_tmep=P2;
			if(sys_tmep_b3==1)
				COMMFLASH=0;
			else
			{
				VOX_ALM=0;
				COMMFLASH=0;
			}
		}

		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;

		// TTL_GREEN signal testing begins.
		COMMFLASH=1;
		TTL_GREEN=0;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;
		WDT=1;ddl_200us(505);
		WDT=0;WDT=1;

		sys_tmep=P2;                   
		if(sys_tmep_b2==0)
		{
			TTL_GREEN=1;
			ddl_200us(505);WDT=0;WDT=1;
			ddl_200us(505);WDT=0;WDT=1;
			ddl_200us(505);WDT=0;WDT=1;

			sys_tmep=P2;
			if(sys_tmep_b2==1)
				COMMFLASH=0;
			else
			{
				TTL_GREEN=0;
				COMMFLASH=0;
			}
		}
  
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;

		// BAR_DOWN signal testing begins.
		COMMFLASH=1;
		BAR_DOWN=0;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		
		sys_tmep=P2;                   
		if(sys_tmep_b1==0)
		{
			BAR_DOWN=1;
			ddl_200us(505);WDT=0;WDT=1;
			ddl_200us(505);WDT=0;WDT=1;
			ddl_200us(505);WDT=0;WDT=1;
			sys_tmep=P2;
			if(sys_tmep_b1==1)
				COMMFLASH=0;
			else
			{
				BAR_DOWN=0;
				COMMFLASH=0;
			}
		}

		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;

		// BAR_UP signal testing begins.
		COMMFLASH=1;
		BAR_UP=0;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;

		sys_tmep=P2;                   
		if(sys_tmep_b0==0)
		{
			BAR_UP=1;
			ddl_200us(505);WDT=0;WDT=1;
			ddl_200us(505);WDT=0;WDT=1;
			ddl_200us(505);WDT=0;WDT=1;
			sys_tmep=P2;
			if(sys_tmep_b0==1)
				COMMFLASH=0;
			else
			{	
				BAR_UP=0;
				COMMFLASH=0;
			}
		}
                        
		TOVOX=1;TOFEE=1;TOCPU=0;
		//数据信息：02 +CheXing +CheZhong +Fee +Balance +EntranceName +03 +BCC
		//费显显示如上信息，对于语音报价器，则播放相应语音
		//CheZhong==0x37时，发'粤通卡写卡成功'
		//CheZhong==0x38时，发'欢迎光临'
		//CheZhong==0x31，并且Fee==0x30,0x30,0x30,0x30时，发'通行卡写卡成功'
		send_mess(0x02);
		send_mess('3'); //chexing
		send_mess('3'); //chezhong
		send_mess('5');
		send_mess('5');
		send_mess('0');
		send_mess('0');  //fee
		send_mess('0');
		send_mess('0');
		send_mess('3');
		send_mess('3');//bal
		send_mess('4');
		send_mess('4');
		send_mess('4');
		send_mess('4');
		send_mess('4');
		send_mess('4');//entrance
		send_mess(0x03);
		send_mess(0x00);
		TOVOX=0;TOFEE=0;TOCPU=1;
       
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		COMMFLASH=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		COMMFLASH=0;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		COMMFLASH=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		COMMFLASH=0;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		COMMFLASH=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		COMMFLASH=0;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		COMMFLASH=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		COMMFLASH=0;

		// 控制信息:02 +CtrlChar +03 +BCC
		// CtrlChar: 'Y'-开费显绿灯,对于语音报价器，则报‘谢谢，请通行'语音'
		// 'Z'-开费显红灯
		TOVOX=1;TOFEE=1;TOCPU=0;
		send_mess(0x02);
		send_mess('Y');
		send_mess(0x03);
		send_mess('Y');
		TOVOX=0;TOFEE=0;TOCPU=1;
                        
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;
		ddl_200us(505);WDT=0;WDT=1;

		TOVOX=1;TOFEE=1;TOCPU=0;
		send_mess(0x02);
		send_mess('Z');
		send_mess(0x03);
		send_mess('Z');
		TOVOX=0;TOFEE=0;TOCPU=1;
	}
	else
	{
		BAK2_USED=1;
		COMMFLASH=0;
	}                  
}

//==============================================================================================
//control.c
//==============================================================================================
void ONE_FEEdisplay_Red()
{
	TOVOX=0;TOFEE=1;TOCPU=0;
	ddl_200us(4);	//delay 2ms
//	send_mess(0x02);		   //P20100607--
//	send_mess(GetControlChar());
//	send_mess(0x03);
//	send_mess(GetControlChar());
	FeedDog();
	send_mess(0x02);	//Close Lane lamp
	send_mess('Z');
	send_mess(0x03);
	send_mess('Z');
	bLastLaneRedGreenOperateState=RED;
	TOVOX=0;TOFEE=0;TOCPU=1;
}

//==============================================================================================
void ONE_FEEdisplay_Green()
{
	TOVOX=0;TOFEE=1;TOCPU=0;
	ddl_200us(4);	//delay 2ms
//	send_mess(0x02);				 //P20100607--
//	send_mess(GetControlChar());
//	send_mess(0x03);
//	send_mess(GetControlChar());
	FeedDog();
	_nop_();
	_nop_();
	send_mess(0x02);
	send_mess('Y');
	send_mess(0x03);	//Open Lane Lamp
	send_mess('Y');
	bLastLaneRedGreenOperateState=GREEN;
	TOVOX=0;TOFEE=0;TOCPU=1;
}
//==============================================================================================
void ClearFEEdisplay(void)
{
	if (bFeeCleared) return;

	FeedDog();
	bFeeCleared=1;
	if(bLastLaneRedGreenOperateState==GREEN)
	{
		if(bFeeDispSelection==0)
			ONE_FEEdisplay_Green();  	//对于陕西版本的费显，只要开绿灯不清除费额
		else ONE_FEEdisplay_Red();	     //对于广东版本的费显，只要开绿灯或红灯都清除费额
	}
	else ONE_FEEdisplay_Red();	//不管是哪个版本的费显，只要开红灯都清除费额
}
//==============================================================================================
void control_device()
{
	FeedDog();

	BAR_DOWN=1;BAR_UP=1;
//--------栏杆控制,这是哪个栏杆?---------------------
	//control_char1=COMM_buff[3];
	// ALG_down_bit 初始化的时候为1
	if(ALG_down_bit==1)
	{
		detect_device();                 //后线圈状态刷新  P20200607+
		if(BACK_coil_bit==1) {      //后线圈没车时才落杆  P20100603＋
			ONE_FEEdisplay_Red();

			pinSwtichALG=0;	//0降杆
			BAR_DOWN=0;		//Valid level: low, Pusle to DOWN LG
			WatchingDelayCount=20;	//20*200us=4 senconds	  20*20ms=400ms

			ALG_down_flag_bit=1;
			ALG_up_flag_bit=0;
		}else{							  //P20100603＋
			bEnBarAutoDown=1;             //后线圈有车，用自动落杆P20100603＋
		}
	}

	if(ALG_up_bit==1)
	{
		ONE_FEEdisplay_Green();

		pinSwtichALG=1;	//1抬杆
		BAR_UP=0;			//Valid level: low, Pusle to UP LG

		ALG_up_flag_bit=1;
		ALG_down_flag_bit=0;
	}
//-------------------------------------
	// Accept the commands from PC, control the devices.
	if(TTL_green_bit==1)
			TTL_GREEN=0;
	else	TTL_GREEN=1;

	if(VOX_alarm_bit==1)
			VOX_ALM=0;
	else	VOX_ALM=1;

	if(Light_alarm_bit==1)
			LIGHT_ALM=0;
	else	LIGHT_ALM=1;

	if(Lane_lamp_bit==1)
			LAN_LAMP=0;
	else	LAN_LAMP=1;
	LastLaneLampState=Lane_lamp_bit;


//--------------------------------------
	ddl_200us(505);					   //244  ms  (P)
	BAR_DOWN=1;BAR_UP=1;		// 这2个是一个下降沿脉冲，宽度为250ms左右
	detect_time_counter=AUTO_DETCET_TIME;
}

//==============================================================================================
void control_speaker()
{
	unsigned char i,j;
 	//control_char1=COMM_buff[3];

	FeedDog();
	// 逻辑? 要等控制命令被清除后再开启扬声器?
  	if(ALG_up_bit==0&&ALG_down_bit==0)
	{
		TOVOX=1;TOFEE=0;TOCPU=0;           //TOCPU=1;      20160419M
		ddl_200us(4);	//delay 2ms
		j=COMM_buff[4];

		for(i=5;i<=19;i++)
			j=j^COMM_buff[i];	//bcc
		send_mess(0x02);
		send_mess(COMM_buff[18]); //chexing
		send_mess(COMM_buff[19]); //chezhong
		send_mess(COMM_buff[4]);
		send_mess(COMM_buff[5]);
		send_mess(COMM_buff[6]);
		send_mess(COMM_buff[7]);	//fee
		send_mess(COMM_buff[8]);
		send_mess(COMM_buff[9]);
		send_mess(COMM_buff[10]);
		send_mess(COMM_buff[11]);//bal
		send_mess(COMM_buff[12]);
		send_mess(COMM_buff[13]);
		send_mess(COMM_buff[14]);
		send_mess(COMM_buff[15]);
		send_mess(COMM_buff[16]);
		send_mess(COMM_buff[17]);//entrance
		send_mess(0x03);
		send_mess(j);
		TOVOX=0;TOFEE=0;TOCPU=1;  // 完成后返回到默认为接收状态
	}
	
	if(ALG_up_bit==1)
	{
		TOVOX=1;TOFEE=0;TOCPU=0;      //TOCPU=1;      20160419M
		ddl_200us(4);	//delay 2ms
		send_mess(0x02);
		send_mess('Y');
		send_mess(0x03);
		send_mess('Y');
		TOVOX=0;TOFEE=0;TOCPU=1;
	}   
}
//==============================================================================================
//除栏杆外的设备状态保持控制
void device_keep()
{
	FeedDog();
	if(TTL_green_bit==1)
		TTL_GREEN=0;
	else	TTL_GREEN=1;

	if(VOX_alarm_bit==1)
		VOX_ALM=0;
	else	VOX_ALM=1;
	
	if(Light_alarm_bit==1)
		LIGHT_ALM=0;
	else	LIGHT_ALM=1;
	
	if(Lane_lamp_bit==1)
		LAN_LAMP=0;
	else	LAN_LAMP=1;    
}
//================================================================================================
#ifdef TEST
void send_test()
{
	TOVOX=1;TOFEE=1;TOCPU=0;
	ddl_200us(4)	//delay 2ms
	FeedDog();
	send_mess(0x80);
	send_mess(0x3f);send_mess(0x30);send_mess(0x5b);
	send_mess(0x4f);send_mess(0x66);send_mess(0x6d);send_mess(0x7d);send_mess(0x07);
	send_mess(0x7f);
	send_mess(0x01);
	TOVOX=0;TOFEE=0;TOCPU=1;
}
#endif
//==============================================================================================
/***********************************************
//codes below added by Liu XingZhou, 2004/05/15
***********************************************/
//#include "Addtional.h"
//==============================================================================================
void UpdateFEEdisplay(void)
{
	unsigned char i,j;
	TOVOX=0;TOFEE=1;TOCPU=0;
	ddl_200us(4);	//delay 2ms
	FeedDog();
		j=COMM_buff[4];
		for(i=5;i<=19;i++)
			j=j^COMM_buff[i];	//bcc
		send_mess(0x02);
		send_mess(COMM_buff[18]); //chexing
		send_mess(COMM_buff[19]); //chezhong
		send_mess(COMM_buff[4]);
		send_mess(COMM_buff[5]);
		send_mess(COMM_buff[6]);
		send_mess(COMM_buff[7]);  //fee
		send_mess(COMM_buff[8]);
		send_mess(COMM_buff[9]);
		send_mess(COMM_buff[10]);
		send_mess(COMM_buff[11]);//bal
		send_mess(COMM_buff[12]);
		send_mess(COMM_buff[13]);
		send_mess(COMM_buff[14]);
		send_mess(COMM_buff[15]);
		send_mess(COMM_buff[16]);
		send_mess(COMM_buff[17]);//entrance
		send_mess(0x03);
		send_mess(j);
	bFeeCleared=0;
	TOVOX=0;TOFEE=0;TOCPU=1;
}
//==============================================================================================
/*bit AutoBarDowningDetect(void)	//run after detect_deivce()
{
	if(ALG_down_flag_bit==1)		//已经发送降杆命令
	{
		if(ALG_detect_bit==1)	//栏杆仍为抬杆状态
		{
			return 1;
		}
	}
	return 0;
}*/
//==============================================================================================
/*
sbit  ALG_up_bit      =control_char1^0; 
sbit  ALG_down_bit    =control_char1^1;
sbit  Lane_lamp_bit   =control_char1^5;
*/
// 自动调整逻辑，以防接收到的上位机命令是矛盾的
void BarOpRectifyLaneLamp(void)
{
	if(ALG_up_bit&ALG_down_bit) //return;	//错误操作
		ALG_down_bit=0;		// 如果同时接收到抬杠和降杠，则抬杠
	FeedDog();
	if(ALG_up_bit) Lane_lamp_bit=1;		//绿灯, 如果抬杠，则必须亮绿灯
	if(ALG_down_bit) Lane_lamp_bit=0;		//红灯, 如果降杠，则必须亮红灯
	if((ALG_up_bit|ALG_down_bit)==0) Lane_lamp_bit=LastLaneLampState;
}
//==============================================================================================
void FeedDog(void)
{
	WDT=0;
	_nop_();
	_nop_();
	_nop_();
    	WDT=1;
}
//==============================================================================================
char GetControlChar()
{
	if(bFeeDispSelection)	//费显选择，1-无保持功能的广东用版本，0-有保持功能的山西版本
	{
		if(bControlOption)	//车控器使用属性: 1-出口, 0-入口
			return 'A';
		else return 'B';
	}
	else
	{
		if(bControlOption)	//车控器使用属性: 1-出口, 0-入口
			return 'C';
		else return 'D';
	}
}
//==============================================================================================
