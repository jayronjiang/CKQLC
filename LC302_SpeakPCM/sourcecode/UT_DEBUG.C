

/*--------------------------------------------------------------------------
__UT_DEBUG__

通过串口输出调试信息
弃用 printf 等函数
--------------------------------------------------------------------------*/
#include "REG51.h"
#include "stc12l5612.h"
#include "UT_DEBUG.H"

/////////////////////////////////////////
#ifdef __UT_DEBUG__
//////////////////////////////////////////


/************** _INIT_ **************/
void debug_init()
{
	SCON  = 0x50;             /* 0101 0000, mode 1: 8-bit、no parity UART, enable receiver*/ 
/*	PCON |= 0x80; */		   /* double baud-rate */ 

	TMOD |= 0x20;            /* timer 1 mode 2: 8-Bit reload */ 
    TH1 = 0xfA;  
    TL1 = 0xfA;
	 
	AUXR|=0x40;         	/*T1x12=1,timer 1 at 1T ----115200BPS */  
/*	AUXR&=~0x40;*/   		/*12T----9600BPS  */ 

	ET1	= 0;		   	/* timer 1 interrupt disable */	 
    TR1 = 1;  

	ES= 0;               /* disable serial port interrupt */ 
}					

/**************_PUTS_ **************/
/**/
#define UART_TX_CH(c) TI=0;SBUF=c;while(!TI);TI=0
/**/
void debug_puts(const char *str)
{
	unsigned ch;

	ch = *str ;
	while(ch)
	{
		UART_TX_CH(ch);		 

		str++;
		ch = *str ;
	}
}

/**************_PRINTF_ **************/
void debug_printf_digit (const char *fmtstr, unsigned long num )
{
	unsigned char c;

	unsigned long carry;
	unsigned char start;

	c=*fmtstr;
	while( c )
	{
		if(c=='%')
		{
			c = *(fmtstr+1);
			if(c=='d') goto PRINT_DECIMAL;
			if(c=='x'||c=='X')goto PRINT_HEX;
			c=*fmtstr;
		}
		UART_TX_CH( c );
NEXT_CHAR:
		fmtstr++;
		c=*fmtstr;
	}
	return;

// 十进制输出
PRINT_DECIMAL:
	carry=1000000;
	start=0;
	
	if(num==0) { 
		UART_TX_CH(0x30); 	} 
	else
		while(carry)
		{
			c = num/carry;
			if(start || c ) 
			{
				start = 1;
				UART_TX_CH(0x30+c);
				num %= carry;
			}
			carry /= 10;
		}

	fmtstr++;
	goto NEXT_CHAR;

// 16进制输出
PRINT_HEX:
	#define H2C(x) ((x<0xa)? (0x30+x) : (0x41-0xa+x))

	c = (num>>4*7)&0xf;  UART_TX_CH(H2C(c));
	c = (num>>4*6)&0xf;  UART_TX_CH(H2C(c));
	c = (num>>4*5)&0xf;  UART_TX_CH(H2C(c));
	c = (num>>4*4)&0xf;  UART_TX_CH(H2C(c));
	c = (num>>4*3)&0xf;  UART_TX_CH(H2C(c));
	c = (num>>4*2)&0xf;  UART_TX_CH(H2C(c));
	c = (num>>4*1)&0xf;  UART_TX_CH(H2C(c));
	c = (num>>4*0)&0xf;  UART_TX_CH(H2C(c));

	fmtstr++;
	goto NEXT_CHAR;
}


////////////////////////////////////////////
/////////////////////////////////////////
#endif //#ifdef __UT_DEBUG__
//////////////////////////////////////////

