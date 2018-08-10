
#include<REG52.H> 
#include <INTRINS.H>    
#include"speakctl.h"
#include "uart.h"
#include "SpeakVox.h"


#define IS_ZERO(array) (array[3]==0&&array[2]==0&&array[1]==0&&array[0]==0)

unsigned char data data_CX;			 //��ų���
unsigned char data data_CN;			 //��ų���
unsigned char idata Fee[4];			 //���ǧλ
unsigned char idata Bal[4];			 //���ǧλ

bit bdata fThanksSpeaked;			//�շ���ɱ�־

void NumberProcess(unsigned char array[]);

/////////////////////////////////////////////////////////////////////////////////////
///���ڽ������ݴ���
///�����ڽ��յ������ݸ�ʽ��ȷʱ����˺���
///���ݸ�ʽ: STX CX CN FEE(4) BAL(4) ENTRANCE(6) ETX BCC
///			 02    1  2 3456   789 10 			  03
///�ο���������Э��
///////////////////////////////////////////////////
void Dataprocess(void)
{
//----�������е����ݴ��������
	data_CX = comm_buff[0];
	data_CN = comm_buff[1];
	Fee[3] = comm_buff[2]&0x0F;	    //ȡ���ǧλ
	Fee[2] = comm_buff[3]&0x0F;			  //��λ
	Fee[1] = comm_buff[4]&0x0F;			  //ʮλ
	Fee[0] = comm_buff[5]&0x0F;			  //��λ
	Bal[3] = comm_buff[6]&0x0F;	    //ȡ���ǧλ
	Bal[2] = comm_buff[7]&0x0F;			  //��λ
	Bal[1] = comm_buff[8]&0x0F;			  //ʮλ
	Bal[0] = comm_buff[9]&0x0F;			  //��λ
	
	if(data_CX=='Y')
	{
		if(!fThanksSpeaked)
		{
			fThanksSpeaked=1;
			Vox_PlayList_Add(ID_XieXie);		//лл
			Vox_PlayList_Add(ID_YLPA);			//һ·ƽ��
		}
	}
	else 
	{	
		fThanksSpeaked=0;
		switch(data_CN)
		{
//			case '0':
			//����
//			break;
			case '1'://==========================================��������
				if(IS_ZERO(Fee))	  							//���Ϊ0
				{
					Vox_PlayList_Add(ID_TXK);					//����ͨ�п���
					Vox_PlayList_Add(ID_XKCG);					//����д���ɹ���
				}
				else
				{
					if(data_CX>='1'&&data_CX<='5')				//���ͷ�Χ��1��5
					{
 //						Vox_PlayList_Add(ID_Delay);			 
						Vox_PlayList_Add(ID_NiHao);				//���
						Vox_PlayList_Add(data_CX&0x0F);			//X
						Vox_PlayList_Add(ID_XingChe);			//�ͳ�
						Vox_PlayList_Add(ID_Qing);				//��
						Vox_PlayList_Add(ID_Jiao);				//��
						NumberProcess(Fee);
						Vox_PlayList_Add(ID_Yuan); 				//Ԫ
					}
				}
				break;
			case '2': 	//=======================================��ѳ�
 //				Vox_PlayList_Add(ID_MFC);			 
				break;
			case '3':  //========================================Ԥ����
 				Vox_PlayList_Add(ID_YFC);						//����Ԥ������
				if(!IS_ZERO(Fee))								//��Ϊ0ʱ����
		 		{		
					Vox_PlayList_Add(ID_Qing);					//�����롱
					Vox_PlayList_Add(ID_Jiao);					//��������
					NumberProcess(Fee);
					Vox_PlayList_Add(ID_Yuan);					//Ԫ
				}
				Vox_PlayList_Add(ID_YuE);						//������
				if(IS_ZERO(Bal))								//���Ϊ0
				{
					Vox_PlayList_Add(ID_NUM_0);
				}  												//���㣬������Ԫ��
				else											//��Ϊ0ʱ������
				{	
					NumberProcess(Bal);
					Vox_PlayList_Add(ID_Yuan); 					//Ԫ
				}				
				break;  
			case '4': //=========================================����
//				Vox_PlayList_Add(ID_GWC);			 
				break;
			case '5': //=========================================������
 				Vox_PlayList_Add(ID_JJC);			 
				break;
			case '6': //=========================================δ����
				Vox_PlayList_Add(ID_WFC);			 
				break;
			case '7': //=========================================��ͨ��д���ɹ�
 //				Vox_PlayList_Add(ID_Delay);			 
				Vox_PlayList_Add(ID_YTK);			 
				Vox_PlayList_Add(ID_XKCG);			 
				break;
			case '8': //=========================================��ӭ���٣��밴��ɫ��ȡ��
 				Vox_PlayList_Add(ID_HYGL);			 
				Vox_PlayList_Add(ID_QingAn);
				Vox_PlayList_Add(ID_LSJ);
				Vox_PlayList_Add(ID_QuKa);			 
				break;
			case 'A': //=========================================��ȡͨ�п�
 				Vox_PlayList_Add(ID_QingQu);
				Vox_PlayList_Add(ID_TXK);			 
				break;
			case 'M': //=========================================��ͨ����ʶ�ɹ�
 				Vox_PlayList_Add(ID_YTK);			 
				Vox_PlayList_Add(ID_BSCG);			 
				break;
			case 'N': //=========================================ͨ�п���ʶ�ɹ�
 				Vox_PlayList_Add(ID_TXK);			 
				Vox_PlayList_Add(ID_BSCG);			 
				break;
			case 'O': //=========================================��ʶ�ɹ�
 				Vox_PlayList_Add(ID_BSCG);			 
				break;
			default:
				break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
///�������
///
///
void NumberProcess(unsigned char array[])
{
	bit bdata fNumberSpeak=0;
   	if( array[3] != 0 )	//ǧ
	{ 
		fNumberSpeak=1;
		Vox_PlayList_Add(array[3]);
		Vox_PlayList_Add(ID_NUM_Qian);	
	}
	if( array[2] != 0 )	//��
	{
		Vox_PlayList_Add(array[2]);
		Vox_PlayList_Add(ID_NUM_Bai);	   
		fNumberSpeak=1;
	}
	else if( fNumberSpeak && ( array[1] != 0 ) )
	{
		Vox_PlayList_Add(0);
	}
	if( array[1] != 0 )	//ʮ
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
