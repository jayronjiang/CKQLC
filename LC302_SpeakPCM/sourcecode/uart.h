

/************************************************************/

void uart_int();
void T0_int();

#define RESET_RX comm_buff_ptr = 0;\
				 comm_bcc = 0;\
				 comm_REnd = 0;\
				comm_RStart = 0;

/************************************************************/
//�ⲿ����

extern unsigned char idata comm_buff[];	//�������ݻ�����
extern bit bdata comm_Stored;			//�������ݸ��±�־
/* ��������дָ�� */
extern unsigned char data comm_buff_ptr;
/* ����� */
extern unsigned char comm_bcc;
/* ���ձ�־ */
extern bit bdata comm_RStart;	//���յ�����ʼ��־
extern bit bdata comm_REnd;	//���յ���������־
extern unsigned char UART_timeout_count;	  //���ڽ��ճ�ʱ����

