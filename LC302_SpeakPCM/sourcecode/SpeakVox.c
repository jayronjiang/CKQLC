

#include "W25X.H"
#include "PCA.H"
#include "ut_debug.h"
#include "SpeakVox.h"


/*******************************************************************************
		 			语音数据在FLASH中的结构 	

 +-------------------------------------------------------------------------+
 | 语音0信息块(*) | 语音1信息块(*) | 、、、、、、、、、、、、、、、、、、、|
 | 、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、|
 | 、、、、、、、、、、、、、、、、、、、、、、、、、、、| 语音n信息块(*)  |
 +-------------------------------------------------------------------------+
 | 语音0的PCM数据(x bytes)  |  语音1的PCM数据(y bytes)  |  、、、、、、、、|
 | 、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、|
 | 、、、、、、、、、、、、、、、、、、、、、、、| 语音n的PCM数据(z bytes) |  
 |																		   |
 +-------------------------------------------------------------------------+
其中：
   “语音信息块”表示了该语音的PCM数据在FLASH中地址、长度；
   每个“语音信息块”都由固定的 8 字节 构成，其结构如下：
   +-------------+---------------------+---------------------+---------------+
   +  ID(1bytes) | PCM数据地址(3bytes) |  PCM数据长度(3ytes) | 校验(1bytes)  |
   +-------------+---------------------+---------------------+---------------+
   
   ID：按存储的物理顺序，依次从0～255，它与程序所支持的语音ID对应。
   校验：前7个字节的异或和
************************************************************************************/
#define VOX_INF_BLOCK_LEN 8


#define INVALID_ID  ((unsigned char)0xFF)

//////////////////////////////////////////////////////////////////////////////////////
// 当前要播放的那条语音的PCM数据地址、长度
unsigned long int idata ulCurrentVox_Addr=0 ;
unsigned long int ulCurrentVox_Len=0 ;

// 即将要播放的语音ID列表
unsigned char idata VoxPlayList[30]="";
unsigned char idata VoxListPutPtr=0;
unsigned char idata VoxListGetPtr=0;


/*
 根据语音ID，读取该语音的PCM数据地址、长度，保存到当前变量
*/
void Vox_read_inf_to_current(unsigned char ID)
{
	unsigned short inf_addr ;
	unsigned char IDx, sum;

	if(ID>MAX_VOX_ID) return;

	inf_addr = ID * VOX_INF_BLOCK_LEN; // 每个信息块是固定的字节数

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
	设置与读取播放列表的ID值
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
		ID = INVALID_ID;	 // 无效ID
		VoxListGetPtr = VoxListPutPtr = 0 ; // 列表复位
	}

	return ID;	
}

// 播放列表空？
unsigned char Vox_PlayList_Empty()
{
	return  VoxListGetPtr==0 && VoxListPutPtr==0;
}

// 当前这条声音播放完了？
unsigned char Vox_CurrPlaying_Done()
{
	return ulCurrentVox_Len==0 ;
}

/*
	准备语音数据，以进行播放
	1、根据播放列表
	2、根据当前播放的那条语音的PCM数据地址、长度  
*/
void Vox_Prepare_PlayCode()
{
	unsigned char temp;

	if(ulCurrentVox_Len==0)	// 当前语音播放完或没有正在播放
	{
		SPI_Read_End();
		temp = Vox_PlayList_Get();  // 取得下一条语音的ID
		if(temp!=INVALID_ID)
		{
			Vox_read_inf_to_current(temp);  // 读当前ID对应的语音的起始地址、长度等信息
		}
	}
	if( ulCurrentVox_Addr	) // 当前即将播放的语音起始地址有效
	{
		SPI_Read_Start( ulCurrentVox_Addr ); // 进入当前语音的起始地址
		ulCurrentVox_Addr = 0;
	}

	if( ulCurrentVox_Len ) // 当前有语音正在播放
	{
		if( PCA_buf_fill_validate() )
		{
			temp = SPI_Get_Byte();	// 从FLASH读一个字节

			PCA_buf_fill_code( temp ); // 填入PCA缓冲
			ulCurrentVox_Len--;	 // 减1，直到0
		}
	}
	
}
/*
  解除当前的播放
*/
void Vox_Dismiss_Play()
{
	VoxListGetPtr = VoxListPutPtr = 0 ; // 列表复位

//	ulCurrentVox_Addr = ulCurrentVox_Len = 0 ; // 当前的清零
}

/* 
  等待全部播放完毕，包括列表取完、当前长度播放完
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
// 测试播放声音
/*****************************************************************************/


// 测试所有语音ID
void Test_All_Vox_ID()
{
  		unsigned char i= ID_NUM_0;
		// 填写所有ID
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
		// 等待全部播放完毕
		Vox_Wait_AllPlayDone();
 }
