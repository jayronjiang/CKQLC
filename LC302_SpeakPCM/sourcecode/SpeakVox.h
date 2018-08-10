
///////////////////////////////////////////////////////////////////////////////
//    程序支持的语音ID
//	  ID对应该条语音的“语音数据信息结构块”在FLASH中的位置
////////////////////////////////////////////////////////////////////////////////
	
#define ID_NUM_0	0		//零
#define ID_NUM_1	1		//一
#define ID_NUM_2	2		//二
#define ID_NUM_3	3		//三
#define ID_NUM_4	4		//四
#define ID_NUM_5	5		//五
#define ID_NUM_6	6		//六
#define ID_NUM_7	7		//七
#define ID_NUM_8	8		//八
#define ID_NUM_9	9		//九
#define ID_NUM_Shi	10		//十
#define ID_NUM_Bai	11		//百
#define ID_NUM_Qian	12		//千
#define ID_Yuan		13		//元
#define ID_NiHao	14		//你好
#define ID_Qing		15		//请
#define ID_Jiao		16		//交
#define ID_XieXie	17		//谢谢
#define ID_QTX		18		//请通行
#define ID_XingChe	19		//型车
#define ID_QingQu	20		//请取
#define ID_HYGL		21		//欢迎光临
#define ID_MFC		22		//免费车
#define ID_YFC		23		//预付车
#define ID_XZC		24		//行政车
#define ID_JJC		25		//军警车
#define ID_WFC		26		//未付车
#define ID_GWC		27		//公务车
#define ID_BJC		28		//包缴车
#define ID_YuE		29		//余额
#define ID_BSCG		30		//标识成功
#define ID_TXK		31		//通行卡
#define ID_YTK		32		//粤通卡
#define ID_XKCG		33		//写卡成功
#define ID_QingAn	34		//请按
#define ID_LSJ		35		//绿色键
#define ID_QuKa		36		//取卡
#define ID_JHAQD	37		//系好安全带
#define ID_YLPA		38		//一路平安
#define ID_SKSB		39		//刷卡失败
#define ID_ChongXing 40		//重新
#define ID_ZhuXiao	41		//注销
#define ID_Delay	42		// 空白
	
#define MAX_VOX_ID ID_Delay
////////////////////////////////////////////////////

//公开接口函数

void Vox_PlayList_Add(unsigned char ID);
void Vox_Prepare_PlayCode();
void Vox_Dismiss_Play();
void Vox_Wait_AllPlayDone();
void Test_All_Vox_ID();
unsigned char Vox_PlayList_Empty();
unsigned char Vox_CurrPlaying_Done();

