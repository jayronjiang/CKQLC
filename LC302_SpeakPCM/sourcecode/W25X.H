
//w25.h-----------------
#ifndef __W25_H__ 
#define __W25_H__ 

#define	uchar	unsigned char 
#define	uint8	unsigned char 
#define	uint16	unsigned int 
#define	uint32	unsigned long int

 
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 

#define M25PE_RDLR              0xE8      //read lock register	   1 instruction 3 address  out 1 data
#define M25PE_WRLR              0xE5      //write to lock register
 
#define nop() _nop_() 

 
uchar	SPI_Read_StatusReg(); 
void	SPI_Write_StatusReg(byte); 
void	SPI_Write_Enable(); 
void	SPI_Write_Disable(); 
uchar	SPI_Read_ID1(); 
uint16	SPI_Read_ID2(uchar ID_Addr); 
uint32	SPI_Read_ID3(); 
uchar	SPI_Read_Byte(uint32 Dst_Addr);  
void	SPI_Read_nBytes(uint32 Dst_Addr, uchar nBytes_128); 
uchar	SPI_FastRead_Byte(uint32 Dst_Addr);  
void	SPI_FastRead_nBytes(uint32 Dst_Addr, uchar nBytes_128); 
void	SPI_Write_Byte(uint32 Dst_Addr, uchar byte); 
void	SPI_Write_nBytes(uint32 Dst_Addr, uchar nBytes_128); 

void 	SPI_Read_Start(uint32 Dst_Addr);
void 	SPI_Read_End(void);
void    SPI_Write_Page(uint32 Dst_Addr);

void	SPI_Erase_Chip();
void    SPI_Erase_Block(uint32 Dst_Addr); 
void	SPI_Erase_Sector(uint32 Dst_Addr); 
void	SPI_Wait_Busy(); 
void	SPI_PowerDown(); 
void	SPI_ReleasePowerDown(); 

void	SPI_init(); 
void	SPI_Send_Byte(uchar out); 
uchar	SPI_Get_Byte(); 
void	delay_nms(uchar i); 
void	delay(uchar tt);

uchar   SPI_M25PE_RDLR(uint32 Dst_Addr);
void   SPI_M25PE_WRLR(uint32 Dst_Addr,uchar byte);

#endif

//w25x.h end------------------   
