#include<REG52.H> 
#include "stc12l5612.h"
#include "uart.h"

#define BAUDRELOAD 0xfA             //波特率采用1T--115200bps 	12T--9600bps 
#define COMM_BUFF_LEN 20

/* 接收缓冲区 */
unsigned char idata comm_buff[COMM_BUFF_LEN];
/* 缓冲区读写指针 */
unsigned char data comm_buff_ptr = 0;
unsigned char UART_timeout_count = 0;
/* 检验和 */
unsigned char comm_bcc = 0;
/* 接收标志 */
bit bdata comm_RStart = 0;	//接收到包开始标志
bit bdata comm_REnd = 0;	//接收到包结束标志

/* 完成标志 */
bit bdata comm_Stored = 0;	//数据接收完毕，数据包可用




/////////////////////////////////////////////////////////////////////////
///串口初始化
void uart_int()
{
	SCON  = 0x50;                      // mode 1: 8-bit UART, enable receiver
	TMOD |= 0x20;                      // timer 1 mode 2: 8-Bit reload
    TH1 = BAUDRELOAD;   			   //定时器1作串行口波特率发生器
    TL1 = BAUDRELOAD;
//	AUXR|=0x40;            			//1T ----115200BPS
    AUXR&=~0x40;  		            /*12T----9600BPS  */ 
	TR1 = 1;    					//启动定时器
	ET1	= 0;						// timer 1 interrupt disable
	TI = 0;
	RI = 0;
	ES = 1;                         // enable serial port interrupt
	EA = 1;
}


///////////////////////////////////////////////////////////////////////
///

//////////////////////////////////////////////////////////////////////
//  串口中断程序
//  接收格式：0x02 + 数据(nbyte) + 0x03 + BCC(1byte)

void UART_Isr() interrupt 4	//using 1
{
	unsigned char temp;
	if(RI)
	{
		UART_timeout_count = 0;
		RI = 0;
		temp = SBUF;
		
		if(temp==0x02&&(!comm_REnd))	// 包头   
		{
			RESET_RX;
			comm_RStart = 1;
			return;
		}
		if( temp==0x03&&comm_RStart&&(!comm_REnd) )	   // 包尾
		{
			comm_REnd = 1;			
			return;
		}

		if( comm_REnd && comm_RStart )	// 已经收到头和尾，接着是校验位
		{
			if(comm_bcc == temp) // 校验正确，收到完整的包，
			{
				comm_Stored = 1; 
			}
			RESET_RX;
			return;
		}


		comm_buff[comm_buff_ptr] = temp;   //填写缓存，注意：包头、包尾、校验位都不在缓存内，仅仅包的内容
		comm_buff_ptr++;	

		comm_bcc = comm_bcc^temp; 	// 计算除了包头、包尾、校验位之外的和

		if(comm_buff_ptr==COMM_BUFF_LEN)	// 缓冲区溢出
		{
			RESET_RX;
		}
	}
	else
		TI=0;
}

///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 定时器0初始化
void T0_int()
{	
	TMOD |= 0x01;                      // timer0 mode 1: 16-Bit
	TMOD&=~0x03; 	// 初始化定时器0的C/T、M1、M0=0
    AUXR&=~0x80;  		//T0x12 为0  
  	TL0 = 0;   
	TH0 = 0;       
 	TF0 = 0;				//清T0中断标志
	TR0 = 1;
  	ET0 = 0;
}

