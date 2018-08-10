#include<REG52.H> 
#include "stc12l5612.h"
#include "uart.h"

#define BAUDRELOAD 0xfA             //�����ʲ���1T--115200bps 	12T--9600bps 
#define COMM_BUFF_LEN 20

/* ���ջ����� */
unsigned char idata comm_buff[COMM_BUFF_LEN];
/* ��������дָ�� */
unsigned char data comm_buff_ptr = 0;
unsigned char UART_timeout_count = 0;
/* ����� */
unsigned char comm_bcc = 0;
/* ���ձ�־ */
bit bdata comm_RStart = 0;	//���յ�����ʼ��־
bit bdata comm_REnd = 0;	//���յ���������־

/* ��ɱ�־ */
bit bdata comm_Stored = 0;	//���ݽ�����ϣ����ݰ�����




/////////////////////////////////////////////////////////////////////////
///���ڳ�ʼ��
void uart_int()
{
	SCON  = 0x50;                      // mode 1: 8-bit UART, enable receiver
	TMOD |= 0x20;                      // timer 1 mode 2: 8-Bit reload
    TH1 = BAUDRELOAD;   			   //��ʱ��1�����пڲ����ʷ�����
    TL1 = BAUDRELOAD;
//	AUXR|=0x40;            			//1T ----115200BPS
    AUXR&=~0x40;  		            /*12T----9600BPS  */ 
	TR1 = 1;    					//������ʱ��
	ET1	= 0;						// timer 1 interrupt disable
	TI = 0;
	RI = 0;
	ES = 1;                         // enable serial port interrupt
	EA = 1;
}


///////////////////////////////////////////////////////////////////////
///

//////////////////////////////////////////////////////////////////////
//  �����жϳ���
//  ���ո�ʽ��0x02 + ����(nbyte) + 0x03 + BCC(1byte)

void UART_Isr() interrupt 4	//using 1
{
	unsigned char temp;
	if(RI)
	{
		UART_timeout_count = 0;
		RI = 0;
		temp = SBUF;
		
		if(temp==0x02&&(!comm_REnd))	// ��ͷ   
		{
			RESET_RX;
			comm_RStart = 1;
			return;
		}
		if( temp==0x03&&comm_RStart&&(!comm_REnd) )	   // ��β
		{
			comm_REnd = 1;			
			return;
		}

		if( comm_REnd && comm_RStart )	// �Ѿ��յ�ͷ��β��������У��λ
		{
			if(comm_bcc == temp) // У����ȷ���յ������İ���
			{
				comm_Stored = 1; 
			}
			RESET_RX;
			return;
		}


		comm_buff[comm_buff_ptr] = temp;   //��д���棬ע�⣺��ͷ����β��У��λ�����ڻ����ڣ�������������
		comm_buff_ptr++;	

		comm_bcc = comm_bcc^temp; 	// ������˰�ͷ����β��У��λ֮��ĺ�

		if(comm_buff_ptr==COMM_BUFF_LEN)	// ���������
		{
			RESET_RX;
		}
	}
	else
		TI=0;
}

///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// ��ʱ��0��ʼ��
void T0_int()
{	
	TMOD |= 0x01;                      // timer0 mode 1: 16-Bit
	TMOD&=~0x03; 	// ��ʼ����ʱ��0��C/T��M1��M0=0
    AUXR&=~0x80;  		//T0x12 Ϊ0  
  	TL0 = 0;   
	TH0 = 0;       
 	TF0 = 0;				//��T0�жϱ�־
	TR0 = 1;
  	ET0 = 0;
}

