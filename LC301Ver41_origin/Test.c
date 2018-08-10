/*************************************************************************************************
车道控制器测试老化程序
版本: Version 1.0
修订: Liu XingZhou
时间: 

设计说明:  
    1.程序启动时候检测输入端口，如果P2.6输入端口均有输入(有电压或者关合开关信号)，则
      转入本程序。
    2.本程序不断定期地从输出端(继电器/语音/RS485费显)输出数据和控制信号
注意事项: 
    使用时
其它信息: 
C_CHAR1.0 - AutoBar up signal
C_CHAR1.1 - AutoBar down signal
C_CHAR1.2 - TTL signal, 1: green, 0:red
C_CHAR1.3 - VoiceAlarm enable signal
C_CHAR1.4 - LightAlarm enable signal
C_CHAR1.5 - LaneLamp signal, 1:green, 0:red
C_CHAR1.6 - Reserved for later using
C_CHAR1.7 - Reserved for later using

C_BAK.0 - enable AutoBarUp Mode (0:ManulBarUp Mode)
C_BAK.1 - enable FeeDisplay
C_BAK.2 - enable Speaker
C_BAK.3 - enable KeepFeeDisplay
*************************************************************************************************/
//Directions End
//================================================================================================
#include <reg52.h>
#include <intrins.h>
#include "XY-LC301.h"
//------------------------------------------------------------------------------------------------
#define cLED_COMMUNICATE COMMFLASH
/*const unsigned char FeeTestCommData[]={0x02,'8','8','8','8','8','8','8','8','8','8','8','8','8','8',
	'8','8',0x03,0x00};
const unsigned char PlayVoiceCommData[]={0x02,'5','1', '1','2','3','4','4','3','2','1',
	0xBB,0xBC,0xCC,0xCD,0xDD,0xDE,0x03,0x01};
*/
extern void FeedDog(void);
//================================================================================================
void delay_ms(unsigned char n_ms)	//OSC = 11.0592Mhz
{
	unsigned char j;

	while(n_ms--)
	{
		for(j=0;j<250;j++)
	 	{
			WDT=1;
			_nop_();
			WDT=0;
			_nop_();
		}
	}
}
//=================================================================================================
bit comm_send_char(unsigned char out_char)
{
	int time_out=1000;
	bit ret = 1;

	FeedDog();
	cLED_COMMUNICATE=1;	//0 to light communication led
	TI = 0;
	SBUF = out_char;
	while(!TI)
	{
		if(time_out--) continue;
		ret =0;
		break;
	}
	TI=0;
	cLED_COMMUNICATE=0;	//shut communication led
	return ret;
}
//================================================================================================

bit InputDedect()
{
	unsigned char tmpInport;

	FeedDog();
	tmpInport=P2;
	if((tmpInport&0x40)==0)
	{
		delay_ms(5);
		tmpInport=P2;
		if((tmpInport&0x40)==0)
			return 1;
	}
	return 0;
}
//================================================================================================
void UpdateFeeDisplay(unsigned char Index)
{
	TOVOX=0;TOFEE=1;TOCPU=0;

	FeedDog();
	if(Index%2)
	{
		comm_send_char(0x02);	//Close Lane lamp
		comm_send_char('Z');
		comm_send_char(0x03);
		comm_send_char('Z');

		delay_ms(250);

		comm_send_char(0x02);
		comm_send_char('8');
		comm_send_char('8');
		comm_send_char('8');
		comm_send_char('8');
		comm_send_char('8');
		comm_send_char('8');
		comm_send_char('8');
		comm_send_char('8');
		comm_send_char('8');
		comm_send_char('8');
		comm_send_char('8');
		comm_send_char('8');
		comm_send_char('8');
		comm_send_char('8');
		comm_send_char('8');
		comm_send_char('8');
		comm_send_char(0x03);
		comm_send_char(0x00);
	}
	else
	{
		comm_send_char(0x02);	//Open Lane lamp
		comm_send_char('Y');
		comm_send_char(0x03);
		comm_send_char('Y');
	}
	TOVOX=0;TOFEE=0;TOCPU=1;
}
//================================================================================================
void PlayVoice(unsigned char Index)
{
	TOVOX=1;TOFEE=0;TOCPU=0;

	FeedDog();
	if(Index%5)
	{
		comm_send_char(0x02);
		comm_send_char('5');
		comm_send_char('1');
		comm_send_char('1');
		comm_send_char('2');
		comm_send_char('3');
		comm_send_char('4');
		comm_send_char('4');
		comm_send_char('3');
		comm_send_char('2');
		comm_send_char('1');
		comm_send_char(0xBB);
		comm_send_char(0xBC);
		comm_send_char(0xCC);
		comm_send_char(0xCD);
		comm_send_char(0xDD);
		comm_send_char(0xDE);
		comm_send_char(0x03);
		comm_send_char(0x01);
	}
	else
	{
		comm_send_char(0x02);
		comm_send_char('Y');
		comm_send_char(0x03);
		comm_send_char('Y');
	}

	TOVOX=0;TOFEE=0;TOCPU=1;
}
//================================================================================================
SetControlPort(bit bOnOff)
{
	unsigned char tmpPortData;

	FeedDog();
	P0=0xFE;
	for(tmpPortData=0;tmpPortData<16;tmpPortData++)
	{
		P0<<=1;
		delay_ms(10);
	}
	delay_ms(200);
	delay_ms(200);
	
	tmpPortData=0xFF;
	if(bOnOff) tmpPortData=0;	//set on

	P0=tmpPortData;
	delay_ms(2);
	P0=tmpPortData|0x03;
}
//================================================================================================
void DelayLong(unsigned char Times)
{
	static int i;

	FeedDog();
	for(i=0;i<Times;i++)
	{
		delay_ms(200);
	 }
}
//================================================================================================
void TestForLC301(void)
{
	unsigned char Index=0;
	if(!InputDedect()) return;

	FeedDog();
	while(1)
	{
		WDT=1;
        WDT=0;
		UpdateFeeDisplay(Index);
		PlayVoice(Index);
		SetControlPort(1);

		DelayLong(10);

		SetControlPort(0);
		
		delay_ms(50);
		Index++;
	}
	return;
}

//================================================================================================

