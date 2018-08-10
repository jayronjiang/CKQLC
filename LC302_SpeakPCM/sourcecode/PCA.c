
#include<REG52.H> 

#include "stc12l5612.h"
#include "ut_debug.h"
#include "delay.h"

#include "pca.h"

extern unsigned char code PCM_Warn[];


 ////////////////////////////////////////////
 // 
 // 环形缓冲区操作（缓存用于填充PCA的数据）
 //
 /////////////////////////////////////////////
unsigned char idata pca_code_buf[8] ;
unsigned char pca_buf_wr=0;	// 写指针
unsigned char pca_buf_rd=0;	// 读指针

unsigned char pac_code_ax=0;
unsigned short pca_over_cnt=0;


// 用于... 判断缓冲区是否有空余
unsigned char PCA_buf_fill_validate()
{
	unsigned char next_wr, cc;

	if(pca_buf_wr != (unsigned char)(sizeof(pca_code_buf)-1) ) // 到了末尾？	
		next_wr = pca_buf_wr + 1;
	else
		next_wr = 0;		// 绕回到 0
	cc = (next_wr!=pca_buf_rd);

	#ifdef __UT_DEBUG__x
	if(!cc)
	{
		char temp[7]="w= r= "; 
		temp[2] = 0x30+	pca_buf_wr	;
		temp[5] = 0x30+	pca_buf_rd	; 
		DEBUG_PUTS_(temp);
	}
	#endif
	return cc;
}

// 用于... 填写数据到缓冲区
// 使用前，先调用PCA_buf_fill_validate()确认缓冲区未满 
void PCA_buf_fill_code(unsigned char pcm)   
{
	// 把PCM码放入缓冲区
	pca_code_buf[pca_buf_wr] = pcm;	 

	// 移动缓冲区的写指针
	if(pca_buf_wr != (unsigned char)(sizeof(pca_code_buf)-1) ) // 到了末尾？	
		pca_buf_wr++;
	else
		pca_buf_wr = 0;		// 绕回到 0

	EPCA_LVD= 1 ; // 开 PCA 中断
}

// 用于PCA中断调用，读缓冲区的数据至PCA寄存器
void PCA_buf_read_code()
{							 
	if( pca_buf_rd != pca_buf_wr ) 	
	{								  
		/* 把缓冲区的PCM码读出	*/		
		pac_code_ax	 = pca_code_buf[pca_buf_rd];		 
											
		/* 移动缓冲区的读指针	*/		  
		if(pca_buf_rd != (unsigned char)(sizeof(pca_code_buf)-1) ) 
			pca_buf_rd++;										 
		else													  
			pca_buf_rd = 0;										 
	}															  
	else														 
	{															 
		pac_code_ax = 0x80;  /*无效时的默认值：50%duty cycle;	*/
				
		EPCA_LVD= 0 ; //   关PCA中断				
	}															
}

/////////////////////////////////////////////////////////
// 用程序内置的PCM数据测试PCA
void PCA_Test_SampleVox()
{
	unsigned short sz;

	for(sz=0; sz<1338; sz++)
	{
		while( !PCA_buf_fill_validate() ) 	// 等待PCA缓冲有空
			DELAY_US(10);

		PCA_buf_fill_code( PCM_Warn[sz] ); // 填入PCA缓冲
	}
	for(sz=0; sz<200; sz++) ;	 // delay
}

/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////

void PCA_init()
{
	CCON = 0; // Initial PCA control register,PCA timer stop/Clear CF flag/ Clear all module interrupt flag

	CMOD=0x03;  //0000,0011 setup PCA timer source as Fosc/2, PCA timer overflow interrupt enable(ECF=1)

	CL=0x00;	// Reset PCA base timer
	CH=0xff;

	CCAP0L=0x80;  //same as CCAP0H
	CCAP0H=0x80;  //PWM0 output 50% duty cycle square wave

	CCAPM0=0x42;  //0100,0010 PCA module-0 work in 8bit PWM mode 

	EPCA_LVD = 1;  // PCA   interrupt

	CR=1;         // PCA Timer start run

	EA=1;
}


void ISR_PCA_ovr()  interrupt 6 //using 2
{
	pca_over_cnt++;
	if(pca_over_cnt==0xffff/80)	 led1=0;				  //??
   	if(pca_over_cnt==0xffff/70)	{led1=1; pca_over_cnt=0;}

	CH=0xfb;			  //5次计数后溢出--中断
	
	PCA_buf_read_code();	 // 把缓冲区的PCM码填入 /
	CCAP0H = pac_code_ax;

	CF=0;
}
/////////////////////////////////////////////////