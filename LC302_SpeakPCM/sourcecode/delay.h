
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

// ����Ƶ��
#define XTAL		22118400

// xxʱ��Ƭ��������������
#define TIME_SPLIT_1US 	(XTAL/(12/_DRT))/1000000		// 1΢����������1.8432@22.118,400MHz
#define TIME_SPLIT_1MS 	(XTAL/(12/_DRT))/1000	   		// 1������������1843.2@22.118,400MHz

// ��ʱn΢�n<=255 or 85)
#define DELAY_US(US)	{ \
							unsigned char cnt;   \
							cnt=US*TIME_SPLIT_1US/_ICT; \
							while(--cnt); \
						}\

// ��ʱn���루n<=70 or 18)
#define DELAY_MS(MS)	{ \
							unsigned char c1,c2; 		\
							c1 =MS*TIME_SPLIT_1MS/_ICT/255; 	\
							for(;c1;--c1)	\
							{ \
								c2=255;		\
								while(--c2);	\
							} \
						}\


// ���߼������루 milliseconds ��
// �÷���ʱ�侫�Ȳ��Ǻܸ�
#define SleepMs(ms)	{	\
						unsigned int x = ms;	\
						for(; x; --x)		\
							DELAY_MS(1); 	\
					}	\



///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////


/***************************************************
*	 ���ö�ʱ��0�������趨��ʱʱ��	****************
*
*	DTIMER_INIT  ���ڳ�ʼ����ʱ��	***************
*	DTIMER_SET() �����趨�����ʱ�䳤��us��**********
*	DTIMER_OVER  ���ڼ���趨��ʱ���Ƿ���**********
****************************************************/

// 1��us�����൱�ڵĶ�ʱ������ʱ����
#define TIME_COLCKS		(XTAL/12/1000000) 

// ��ʱ��T0,������ʽ1(16λ��ʱ��)���жϽ�ֹ
#define DTIMER_INIT		TMOD&=~0x03; TMOD|=0x01; ET0 = 0;

// �趨��ʱ��T��������ֵ
#define DTIMER_LOAD(tf)	TH0=(0xffff-(tf))>>8; TL0=(0xffff-(tf));

// ��ʱ��������ֹͣ
#define DTIMER_ENABLE	TR0

// ��ʱ����������
#define DTIMER_OVER		TF0


#define DTIMER_SET(us)	DTIMER_ENABLE = 0; \
						DTIMER_OVER = 0;	\
						DTIMER_LOAD( us * TIME_COLCKS -1 ); \
						DTIMER_ENABLE = 1;

