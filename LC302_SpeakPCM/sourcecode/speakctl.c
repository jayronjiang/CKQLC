
#include<REG52.H> 
#include <INTRINS.H>    
#include"speakctl.h"
#include "uart.h"
#include "SpeakVox.h"


#define IS_ZERO(array) (array[3]==0&&array[2]==0&&array[1]==0&&array[0]==0)

unsigned char data data_CX;			 //存放车型
unsigned char data data_CN;			 //存放车种
unsigned char idata Fee[4];			 //金额千位
unsigned char idata Bal[4];			 //余额千位

bit bdata fThanksSpeaked;			//收费完成标志

void NumberProcess(unsigned char array[]);

/////////////////////////////////////////////////////////////////////////////////////
///串口接收数据处理
///当串口接收到的数据格式正确时进入此函数
///数据格式: STX CX CN FEE(4) BAL(4) ENTRANCE(6) ETX BCC
///			 02    1  2 3456   789 10 			  03
///参考语音播放协议
///////////////////////////////////////////////////
void Dataprocess(void)
{
//----将缓存中的数据存入各变量
	data_CX = comm_buff[0];
	data_CN = comm_buff[1];
	Fee[3] = comm_buff[2]&0x0F;	    //取金额千位
	Fee[2] = comm_buff[3]&0x0F;			  //百位
	Fee[1] = comm_buff[4]&0x0F;			  //十位
	Fee[0] = comm_buff[5]&0x0F;			  //个位
	Bal[3] = comm_buff[6]&0x0F;	    //取余额千位
	Bal[2] = comm_buff[7]&0x0F;			  //百位
	Bal[1] = comm_buff[8]&0x0F;			  //十位
	Bal[0] = comm_buff[9]&0x0F;			  //个位
	
	if(data_CX=='Y')
	{
		if(!fThanksSpeaked)
		{
			fThanksSpeaked=1;
			Vox_PlayList_Add(ID_XieXie);		//谢谢
			Vox_PlayList_Add(ID_YLPA);			//一路平安
		}
	}
	else 
	{	
		fThanksSpeaked=0;
		switch(data_CN)
		{
//			case '0':
			//保留
//			break;
			case '1'://==========================================正常车种
				if(IS_ZERO(Fee))	  							//金额为0
				{
					Vox_PlayList_Add(ID_TXK);					//发“通行卡”
					Vox_PlayList_Add(ID_XKCG);					//发“写卡成功”
				}
				else
				{
					if(data_CX>='1'&&data_CX<='5')				//车型范围：1～5
					{
 //						Vox_PlayList_Add(ID_Delay);			 
						Vox_PlayList_Add(ID_NiHao);				//你好
						Vox_PlayList_Add(data_CX&0x0F);			//X
						Vox_PlayList_Add(ID_XingChe);			//型车
						Vox_PlayList_Add(ID_Qing);				//请
						Vox_PlayList_Add(ID_Jiao);				//交
						NumberProcess(Fee);
						Vox_PlayList_Add(ID_Yuan); 				//元
					}
				}
				break;
			case '2': 	//=======================================免费车
 //				Vox_PlayList_Add(ID_MFC);			 
				break;
			case '3':  //========================================预付车
 				Vox_PlayList_Add(ID_YFC);						//发“预付车”
				if(!IS_ZERO(Fee))								//金额不为0时报价
		 		{		
					Vox_PlayList_Add(ID_Qing);					//发“请”
					Vox_PlayList_Add(ID_Jiao);					//发“交”
					NumberProcess(Fee);
					Vox_PlayList_Add(ID_Yuan);					//元
				}
				Vox_PlayList_Add(ID_YuE);						//发“余额”
				if(IS_ZERO(Bal))								//余额为0
				{
					Vox_PlayList_Add(ID_NUM_0);
				}  												//报零，跳过“元”
				else											//余额不为0时报数额
				{	
					NumberProcess(Bal);
					Vox_PlayList_Add(ID_Yuan); 					//元
				}				
				break;  
			case '4': //=========================================公务车
//				Vox_PlayList_Add(ID_GWC);			 
				break;
			case '5': //=========================================军警车
 				Vox_PlayList_Add(ID_JJC);			 
				break;
			case '6': //=========================================未付车
				Vox_PlayList_Add(ID_WFC);			 
				break;
			case '7': //=========================================粤通卡写卡成功
 //				Vox_PlayList_Add(ID_Delay);			 
				Vox_PlayList_Add(ID_YTK);			 
				Vox_PlayList_Add(ID_XKCG);			 
				break;
			case '8': //=========================================欢迎光临，请按绿色键取卡
 				Vox_PlayList_Add(ID_HYGL);			 
				Vox_PlayList_Add(ID_QingAn);
				Vox_PlayList_Add(ID_LSJ);
				Vox_PlayList_Add(ID_QuKa);			 
				break;
			case 'A': //=========================================请取通行卡
 				Vox_PlayList_Add(ID_QingQu);
				Vox_PlayList_Add(ID_TXK);			 
				break;
			case 'M': //=========================================粤通卡标识成功
 				Vox_PlayList_Add(ID_YTK);			 
				Vox_PlayList_Add(ID_BSCG);			 
				break;
			case 'N': //=========================================通行卡标识成功
 				Vox_PlayList_Add(ID_TXK);			 
				Vox_PlayList_Add(ID_BSCG);			 
				break;
			case 'O': //=========================================标识成功
 				Vox_PlayList_Add(ID_BSCG);			 
				break;
			default:
				break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
///数额处理函数
///
///
void NumberProcess(unsigned char array[])
{
	bit bdata fNumberSpeak=0;
   	if( array[3] != 0 )	//千
	{ 
		fNumberSpeak=1;
		Vox_PlayList_Add(array[3]);
		Vox_PlayList_Add(ID_NUM_Qian);	
	}
	if( array[2] != 0 )	//百
	{
		Vox_PlayList_Add(array[2]);
		Vox_PlayList_Add(ID_NUM_Bai);	   
		fNumberSpeak=1;
	}
	else if( fNumberSpeak && ( array[1] != 0 ) )
	{
		Vox_PlayList_Add(0);
	}
	if( array[1] != 0 )	//十
	{	
		Vox_PlayList_Add(array[1]);
		Vox_PlayList_Add(ID_NUM_Shi);	   
	}
	else if( fNumberSpeak && ( array[0] != 0 ) )
	{
		Vox_PlayList_Add(0);
	}
	if( array[0] != 0 )
	{
		Vox_PlayList_Add(array[0]);
	}
}
