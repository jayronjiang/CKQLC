//xy-lc301.h
//==========================================================================================

#define COMM_LENTH 24
#define BAR_CONTROL_DELAY_TIME 2	//2 * 200ms
#define ONE_SECOND 5
#define AUTO_DETCET_TIME	3  *ONE_SECOND	//3  seconds
#define DETECT_DELAY_TIME	2  *ONE_SECOND	//1  seconds
#define ALARM_TIME		 	10 *ONE_SECOND	//10 seconds
#define GREEN 1
#define RED 0
#define RECEIVE_TIMEOUT		6
//==========================================================================================

sbit WDT		=P1^4;
sbit COMMFLASH	=P1^7;	//Communication LED	, H-light, L-unlight

// 3 Vars-assemble blow to select the UART source.
// e.g. TOVOX = 0, TOFEE=1, TOCPU=0, means select the FEE device to communicate.
sbit TOVOX	=P1^0;	//Selection of voice
sbit TOFEE	=P1^1;	//Selection of fee_display
sbit TOCPU	=P1^3;	//Selection of cmmunication with PC

sbit BAR_UP	=P0^0;	//8-chanel output-control.(8-chanel input-detect legcy to P2)
sbit BAR_DOWN	=P0^1;
sbit TTL_GREEN	=P0^2;
sbit VOX_ALM	=P0^3;
sbit LIGHT_ALM	=P0^4;
sbit LAN_LAMP	=P0^5;
sbit BAK1_USED	=P0^6;
sbit BAK2_USED	=P0^7;
sbit pinSwtichALG =P0^6;
//==========================================================================================



