
#include<REG52.H> 

/*
//#include <INTRINS.H>    
//#include <ABSACC.H>  
//#include <STRING.H>
//#include <NO-STDIO.H>
*/

#include "stc12l5612.h"
#include "PCA.H"
#include "W25X.H"
#include "SpeakVox.h"
#include "ATE_Shell.h"

#include "delay.h"
#include "ut_debug.h"

#include"speakctl.h"
#include "uart.h"
  
 unsigned int T0_count;

///////////////////////////////////////////////////////////////////////////
/********************************/
unsigned char code VERSION_NO[]="SpeakPCM V1.1.0 for LC301.";
/********************************/
void main()
{
	#if 0
	unsigned long ulCCC = 1000;
	if(ulCCC<1024*100)
	((uchar*)&ulCCC)[0] = 0;
	((uchar*)&ulCCC)[1] = 1;
	((uchar*)&ulCCC)[2] = 2;
	((uchar*)&ulCCC)[3] = 3;
	#endif

    IE = 0;   
    P0 = 0xFF;   
    P1 = 0xFF;   
    P2 = 0xFF;   
    P3 = 0xFF;   
//    P4 = 0xFF;   
	P3M0=0x00;	//标准8051准双向口
	P3M1=0x00;	//

	P2M0=0x00;	//配置为标准8051准双向口
	P2M1=0x00;

	P1M0=0x00;	//配置为标准8051准双向口
	P1M1=0x00;

	P0M0=0x00;	//配置为标准8051准双向口
	P0M1=0x00;
   
	EA = 1;

	led2 = 1;		 //关指示灯LED1、LED2
	led1 = 1;		 //
	P33 = 0;

	DEBUG_INIT_();
#if 0 
	// ATE部分代码的调试（SPI、PCA未能用）
	ATE_init();
	ATE_main();

	DEBUG_PUTS_("sleep 1s\n");
	SleepMs(1000);

	DEBUG_PUTS_("sleep 5s\n");
	SleepMs(5000);

	DEBUG_PUTS_("sleep 10s\n");
	SleepMs(10000);

	DEBUG_PUTS_("sleep end\n");
#endif

	ATE_init();
	SPI_init();
	PCA_init();

	DEBUG_PUTS_("Start up....\n");

  // 用内置的语音样本测试PCA
   PCA_Test_SampleVox();   DEBUG_PUTS_("Beep....\n");
   PCA_Test_SampleVox();   DEBUG_PUTS_("Beep....\n");
   PCA_Test_SampleVox();   DEBUG_PUTS_("Beep....\n");

#if 0
	// 测试flash中所有语音（调试时用）
   led2=1;
   Test_All_Vox_ID();
   DEBUG_PUTS_("All Vox Test Done!\n");
#endif

	// 尝试进入异步终端，下载flash、测试......
	led2=0;							//编程指示灯亮
	ATE_main();
	led2=1;					  		//编程指示灯灭

	// 测试Flash中的‘你好’
	Vox_PlayList_Add( ID_NiHao );
	Vox_Wait_AllPlayDone();
	led1=1;					  		//语音指示灯灭

	uart_int();
	T0_int();
	// 主循环
	while(1)
	{
	  	if(TF0)						  //定时器溢出查询
		{
			TF0=0;
			T0_count++;
			UART_timeout_count++;
			if(T0_count==0xffff)
			{	
				T0_count=0;
				fThanksSpeaked=0;
			}
		 	if(UART_timeout_count==6)
			{
				UART_timeout_count=0;
				RESET_RX;
			}					
				// 重装定时器
			TF0=0;
			TR0=0;
			TL0=0;   
			TH0=0;       
			TR0=1;
		}
		if(comm_Stored)				   //串口有数据更新
		{
			comm_Stored=0;
			Vox_Dismiss_Play();
			Dataprocess();
		}
		Vox_Prepare_PlayCode();			//播放语音
	}
}

