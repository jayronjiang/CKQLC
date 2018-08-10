
// MCU Type is: STC12LE5612AD

sfr P4    = 0xE8;   
sfr WDTCR = 0xE1;   

sfr AUXR  = 0x8E;   
sfr P0M0=0x93;
sfr P0M1=0x94;
sfr P1M0=0x91;
sfr P1M1=0x92;
sfr P2M0=0x95;
sfr P2M1=0x96;
sfr P3M0=0xb1;
sfr P3M1=0xb2;


/* Declare SFR associated with the PCA */
sfr  CCON	= 0xD8;		// PCA control register
sbit CCF0	= CCON^0;	// PCA module-0 interrupt flag
sbit CR		= CCON^6; 	// PCA timer run controll bit
sbit CF		= CCON^7; 	// PCA timer overflow flag
sfr  CMOD 	= 0xD9; 	// PCA mode register
sfr  CL 	= 0xE9; 	// PCA base timer LOW
sfr  CH 	= 0xF9; 	// PCA base timer HIGH
sfr CCAPM0	= 0xDA; 	//PCA module-0 mode register
sfr CCAP0L	= 0xEA; 	//PCA module-0 mode capture register LOW
sfr CCAP0H	= 0xFA; 	//PCA module-0 mode capture register HIGH
sbit EPCA_LVD=IE^6;

/*SPI*/
sfr SPSTAT = 0x84;	//SPI status register
#define SPIF 0x80	//SPSTAT.7
#define WCOL 0x40	//SPSTAT.6

sfr SPCTL = 0x85;	//SPI control register
#define SSIG 0x80	//SPCTL.7
#define SPEN 0x40	//SPCTL.6
#define DORD 0x20	//SPCTL.5
#define MSTR 0x10	//SPCTL.4
#define CPOL 0x08	//SPCTL.3
#define CPHA 0x04	//SPCTL.2

#define SPR_DIV4	0x00	//SPCTL.1 SPCTL.0, clock rate=CPU_CLK/4
#define SPR_DIV16	0x01	//SPCTL.1 SPCTL.0, clock rate=CPU_CLK/16
#define SPR_DIV64	0x02	//SPCTL.1 SPCTL.0, clock rate=CPU_CLK/64
#define SPR_DIV128	0x03	//SPCTL.1 SPCTL.0, clock rate=CPU_CLK/128

sfr SPDAT = 0x86;	//SPI data register

sbit SPISS = P1^4;	//SPI slave select,connect to slave'SS (P1.4)pin

sbit EADC_SPI = IE^5;	//IE.5 EADC_SPI
#define ESPI 0x80		//AUXR.3 ESPI


/**/
sbit P10=P1^0;
sbit P11=P1^1;
sbit P12=P1^2;
sbit P33=P3^3;

#define led1 P12
#define led2 P10
