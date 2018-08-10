
/*

¡ö Page size: 256 bytes
¡ö Subsector size: 4 Kbytes
¡ö Sector size: 64 Kbytes
Each sector contains 256 pages 
and each subsector contains 16 pages. Each page is 256-byte wide.
 
program (changing bits from ¡®1¡¯ to ¡®0¡¯)
write   (changing bits to ¡®0¡¯ or ¡®1¡¯)	= 'erase + program'

The page write instruction consists of 
an integrated page erase cycle followed by a page program cycle.

The page write (PW) instruction is entered by driving Chip Select (S) Low, 
and then transmitting the instruction byte, and at least one data byte,
and then driving Chip Select (S) High. 
When Chip Select (S) is driven High, the write cycle starts.
For optimized timings, it is recommended to use the page write (PW) instruction.
¨C Page write in 11 ms (typical)
¨C Page program in 0.8 ms (typical)
¨C Page erase in 10 ms (typical)

Device SPI mode:
--Serial Data Input (D) is latched on the rising edge of Serial Clock (C). 
--Serial data input (D) is sampled on the first rising edge of Serial Clock (C) after Chip Select(S) is driven Low.

--Serial Data Output (Q) changes after the falling edge of Serial Clock (C).

*/
