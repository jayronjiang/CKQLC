
#define _INS_1T

#ifdef _INS_1T
	#define _ICT 1
#else
	#define _ICT 2
#endif

#define _D_RATE

#ifdef _D_RATE
	#define _DRT 2
#else
	#define _DRT 1
#endif

// 晶振频率
#define XTAL		22118400

// xx时间片（机器周期数）
#define TIME_SPLIT_1US 	(XTAL/(12/_DRT))/1000000		// 1微妙周期数，1.8432@22.118,400MHz
#define TIME_SPLIT_1MS 	(XTAL/(12/_DRT))/1000	   		// 1毫秒周期数，1843.2@22.118,400MHz

// 延时n微妙（n<=255 or 85)
#define DELAY_US(US)	{ \
							unsigned char cnt;   \
							cnt=US*TIME_SPLIT_1US/_ICT; \
							while(--cnt); \
						}\

// 延时n毫秒（n<=70 or 18)
#define DELAY_MS(MS)	{ \
							unsigned char c1,c2; 		\
							c1 =MS*TIME_SPLIT_1MS/_ICT/255; 	\
							for(;c1;--c1)	\
							{ \
								c2=255;		\
								while(--c2);	\
							} \
						}\


// 休眠几个毫秒（ milliseconds ）
// 该方法时间精度不是很高
#define SleepMs(ms)	{	\
						unsigned int x = ms;	\
						for(; x; --x)		\
							DELAY_MS(1); 	\
					}	\



///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////


/***************************************************
*	 利用定时器0计数来设定延时时间	****************
*
*	DTIMER_INIT  用于初始化定时器	***************
*	DTIMER_SET() 用于设定所需的时间长（us）**********
*	DTIMER_OVER  用于检查设定的时间是否到了**********
****************************************************/

// 1（us）所相当于的定时器计数时钟数
#define TIME_COLCKS		(XTAL/12/1000000) 

// 定时器T0,工作方式1(16位定时器)，中断禁止
#define DTIMER_INIT		TMOD&=~0x03; TMOD|=0x01; ET0 = 0;

// 设定定时器T计数器初值
#define DTIMER_LOAD(tf)	TH0=(0xffff-(tf))>>8; TL0=(0xffff-(tf));

// 定时器启动、停止
#define DTIMER_ENABLE	TR0

// 定时器，溢出标记
#define DTIMER_OVER		TF0


#define DTIMER_SET(us)	DTIMER_ENABLE = 0; \
						DTIMER_OVER = 0;	\
						DTIMER_LOAD( us * TIME_COLCKS -1 ); \
						DTIMER_ENABLE = 1;

