

/************************************************************/

void uart_int();
void T0_int();

#define RESET_RX comm_buff_ptr = 0;\
				 comm_bcc = 0;\
				 comm_REnd = 0;\
				comm_RStart = 0;

/************************************************************/
//外部变量

extern unsigned char idata comm_buff[];	//串口数据缓存区
extern bit bdata comm_Stored;			//串口数据更新标志
/* 缓冲区读写指针 */
extern unsigned char data comm_buff_ptr;
/* 检验和 */
extern unsigned char comm_bcc;
/* 接收标志 */
extern bit bdata comm_RStart;	//接收到包开始标志
extern bit bdata comm_REnd;	//接收到包结束标志
extern unsigned char UART_timeout_count;	  //串口接收超时计数

