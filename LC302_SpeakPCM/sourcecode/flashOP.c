//Flash²Ù×÷
#include <reg52.h>
#include "flashOP.h"
#include "delay.h"
#include "ate_shell.h"
#include "w25x.h"

bit bdata Enter_NorMode;

void flash_download()
{
	serial_put_chars("Press d and 'enter' in 10s to download voice data");
	serial_get_cmd(3);
	if(cmd_char[0]='d')		 //²ÁÐ´flash
	{
		ATE_Flash_EraseChip();
		ATE_Flash_WriteDown();
		cmd_char[0]='0';
	}
	serial_put_chars("Press q and 'enter' in 10s to enter ATE mode");
	serial_get_cmd(3);
	if(cmd_char[0]='q')
	{
		Enter_NorMode=1;
		cmd_char[0]='0';
	}
	else  Enter_NorMode=0;
}