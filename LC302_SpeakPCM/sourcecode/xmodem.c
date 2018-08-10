
#include<REG52.H>

///#include "crc16.h"


#define XMODEM_SOH  0x01
#define XMODEM_STX  0x02
#define XMODEM_EOT  0x04
#define XMODEM_ACK  0x06
#define XMODEM_NAK  0x15
#define XMODEM_CAN  0x18
#define XMODEM_CTRLZ 0x1A

//////////////////////////////////////////////////////////////////////////////////

extern unsigned short code crc16_tab[256];

//////////////////////////////////////////////////////////////////
/* �ص�����*/

extern unsigned char *xmodem_buff; 

extern unsigned char xm_flush_rx_record(unsigned char len);
extern unsigned char xm_load_tx_record ( );

// ��ָ�뷽ʽ
//unsigned char (*xm_flush_rx_record)(unsigned char len);
//unsigned char (*xm_load_tx_record) ( );

///////

#define XMODEM_XMIT_SIZE 128

#define CALL_xm_flush_rx_record(len) xm_flush_rx_record(len)
#define CALL_xm_load_tx_record(  ) xm_load_tx_record(  )

/////////////////////////////////////////////////////////////////////////////////

unsigned char  _wait_byte_1TS()
{
	unsigned char timecnt=0xff;
	unsigned char timeout=0xff;

	while(timecnt--)
	{
    	while(timeout--)
		{
			if(RI) return 1;
		}
	}
	return 0;
}
unsigned char  _wait_byte()
{
	if( _wait_byte_1TS() ) return 1;
	if( _wait_byte_1TS() ) return 1;
	if( _wait_byte_1TS() ) return 1;
	if( _wait_byte_1TS() ) return 1;
	if( _wait_byte_1TS() ) return 1;
	
	if( _wait_byte_1TS() ) return 1;
	if( _wait_byte_1TS() ) return 1;
	if( _wait_byte_1TS() ) return 1;
	if( _wait_byte_1TS() ) return 1;
	if( _wait_byte_1TS() ) return 1;

	return 0;
}
 
unsigned char  _inbyte()
{
	unsigned char cTm;  
    cTm = SBUF;   
    RI=0;   
    return cTm;   
}

void _outbyte(unsigned char c)
{
    TI=0;   
    SBUF=c;
	while(!TI);
	TI=0;  
}

//////////////////////////////////

#define MAX_RETRY 10

#define E0_OK	 0	/* normal end */
#define E1_RECV  1	/* recv error */
#define E2_CAN	 2	/* canceled by remote */
#define E3_SYNC  3	 /* sync error */
#define E4_RETRY 4	/* too many retry error */
#define E9_FRW	 9

/*
����������䣿
1.�����ɽ��շ����������������ͷ�����"C"����NAK��
  ����NAK�ź�-��ʾ���շ��������ۼӺ�У�飻�����ַ�"C"-���ʾ���շ������ʹ��CRCУ��.
2.���ͷ���Ϊ���Է��͵�һ�����ݰ��������Ѿ�������
  ���ͷ�����Ӧ�ý�������ÿ��128/1k�ֽڵ����ݼ��ϰ�ͷ�����ţ����Ų��룬ĩβ����У��ͣ������֡��ʽ���͡�

| Start Of Hearder | Packet Number | ~(Packet Number) | Packet Data | 16-Bit CRC |
*/


unsigned char xmodemReceive()
{
	unsigned char  retry;
	unsigned char  c;
	unsigned char  transz = 0;
	unsigned short i;
	unsigned char  ipacketno = 1, blockno;
	unsigned short  crc;

	// �����ͣ�����������	
	// ����֡ͷ
	retry = 0;
	start_frame:
		for( ; ; ) 
		{
			retry++;
			if(transz==0 && retry>MAX_RETRY*2) break;
			if(transz!=0 && retry>MAX_RETRY) break;
			
			if(transz==0) _outbyte('C');	// ��һ��ʱ�������䡢����, ��CRCУ��

			if(  _wait_byte()==0 ) continue;

			c = _inbyte();
			switch (c) 
			{
			case XMODEM_SOH:
					transz = 128;
					goto start_recv;
			case XMODEM_STX:
					transz = 250;
					goto start_recv;
			case XMODEM_EOT:
					_outbyte(XMODEM_ACK);
					return E0_OK; /* normal end */
			case XMODEM_CAN:
					_wait_byte();
					c = _inbyte();
					if( c == XMODEM_CAN)
					{
						_outbyte(XMODEM_ACK);
						return E2_CAN; /* canceled by remote */
					}
					break;
			default:
					break;
			}
		}
		_outbyte(XMODEM_CAN);
		_outbyte(XMODEM_CAN);
		_outbyte(XMODEM_CAN);
		return E3_SYNC; /* sync error */

	start_recv:
		if(  _wait_byte()==0 )  return E1_RECV; /* recv error */
		blockno = _inbyte();	
		if(  _wait_byte()==0 )  return E1_RECV; /* recv error */
		c = _inbyte();

		if( (blockno != (255-c) ) || 
			(blockno !=ipacketno && blockno !=(ipacketno-1) ) ) 
		{
			for(i=0; i<transz+2; i++)	// flush data
			{
				 _wait_byte();
				 c = _inbyte();
			}
			goto reject;
		}

		crc = 0;
		for(i=0; i<transz; i++)
		{
			if(  _wait_byte()==0 )  return E1_RECV; /* recv error */
			c = _inbyte();

			crc = crc16_tab[ ((crc>>8) ^ c) & 0xFF ] ^ (crc << 8);
			xmodem_buff[ i ] = c;
		}

		if(  _wait_byte()==0 )  return E1_RECV; /* recv error */
		c = _inbyte();
		if( c != ((crc >> 8)&0xFF) ) goto reject;

		if(  _wait_byte()==0 )  return E1_RECV; /* recv error */
		c = _inbyte();
	    if( c != (crc&0xFF) ) goto reject;   

		if( blockno ==(ipacketno-1) ) ;// �ش��İ���ô����
		c = CALL_xm_flush_rx_record(transz);
		if( c!=0 ) 
		{
			_outbyte(XMODEM_CAN);
			_outbyte(XMODEM_CAN);
			_outbyte(XMODEM_CAN);
			return E9_FRW;
		 }
		if(blockno == ipacketno) ipacketno++;

		_outbyte(XMODEM_ACK);

		retry = 0;	// ???
		goto start_frame;

	reject:
		retry ++;
		if(retry>MAX_RETRY)
		{
			_outbyte(XMODEM_CAN);
			_outbyte(XMODEM_CAN);
			_outbyte(XMODEM_CAN);
			return E4_RETRY; /* too many retry error */
		}
	
		for(i=0; i<0xff; i++); // delay

		_outbyte(XMODEM_NAK);
		goto start_frame;
}

unsigned char xmodemTransmit()
{
	unsigned char retry;
	unsigned char c;
	unsigned char packetno = 1;
	unsigned short crc=0;

	// �ȴ����շ�����
	for( retry = 0; retry < MAX_RETRY*2; ++retry) 
	{
		if ( _wait_byte()==0 ) continue;

		c = _inbyte();
		switch (c) 
		{
			case 'C':
					goto start_trans; // ����֧��CRCУ��
			case XMODEM_NAK:
//					goto start_trans;  // �ݲ�֧�ֺ�У��
					continue;
			case XMODEM_CAN:
					_wait_byte();
					c = _inbyte();
					if( c == XMODEM_CAN) 
					{
						_outbyte(XMODEM_ACK);
						return E2_CAN; /* canceled by remote */
					}
					break;
			default:
					break;
		}
	}
	_outbyte(XMODEM_CAN);
	_outbyte(XMODEM_CAN);
	_outbyte(XMODEM_CAN);
	return E3_SYNC; /* no sync */
   
    // ��ʼ��֡����
	start_trans:
 
		c = CALL_xm_load_tx_record( );  
		if(c)
		{
			while(c<XMODEM_XMIT_SIZE) xmodem_buff[c++] = XMODEM_CTRLZ;	// packet size = 128
			for (retry = 0; retry < MAX_RETRY/2; ++retry)
			{
				// send packet
				_outbyte( XMODEM_SOH );
				_outbyte( packetno );
				_outbyte( 255-packetno );
				crc = 0;
				for(c=0; c<XMODEM_XMIT_SIZE; c++)
				{
					_outbyte( xmodem_buff[c] );
					crc = crc16_tab[((crc>>8) ^ xmodem_buff[c]) & 0xFF] ^ (crc << 8);
				}
				_outbyte(crc>>8);   
				_outbyte(crc); 
			
				// wait for answer
				for(c=0,crc=0; c<3 && crc==0; c++) 
					crc = _wait_byte() ;
				
				if(crc==0) continue;

				c = _inbyte();
				switch (c) 
				{
					case XMODEM_ACK:
						++packetno;
						goto start_trans;
					case XMODEM_CAN:
						_wait_byte();
						c = _inbyte(); 
						if ( c == XMODEM_CAN)
						{
							_outbyte(XMODEM_ACK);
							return E2_CAN; /* canceled by remote */
						}
						break;
					case XMODEM_NAK:
					default:
						break;
				}
			}	//- end of 'retry < MAX_RETRY'
			_outbyte(XMODEM_CAN);
			_outbyte(XMODEM_CAN);
			_outbyte(XMODEM_CAN);
			return E4_RETRY; /* too many xmit error */
		}
		else // -if c==0
		{
			for (retry = 0; retry < 10; ++retry) 
			{
				_outbyte(XMODEM_EOT);

				_wait_byte();
				c = _inbyte();
				if ( c == XMODEM_ACK) break;
			}
//			TRACE("XMODEM_EOT:%s\n", c==XMODEM_ACK? "ACK" : "??");
			return E0_OK;
		}
}


/////////////////////////////////////////////////////////


