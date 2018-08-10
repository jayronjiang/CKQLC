
/****************************************************************************/   
   
/****************************************************************************/   
#include <REG52.H>   
#include "W25X.H"
#include "W25X_note.H"     
#include "stc12l5612.h"

//////////////////////////////////////////////////////////////

sbit	W25X_WP	= P1^3;    //NC
sbit	W25X_CS	= P3^4;    //P1^4; 
//sbit	W25X_DI	= P1^5; 
//sbit	W25X_DO	= P1^6; 
sbit	W25X_CLK= P1^7; 

   
//=================================================================================================    
#define SPI_PAGESIZE 256              //SPI的一个Flash页为256字节(Xmodem-128字) 

uchar xdata FLASH_ext_bufx[SPI_PAGESIZE /2 ] _at_ 0;

//================================================================================================

#ifdef _OPTIONAL_CODE  	 /* optional use */

void    delay_nms(uchar i)   
{   
	uchar  j;   
    i=i*2;   
    for(;i>0;i--)    {   j = 246;    while(--j); }   
}
#endif
   
void    delay(uchar tt)   
{   
	while(tt--);
}  
 

//=================================================================================================    
//SPI_Read_StatusReg        Reads the status register of the serial flash    
//SPI_Write_StatusReg       Performs a write to the status register    
//SPI_Write_Enable          Write enables the serial flash    
//SPI_Write_Disable         Write disables the serial flash    
//SPI_Read_ID1              Reads the device ID using the instruction 0xAB    
//SPI_Read_ID2              Reads the manufacturer ID and device ID with 0x90    
//SPI_Read_ID3()            Reads the JedecDevice ID    
//SPI_Read_Byte             Reads one byte from the serial flash and returns byte(max of 20 MHz CLK frequency)    
//SPI_Read_nBytes           Reads multiple bytes(max of 20 MHz CLK frequency)    
//void SPI_Read_Start(uint32 Dst_Addr)
//void SPI_Read_End(void)

//SPI_FastRead_Byte         Reads one byte from the serial flash and returns byte(max of 33 MHz CLK frequency)    
//SPI_FastRead_nBytes       Reads multiple bytes(max of 33 MHz CLK frequency)    
//SPI_Write_Byte            Program one byte to the serial flash    
//SPI_Write_nBytes          Program n bytes to the serial flash, n<=256    
//SPI_Erase_Chip            Erases entire serial flash    
//SPI_Erase_Sector          Erases one sector (64 KB) of the serial flash    
//SPI_Wait_Busy             Polls status register until busy bit is low    
//=================================================================================================    
uchar   SPI_Read_StatusReg()   
{   
	uchar byte = 0;   
    
	W25X_WP = 1;  		   //写保护置1，允许读写操作
	W25X_CS = 0;                            //  enable device    
    SPI_Send_Byte(W25X_ReadStatusReg);      //  send Read Status Register command    
    byte = SPI_Get_Byte();                  //  receive byte    
    W25X_CS = 1;                            //  disable device      
	W25X_WP = 0;
    return byte;   
}
#if 0   
void    SPI_Write_StatusReg(byte)   
{   
	W25X_WP = 1;
	W25X_CS = 0;                            //  enable device    
    SPI_Send_Byte(W25X_WriteStatusReg);     //  select write to status register    
    SPI_Send_Byte(byte);                    //  data that will change the status(only bits 2,3,7 can be written)    
    W25X_CS = 1;                            //  disable the device    
	W25X_WP = 0;
}
#endif   

void    SPI_Write_Enable()   
{   
	W25X_WP = 1;
	W25X_CS = 0;                            //  enable device    
    SPI_Send_Byte(W25X_WriteEnable);        //  send W25X_Write_Enable command    
    W25X_CS = 1;                            //  disable device    
	W25X_WP = 0;
}   
void    SPI_Write_Disable()   
{   
	W25X_WP = 1;
	W25X_CS = 0;                            //  enable device    
    SPI_Send_Byte(W25X_WriteDisable);       //  send W25X_WriteW25X_DIsable command    
    W25X_CS = 1;                            //  disable device    
	W25X_WP = 0;
}

///////////////////
   
uchar   SPI_Read_ID1()   
{   
	uchar byte;   
    
	W25X_WP = 1;
	W25X_CS = 0;                            //  enable device    
    SPI_Send_Byte(W25X_DeviceID);           //  send read device ID command (ABh)    
    SPI_Send_Byte(0);                       //  send address    
    SPI_Send_Byte(0);                       //  send address    
    SPI_Send_Byte(0);                       //  send 3_Dummy address    
    byte = SPI_Get_Byte();                  //  receive Device ID byte      
    W25X_CS  = 1;                           //  disable device    
    delay(4);                               //  remain CS high for tRES2 = 1.8uS    
	W25X_WP = 0;
    return byte;   
}   
uint16   SPI_Read_ID2(uchar ID_Addr)   
{   
	uint16 IData16;   
    
	W25X_WP = 1;
	W25X_CS = 0;                            //  enable device    
    SPI_Send_Byte(W25X_ManufactDeviceID);   //  send read ID command (90h)    
    SPI_Send_Byte(0x00);                    //  send address    
    SPI_Send_Byte(0x00);                    //  send address    
    SPI_Send_Byte(ID_Addr);                 //  send W25Pxx selectable ID address 00H or 01H    
    IData16 = SPI_Get_Byte()<<8;          //  receive Manufature or Device ID byte    
    IData16 |= SPI_Get_Byte();              //  receive Device or Manufacture ID byte    
    W25X_CS = 1;                            //  disable device      
	W25X_WP = 0;
    return IData16;   
}   
uint32   SPI_Read_ID3()   
{   
	uint32 IData32=0;   
    
	W25X_WP = 1;
	W25X_CS = 0;                            //  enable device    
    SPI_Send_Byte(W25X_JedecDeviceID);      //  send read ID command (9Fh)    
    IData32 = SPI_Get_Byte()<<(8+8);          //  receive Manufature or Device ID byte    
    IData32 |= (SPI_Get_Byte()<<8);              //  receive Device or Manufacture ID byte    
    IData32 |=  SPI_Get_Byte();       
    W25X_CS = 1;                            //  disable device      
	W25X_WP = 0;
    return IData32;   
} 

//////////////////////////////////

#ifdef _OPTIONAL_CODE  	 /* optional use */

uchar   SPI_Read_Byte(uint32 Dst_Addr)    
{   
	uchar byte = 0;    
    
	W25X_WP = 1;
	W25X_CS = 0;                                        //  enable device    
    SPI_Send_Byte(W25X_ReadData);                       //  read command    
    SPI_Send_Byte((uchar)((Dst_Addr & 0xFFFFFF) >> 16));//    send 3 address bytes    
    SPI_Send_Byte((uchar)((Dst_Addr & 0xFFFF) >> 8));   
    SPI_Send_Byte((uchar)(Dst_Addr & 0xFF));   
    byte = SPI_Get_Byte();   
    W25X_CS = 1;                                        //  disable device      
	W25X_WP = 0;
    return byte;                                        //  return one byte read    
}
#endif   

void    SPI_Read_nBytes(uint32 Dst_Addr, uchar nBytes_128)   
{   
	uchar i = 0;      

	W25X_WP = 1;
    W25X_CS = 0;                                        //  enable device    
    SPI_Send_Byte(W25X_ReadData);                       //  read command    
    SPI_Send_Byte(((Dst_Addr & 0xFFFFFF) >> 16));     //  send 3 address bytes    
    SPI_Send_Byte(((Dst_Addr & 0xFFFF) >> 8));   
    SPI_Send_Byte(Dst_Addr & 0xFF);   
    for (i = 0; i < nBytes_128; i++)                 //  read until no_bytes is reached    
        FLASH_ext_bufx[i] = SPI_Get_Byte();                  //  receive byte and store at address 80H - FFH    
    W25X_CS = 1;                                        //  disable device    
	W25X_WP = 0;
}


void SPI_Read_Start(uint32 Dst_Addr)
{     
	W25X_WP = 1;
    W25X_CS = 0;                                        //  enable device    
    SPI_Send_Byte(W25X_ReadData);                       //  read command    
    SPI_Send_Byte(((Dst_Addr & 0xFFFFFF) >> 16));     //  send 3 address bytes    
    SPI_Send_Byte(((Dst_Addr & 0xFFFF) >> 8));   
    SPI_Send_Byte(Dst_Addr & 0xFF);   
}

void SPI_Read_End(void)
{   
	W25X_CS = 1;
	W25X_WP = 0;
}

#ifdef _OPTIONAL_CODE  	 /* optional use */
  
uchar   SPI_FastRead_Byte(uint32 Dst_Addr)    
{   
	uchar byte = 0;   

	W25X_WP = 1;
    W25X_CS = 0;                                        //  enable device    
    SPI_Send_Byte(W25X_FastReadData);                   //  fast read command    
    SPI_Send_Byte(((Dst_Addr & 0xFFFFFF) >> 16));     //  send 3 address bytes    
    SPI_Send_Byte(((Dst_Addr & 0xFFFF) >> 8));   
    SPI_Send_Byte(Dst_Addr & 0xFF);   
    SPI_Send_Byte(0xFF);                                //  dummy byte    
    byte = SPI_Get_Byte();   
    W25X_CS = 1;                                        //  disable device      
	W25X_WP = 0;
    return byte;                                        //  return one byte read    
}   
void    SPI_FastRead_nBytes(uint32 Dst_Addr, uchar nBytes_128)   
{   
	uchar i = 0;       

	W25X_WP = 1;
    W25X_CS = 0;                                        //  enable device    
    SPI_Send_Byte(W25X_FastReadData);                   //  read command    
    SPI_Send_Byte(((Dst_Addr & 0xFFFFFF) >> 16));     //  send 3 address bytes    
    SPI_Send_Byte(((Dst_Addr & 0xFFFF) >> 8));   
    SPI_Send_Byte(Dst_Addr & 0xFF);   
    SPI_Send_Byte(0xFF);                                //  dummy byte    
    for (i = 0; i < nBytes_128; i++)                 //  read until no_bytes is reached    
        FLASH_ext_bufx[i] = SPI_Get_Byte();                  //  receive byte and store at address 80H - FFH    
    W25X_CS = 1;                                        //  disable device    
	W25X_WP = 0;
} 
#endif

#ifdef _OPTIONAL_CODE  	 /* optional use */
  
void    SPI_Write_Byte(uint32 Dst_Addr, uchar byte)   
{   
	W25X_WP = 1;
	W25X_CS = 0;                                    //  enable device    
    SPI_Write_Enable();                             //  set WEL    
    SPI_Wait_Busy();       
    W25X_CS = 0;       
    SPI_Send_Byte(W25X_PageProgram);                //  send Byte Program command    
    SPI_Send_Byte(((Dst_Addr & 0xFFFFFF) >> 16)); //  send 3 address bytes    
    SPI_Send_Byte(((Dst_Addr & 0xFFFF) >> 8));   
    SPI_Send_Byte(Dst_Addr & 0xFF);   
    SPI_Send_Byte(byte);                            //  send byte to be programmed   
	
	W25X_CS = 1;						   //  disable device
	W25X_WP = 0;
	
	SPI_Write_Disable();
                                        
}  
#endif
 
void    SPI_Write_nBytes(uint32 Dst_Addr, uchar nBytes_128)   
{      
    uchar byte;
	uchar i;     

	W25X_WP = 1;
    W25X_CS = 0;                    /* enable device */   
    SPI_Write_Enable();             /* set WEL */   
    W25X_CS = 0;   
    SPI_Send_Byte(W25X_PageProgram);        /* send Byte Program command */   
    SPI_Send_Byte(((Dst_Addr & 0xFFFFFF) >> 16)); /* send 3 address bytes */   
    SPI_Send_Byte(((Dst_Addr & 0xFFFF) >> 8));   
    SPI_Send_Byte(Dst_Addr & 0xFF);   
       
    for (i = 0; i < nBytes_128; i++)   
    {   
        byte = FLASH_ext_bufx[i];   
        SPI_Send_Byte(byte);        /* send byte to be programmed */   
    }     

    W25X_CS = 1;                /* disable device */   
	W25X_WP = 0;
	
	SPI_Write_Disable();
 
}   

#ifdef _OPTIONAL_CODE  	 /* optional use */
/*
For optimized timings, it is recommended to use the page write (PW) instruction.
*/
void    SPI_Write_Page(uint32 Dst_Addr)   
{      
    uchar byte;
	uint16 i;     

	W25X_WP = 1;
    W25X_CS = 0;                    /* enable device */   
    SPI_Write_Enable();             /* set WEL */   
    W25X_CS = 0;   
    SPI_Send_Byte(W25X_PageProgram);        /* send Byte Program command */   
    SPI_Send_Byte(((Dst_Addr & 0xFFFFFF) >> 16)); /* send 3 address bytes */   
    SPI_Send_Byte(((Dst_Addr & 0xFFFF) >> 8));   
    SPI_Send_Byte(Dst_Addr & 0xFF);   
       
    for (i = 0; i < 256; i++)   
    {   
        byte = FLASH_ext_bufx[i];   
        SPI_Send_Byte(byte);        /* send byte to be programmed */   
    }
	      
    W25X_CS = 1;                /* disable device */   
	W25X_WP = 0;

	SPI_Write_Disable();

}   
#endif

//////////////////////////

void    SPI_Erase_Chip()   
{   
	W25X_WP = 1;
    W25X_CS = 0;                                        //  enable device    
    SPI_Write_Enable();                                 //  set WEL    
    W25X_CS = 0;   
    SPI_Wait_Busy();   
    W25X_CS = 0;   
    SPI_Send_Byte(W25X_ChipErase);                      //  send Chip Erase command    
    W25X_CS = 1;
	SPI_Wait_Busy();
	W25X_WP = 0;
}
#ifdef _OPTIONAL_CODE  	 /* optional use */   
void    SPI_Erase_Sector(uint32 Dst_Addr)   
{   
	W25X_WP = 1;  		   //写保护置1，允许读写操作
	W25X_CS = 0;                                        //  enable device    
    SPI_Write_Enable();                                 //  set WEL    
    W25X_CS = 0;   
    SPI_Send_Byte(W25X_SectorErase);                    //  send Sector Erase command    
    SPI_Send_Byte((uchar)((Dst_Addr & 0xFFFFFF) >> 16));//    send 3 address bytes    
    SPI_Send_Byte((uchar)((Dst_Addr & 0xFFFF) >> 8));   
    SPI_Send_Byte((uchar)Dst_Addr & 0xFF);   
    W25X_CS = 1;                                        //  disable device    
	W25X_WP = 0;
}
#endif   
void    SPI_Erase_Block(uint32 Dst_Addr)   
{   
	W25X_WP = 1;  		   //写保护置1，允许读写操作
	W25X_CS = 0;                                        //  enable device    
    SPI_Write_Enable();                                 //  set WEL    
    W25X_CS = 0;   
    SPI_Send_Byte(W25X_BlockErase);                    //  send Sector Erase command    
    SPI_Send_Byte((uchar)((Dst_Addr & 0xFFFFFF) >> 16));//    send 3 address bytes    
    SPI_Send_Byte((uchar)((Dst_Addr & 0xFFFF) >> 8));   
    SPI_Send_Byte((uchar)Dst_Addr & 0xFF);   
    W25X_CS = 1;                                        //  disable device    
	W25X_WP = 0;
}   

//////////////////////////////////////

void    SPI_Wait_Busy()   
{   
	while (SPI_Read_StatusReg() == 0x03)   
           /*SPI_Read_StatusReg()*/;   //  waste time until not busy WEL & Busy bit all be 1 (0x03)    
} 

////////////////////////

#ifdef _OPTIONAL_CODE  	 /* optional use */
  
void    SPI_PowerDown()   
{   
	W25X_WP = 1;  		   //写保护置1，允许读写操作	
	W25X_CS = 0;                            //  enable device    
    SPI_Send_Byte(W25X_PowerDown);          //  send W25X_PowerDown command 0xB9    
    W25X_CS = 1;                            //  disable device    
    delay(6);                               //  remain CS high for tPD = 3uS    
	W25X_WP = 0;
}   
void    SPI_ReleasePowerDown()   
{   
	W25X_WP = 1;  		   //写保护置1，允许读写操作
	W25X_CS = 0;                            //  enable device    
    SPI_Send_Byte(W25X_ReleasePowerDown);   //  send W25X_PowerDown command 0xAB    
    W25X_CS = 1;                            //  disable device    
    delay(6);                               //  remain CS high for tRES1 = 3uS    
	W25X_WP = 0;
}
#endif

uchar   SPI_M25PE_RDLR(uint32 Dst_Addr)    
{   
	uchar byte = 0;   

	W25X_WP = 1;  		   //写保护置1，允许读写操作    
	W25X_CS = 0;                                        //  enable device    
    SPI_Send_Byte(M25PE_RDLR);                   //  fast read command    
    SPI_Send_Byte(((Dst_Addr & 0xFFFFFF) >> 16));     //  send 3 address bytes    
    SPI_Send_Byte(((Dst_Addr & 0xFFFF) >> 8));   
    SPI_Send_Byte(Dst_Addr & 0xFF);   
//    SPI_Send_Byte(0xFF);                                //  dummy byte    
    byte = SPI_Get_Byte();   
    W25X_CS = 1;                                        //  disable device      
	W25X_WP = 0;
    return byte;                                        //  return one byte read    
}   
   
#ifdef _OPTIONAL_CODE  	 /* optional use */

void   SPI_M25PE_WRLR(uint32 Dst_Addr,uchar byte)    
{   
	W25X_WP = 1;  		   //写保护置1，允许读写操作
	W25X_CS = 0;                                        //  enable device    
    SPI_Write_Enable();                                 //  set WEL    
	W25X_CS = 0;                                        //  enable device    
    SPI_Send_Byte(M25PE_WRLR);                   //  fast read command    
    SPI_Send_Byte(((Dst_Addr & 0xFFFFFF) >> 16));     //  send 3 address bytes    
    SPI_Send_Byte(((Dst_Addr & 0xFFFF) >> 8));   
    SPI_Send_Byte(Dst_Addr & 0xFF);   
//    SPI_Send_Byte(0xFF);                                //  dummy byte    
    SPI_Send_Byte(byte);                                //  dummy byte    
    W25X_CS = 1;                                        //  disable device      
	W25X_WP = 0;
}   
#endif
////////////////////////////////////

void    SPI_init()   
{
	W25X_CS = 1;	
	W25X_CLK = 1;          //  set clock to High initial state for SPI operation mode 3 
    W25X_WP = 0;  		   //写保护置0，不允许读写操作
 //   W25X_WP = 1;  		   //写保护置1，允许读写操作

	AUXR &= (~ESPI);	// disable interrupt
	EADC_SPI = 0;		// IE.5 EADC_SPI

	SPDAT = 0;			//clear SPI data
	SPSTAT = SPIF|WCOL;	//clear SPI status	  // 软件写入‘1’清零

	SPCTL = SPEN|SSIG|MSTR | /*CPOL|CPHA |*/ SPR_DIV4;	// - master mode, MSB
	//
	// 用 | /*CPOL|*/CPHA   --fail ID3:  00000000,写、擦失败，读存储内容为0
	//	  | CPOL/*|CPHA*/	--ok ID3:  FFFF8015
	//	  | /*CPOL|CPHA |*/ -- ok
	//	  | CPOL|CPHA   -----ok

//	SPI_Write_Disable();
}   

void    SPI_Send_Byte(uchar dat)   
{
//	SPISS = 0;		// pull low slave SS

	SPDAT = dat;	//trigger SPI send

	while( !(SPSTAT&SPIF) );// wait send complete

	SPSTAT = SPIF|WCOL;	//clear SPI status

//	SPISS = 1;	// pull high slave SS
}   

uchar   SPI_Get_Byte()   
{
//	SPISS = 0;		// pull low slave SS

	SPDAT = 0;	//trigger SPI 	 ???

	while( !(SPSTAT&SPIF) ) ;// wait  complete

	SPSTAT = SPIF|WCOL;	//clear SPI status

//	SPISS = 1;	// pull high slave SS
 
    return SPDAT;   
} 
////

/////////////////////////////////////////////
