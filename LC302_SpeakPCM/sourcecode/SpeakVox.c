

#include "W25X.H"
#include "PCA.H"
#include "ut_debug.h"
#include "SpeakVox.h"


/*******************************************************************************
		 			����������FLASH�еĽṹ 	

 +-------------------------------------------------------------------------+
 | ����0��Ϣ��(*) | ����1��Ϣ��(*) | ��������������������������������������|
 | ������������������������������������������������������������������������|
 | ������������������������������������������������������| ����n��Ϣ��(*)  |
 +-------------------------------------------------------------------------+
 | ����0��PCM����(x bytes)  |  ����1��PCM����(y bytes)  |  ����������������|
 | ������������������������������������������������������������������������|
 | ����������������������������������������������| ����n��PCM����(z bytes) |  
 |																		   |
 +-------------------------------------------------------------------------+
���У�
   ��������Ϣ�顱��ʾ�˸�������PCM������FLASH�е�ַ�����ȣ�
   ÿ����������Ϣ�顱���ɹ̶��� 8 �ֽ� ���ɣ���ṹ���£�
   +-------------+---------------------+---------------------+---------------+
   +  ID(1bytes) | PCM���ݵ�ַ(3bytes) |  PCM���ݳ���(3ytes) | У��(1bytes)  |
   +-------------+---------------------+---------------------+---------------+
   
   ID�����洢������˳�����δ�0��255�����������֧�ֵ�����ID��Ӧ��
   У�飺ǰ7���ֽڵ�����
************************************************************************************/
#define VOX_INF_BLOCK_LEN 8


#define INVALID_ID  ((unsigned char)0xFF)

//////////////////////////////////////////////////////////////////////////////////////
// ��ǰҪ���ŵ�����������PCM���ݵ�ַ������
unsigned long int idata ulCurrentVox_Addr=0 ;
unsigned long int ulCurrentVox_Len=0 ;

// ����Ҫ���ŵ�����ID�б�
unsigned char idata VoxPlayList[30]="";
unsigned char idata VoxListPutPtr=0;
unsigned char idata VoxListGetPtr=0;


/*
 ��������ID����ȡ��������PCM���ݵ�ַ�����ȣ����浽��ǰ����
*/
void Vox_read_inf_to_current(unsigned char ID)
{
	unsigned short inf_addr ;
	unsigned char IDx, sum;

	if(ID>MAX_VOX_ID) return;

	inf_addr = ID * VOX_INF_BLOCK_LEN; // ÿ����Ϣ���ǹ̶����ֽ���

	SPI_Read_Start(inf_addr);

	IDx = SPI_Get_Byte();
	DEBUG_ASSERT_( ID==IDx );

	((uchar*)&ulCurrentVox_Addr)[0] = 0;
	((uchar*)&ulCurrentVox_Addr)[1] = SPI_Get_Byte();
	((uchar*)&ulCurrentVox_Addr)[2] = SPI_Get_Byte();
	((uchar*)&ulCurrentVox_Addr)[3] = SPI_Get_Byte();

	((uchar*)&ulCurrentVox_Len)[0] = 0;
	((uchar*)&ulCurrentVox_Len)[1] = SPI_Get_Byte();
	((uchar*)&ulCurrentVox_Len)[2] = SPI_Get_Byte();
	((uchar*)&ulCurrentVox_Len)[3] = SPI_Get_Byte();

	sum = SPI_Get_Byte();
	DEBUG_ASSERT_( sum==0x0a );

	SPI_Read_End();

	DEBUG_PUTS_("Vox Read Info: ");
	DEBUG_PRINTF_NUM_("VOX ID=%d ", ID);
	DEBUG_PRINTF_NUM_("Addr=%X ", ulCurrentVox_Addr);
	DEBUG_PRINTF_NUM_("Len=%x ", ulCurrentVox_Len);
	DEBUG_PUTS_(" \n");

	if(ulCurrentVox_Addr==0xFFffFF || ulCurrentVox_Len==0xFFffFF)
	{
		ulCurrentVox_Addr=0;
		ulCurrentVox_Len=0;	
	}
}

/*
	�������ȡ�����б��IDֵ
*/
void Vox_PlayList_Add(unsigned char ID)
{
	DEBUG_ASSERT_( VoxListPutPtr<sizeof(VoxPlayList) );

	VoxPlayList[VoxListPutPtr] = ID;
	VoxListPutPtr++;	
}
//
unsigned char Vox_PlayList_Get()
{
	unsigned char ID;
	if( VoxListGetPtr != VoxListPutPtr)
	{
		ID = VoxPlayList[VoxListGetPtr];
		VoxListGetPtr++;
	}
	else
	{	
		ID = INVALID_ID;	 // ��ЧID
		VoxListGetPtr = VoxListPutPtr = 0 ; // �б�λ
	}

	return ID;	
}

// �����б�գ�
unsigned char Vox_PlayList_Empty()
{
	return  VoxListGetPtr==0 && VoxListPutPtr==0;
}

// ��ǰ���������������ˣ�
unsigned char Vox_CurrPlaying_Done()
{
	return ulCurrentVox_Len==0 ;
}

/*
	׼���������ݣ��Խ��в���
	1�����ݲ����б�
	2�����ݵ�ǰ���ŵ�����������PCM���ݵ�ַ������  
*/
void Vox_Prepare_PlayCode()
{
	unsigned char temp;

	if(ulCurrentVox_Len==0)	// ��ǰ�����������û�����ڲ���
	{
		SPI_Read_End();
		temp = Vox_PlayList_Get();  // ȡ����һ��������ID
		if(temp!=INVALID_ID)
		{
			Vox_read_inf_to_current(temp);  // ����ǰID��Ӧ����������ʼ��ַ�����ȵ���Ϣ
		}
	}
	if( ulCurrentVox_Addr	) // ��ǰ�������ŵ�������ʼ��ַ��Ч
	{
		SPI_Read_Start( ulCurrentVox_Addr ); // ���뵱ǰ��������ʼ��ַ
		ulCurrentVox_Addr = 0;
	}

	if( ulCurrentVox_Len ) // ��ǰ���������ڲ���
	{
		if( PCA_buf_fill_validate() )
		{
			temp = SPI_Get_Byte();	// ��FLASH��һ���ֽ�

			PCA_buf_fill_code( temp ); // ����PCA����
			ulCurrentVox_Len--;	 // ��1��ֱ��0
		}
	}
	
}
/*
  �����ǰ�Ĳ���
*/
void Vox_Dismiss_Play()
{
	VoxListGetPtr = VoxListPutPtr = 0 ; // �б�λ

//	ulCurrentVox_Addr = ulCurrentVox_Len = 0 ; // ��ǰ������
}

/* 
  �ȴ�ȫ��������ϣ������б�ȡ�ꡢ��ǰ���Ȳ�����
*/
void Vox_Wait_AllPlayDone()
{
	while( !Vox_PlayList_Empty() || !Vox_CurrPlaying_Done() )
	{
		Vox_Prepare_PlayCode();	
	}
		SPI_Read_End();
}

/*****************************************************************************/
// ���Բ�������
/*****************************************************************************/


// ������������ID
void Test_All_Vox_ID()
{
  		unsigned char i= ID_NUM_0;
		// ��д����ID
		while(1)
		{
			if(i<=MAX_VOX_ID) Vox_PlayList_Add(i++);
			if(i<=MAX_VOX_ID) Vox_PlayList_Add(i++);
			if(i<=MAX_VOX_ID) Vox_PlayList_Add(i++);
			if(i<=MAX_VOX_ID) Vox_PlayList_Add(i++);

			while( !Vox_PlayList_Empty() ) 
			{
				Vox_Prepare_PlayCode();
			}

			if(i > MAX_VOX_ID) break;
		}
		// �ȴ�ȫ���������
		Vox_Wait_AllPlayDone();
 }
