

#include <REG52.H>

#include "stc12l5612.h"
#include "delay.h"

#include "w25x.h"
#include "PCA.H"
#include "SpeakVox.h"

#include "ATE_Shell.h"

////////////////////////////////////////////////////

extern unsigned char code VERSION_NO[];

/* 用于xmodem的回调 引用*/

unsigned char *xmodem_buff; 
unsigned char xm_flush_rx_record(unsigned char len);
unsigned char xm_load_tx_record(  );

/**/
#define XMODEM_XMIT_SIZE 128

extern unsigned char xmodemReceive();
extern unsigned char xmodemTransmit();

//////////////////////////////////////////////////
/*Flash 缓冲引用*/
extern uchar xdata FLASH_ext_bufx[];

////////////////////////////////////////////////////

unsigned long FLASH_addr;

unsigned char cmd_char[6] = "";

// function-prototype
void ATE_Flash_ListStatus();
void ATE_Flash_EraseBlock();
void ATE_Flash_EraseChip();
void ATE_Flash_WriteDown();
void ATE_Flash_ReadUp();
void ATE_Flash_VoxInfoHead();
void ATE_Play_Sound();  

///////////////////////////////////////////////////////////////////////////////

#define BAUDRELOAD 0xfA             //波特率采用1T--115200bps 	12T--9600bps

///////////////////////////////////////////////////////////////////////////////

void ATE_init()
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

////////////////////////////////////////////////////////////////////////////////

#define UL_LOOP_MS (TIME_SPLIT_1MS)/6

// 从串口接收一个字符    
void serial_get_char(unsigned char tns)
{   
    unsigned long ulTmo;
	
	RI=0;
    while(tns--)   
    {   
        for (ulTmo=0; ulTmo<1000*UL_LOOP_MS; ulTmo++)   
        {   
            if (RI)
			{
				cmd_char[0] = SBUF;   
    			RI=0;
				return;
			}   
        }   
    }
	cmd_char[0] = 0x00;   
}
// 从串口接收以回车（0x0D）为结束符的命令字符
void serial_get_cmd(unsigned char tns)
{   
    unsigned long ulTmo;
	unsigned char i = 0;
	
	RI=0;
    while(tns--)   
    {   
        for (ulTmo=0; ulTmo<1000*UL_LOOP_MS; ulTmo++)   
        {   
            if (RI)
			{
				cmd_char[i] = SBUF;   
    			RI=0;
				if( cmd_char[i] == '\r' || i==(sizeof(cmd_char)-1) )
				{
					 cmd_char[i] = 0 ;
					 return;
				}
				i++;
			}   
        }   
    }
	cmd_char[0] = 0x00;   
}
   
// 向串口发送字符串 
#define UART_TX_CH(c) TI=0;SBUF=c;while(!TI);TI=0

void serial_put_chars(unsigned char *cstr)   
{   
	unsigned ch;

	ch = *cstr ;
	while(ch)
	{
		UART_TX_CH(ch);
		
		cstr++;
		ch = *cstr ;
	} 
}

void serial_printf_digit (const char *fmtstr, unsigned long num )
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
		UART_TX_CH(0x30); }
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
   
   
///////////////////////////////////////////////////////////////////////////////

// ATE - Asynchronous Terminal Emulation
void ATE_main()
{
	serial_put_chars("For enter ATE, Type 'c' and 'enter'.\n");
	serial_put_chars("Download data to flash, Type 't' and 'enter'.\n");
	serial_put_chars("(Please input command in 5s).\n");

	serial_get_cmd( 2 );
	if((cmd_char[0]!='c')&&(cmd_char[0]!='t'))
	{
		serial_put_chars("Good Bye!\r\n");
		return;
	}
	else if(cmd_char[0]=='t')
	{
		ATE_Flash_WriteDown();
		return;
	}


ATE_MENU:
	serial_put_chars("\n");
	serial_put_chars("***************************************\n");
	serial_put_chars("*  l --list SPI flash status          *\n");
	serial_put_chars("*  b --erase SPI flash blocks         *\n");
	serial_put_chars("*  c --erase SPI flash chip           *\n");
	serial_put_chars("*  r --readup from SPI flash memory   *\n");
	serial_put_chars("*  w --download data to SPI flash     *\n");
	serial_put_chars("*  h --show vox.inf in SPI flash      *\n");
	serial_put_chars("*  p --play sound                     *\n");
	serial_put_chars("*  v --version info                   *\n");
	serial_put_chars("*  q --quit                           *\n");
	serial_put_chars("***************************************\n");

	for(;;)
	{
		serial_get_char( 1 );
		switch( cmd_char[0] )
		{
		case 'l':		// --list SPI flash status 
			ATE_Flash_ListStatus();  goto ATE_MENU; 
			break;  
		case 'b':		// --erase SPI flash blocks 
			 ATE_Flash_EraseBlock(); goto ATE_MENU; 
			 break;       
		case 'c':		// --erase SPI flash chip 
			ATE_Flash_EraseChip(); goto ATE_MENU; 
			break;         
		case 'r':		// --readup from SPI flash memory   
			ATE_Flash_ReadUp(); goto ATE_MENU; 
			break;
		case 'w':		// --download to SPI flash 
			ATE_Flash_WriteDown(); goto ATE_MENU; 
			break;
		case 'h':		// - 语音信息头
			ATE_Flash_VoxInfoHead(); goto ATE_MENU;
			break;        
		case 'p':	// --play sound    
			ATE_Play_Sound(); goto ATE_MENU; 
			break;
		case 'v':	// --version    
			serial_put_chars("\n");serial_put_chars(VERSION_NO); goto ATE_MENU;
			break;                  
		case 'q':// --quit
			serial_put_chars("Good Bye!\r\n");
			return; 
		default:
			break;                         
		}
	}
} 

////////////////////////////////////////////////////////////////////////////

void ATE_Flash_ListStatus()
{
	serial_printf_digit("RDSR: %x\n", SPI_Read_StatusReg() );

	serial_printf_digit("ID1:  %x\n", SPI_Read_ID1() );  

	serial_printf_digit("ID2-0:%x  ", SPI_Read_ID2(0) );
	serial_printf_digit("ID2-1:%x\n", SPI_Read_ID2(1) );
	 
	serial_printf_digit("ID3:  %x\n", SPI_Read_ID3() );  
}

void ATE_Flash_EraseBlock()
{
	unsigned char i;

	serial_put_chars("Erase Flash Block ......\n");

	FLASH_addr=0;
	for(i=0;i<32;i++)	  // Unlock & sec
	{
		serial_put_chars("Erase...");
		serial_printf_digit(" block=%d", i);
		serial_printf_digit(" FLASH_addr=%x\n", FLASH_addr ); 
		
		serial_printf_digit("RDLR: %x\n", SPI_M25PE_RDLR(FLASH_addr) );

		SPI_Erase_Block(FLASH_addr);
		SPI_Wait_Busy();

		FLASH_addr+=0x10000;
	} 
	serial_put_chars("Erase Flash Block ......OK!\n");
}

void ATE_Flash_EraseChip()
{
	serial_put_chars("Erase Flash Chip ......\n");

	serial_printf_digit("RDSR : %x\n", SPI_Read_StatusReg() );

	SPI_Erase_Chip();
	serial_put_chars("Erase Flash Chip ......OK!\n");
}

///////////////////////////////////	 /////////////////////

/* 用于xmodem的回调函数*/
//接收
unsigned char xm_flush_rx_record(unsigned char len)
{
//	SPI_Write_Page(FLASH_addr);   // 写入flash
	SPI_Write_nBytes((FLASH_addr), len);
	FLASH_addr += len;

	return 0;
}
void ATE_Flash_WriteDown()			 //先擦除，再写入
{
	unsigned char c;
	serial_put_chars("\nPlease select file and wait...\n");
	
	SPI_Erase_Chip();

	serial_put_chars("Click send button to write flash.\n");

	xmodem_buff = FLASH_ext_bufx; 
//	xm_flush_rx_record = cb_xm_flush_rx_record;

	FLASH_addr = 0;
	c = xmodemReceive();

	SleepMs(250);
	if( c == 0)
		serial_put_chars("Flash Write Down ...Ok\n");
	else
		serial_put_chars("Flash Write Down ...Error\n");

	serial_printf_digit("Error Code = %d\n", c);

}

/* 用于xmodem的回调函数*/
// 发送
unsigned char xm_load_tx_record( )
{
//	if( FLASH_addr<1024*200 )  // 这句这样写不行？！
	if( FLASH_addr/1024<200 ) // 传200k
	{
		SPI_Read_nBytes(FLASH_addr, XMODEM_XMIT_SIZE);
		FLASH_addr += XMODEM_XMIT_SIZE;

		return XMODEM_XMIT_SIZE;
	}

	return 0;
}
void ATE_Flash_ReadUp()
{
	unsigned char c;

	xmodem_buff = FLASH_ext_bufx; 
//	xm_load_tx_record = cb_xm_load_tx_record;

	serial_put_chars("Flash read up ...\n");

	FLASH_addr = 0;
	c = xmodemTransmit();

	SleepMs(250);
	if( c == 0)
		serial_put_chars("Flash read up ...Ok\n");
	else
		serial_put_chars("Flash read up ...Error\n");

	serial_printf_digit("Error Code = %d\n", c);

}
 
 ////
void ATE_Flash_VoxInfoHead()
{  
	#define HEX(x) ((x<0xa)? (0x30+x) : (0x41-0xa+x))
	
	unsigned char i;
	unsigned char j;
	unsigned short a;
	unsigned char c;

	FLASH_addr = 0;

	for(j=0; j<3; j++)	 // total read=J*128
	{
		SPI_Read_Start(FLASH_addr);
		for(i=0; i<128; i++)
			FLASH_ext_bufx[i] = SPI_Get_Byte();
		SPI_Read_End();	
		FLASH_addr += 128;
	
		for(i=0; i<128; i++)
		{
			if(i%16==0) 
			{
				UART_TX_CH('\n');
				 
				a=j*128+i;
				c = (a>>4*3)&0xf;  UART_TX_CH( HEX(c) );
				c = (a>>4*2)&0xf;  UART_TX_CH( HEX(c) );
				c = (a>>4*1)&0xf;  UART_TX_CH( HEX(c) );
				c = (a>>4*0)&0xf;  UART_TX_CH( HEX(c) ); 

				UART_TX_CH(':'); UART_TX_CH(' ');
			}
			a=FLASH_ext_bufx[i];
			c = (a>>4*1)&0xf;  UART_TX_CH( HEX(c) );
			c = (a>>4*0)&0xf;  UART_TX_CH( HEX(c) );
			UART_TX_CH(' ');
		}
	}
}
 
///////////////////////////////////	 /////////////////////

void ATE_Play_Sound()
{
	unsigned char num, i;

PLAY_MENU:
	serial_put_chars("\n");
	serial_put_chars("***************************************\n");
	serial_put_chars("*  Input Vox-ID(form 0 to MAX_ID),     *\n");
	serial_put_chars("*  Or input A for all of them,         *\n");
	serial_put_chars("*  Or input B to backward,             *\n");
	serial_put_chars("*  And then press 'Enter'.             *\n");
	serial_put_chars("***************************************\n");

	for(;;)
	{
		serial_put_chars(".");

		serial_get_cmd( 0xff );
		if( cmd_char[0] == 'b' ) {return; }// --back
		if( cmd_char[0] == 'a' ) {Test_All_Vox_ID(); goto PLAY_MENU;}

		if( cmd_char[0] && cmd_char[0]<='9' )   // -- input a digit
		{
			num = 0;
			i = 0;
			while(cmd_char[i]) 	
			{
				num = num*10 + cmd_char[i]-0x30;
				i++	;
			}
			serial_printf_digit("Input number is:%d.\n", num);

			if(num>MAX_VOX_ID) 
			{ 
				serial_put_chars("Invalid ID.\n");
				goto PLAY_MENU; 
			}

			if(num==0)
				PCA_Test_SampleVox();
			else
			{
				Vox_PlayList_Add( num-1 );
				Vox_Wait_AllPlayDone();
			}
			serial_put_chars("Play done.\n");

			goto PLAY_MENU;
		}  // end if 'cmd_char is digit'
	}
}

///////////////////////////////////	 /////////////////////
///////////////////////////////////	 /////////////////////
