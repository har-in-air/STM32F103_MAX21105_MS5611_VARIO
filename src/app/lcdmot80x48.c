#include "common.h"
#include "board.h"
#include "config.h"
#include "i2clcd.h"
#include "lcdmot80x48.h"

#include "font6x8.txt"
#include "font13x16.txt"

#define CMD_ADDR	0x78
#define DAT_ADDR	0x7A
            
#define XCMD(val) i2clcd_XmtCommand(CMD_ADDR,val)

static uint8_t gnPage = 0;
static uint8_t gnCol = 0;



void lcd_SetPos(uint8_t r,uint8_t c){
	gnPage = r;
	gnCol = c;
	XCMD(0xB0|(r & 0x0F));
	XCMD(0x10|((c>>4)));
	XCMD(0x00|(c & 0x0F));
	}

void lcd_Init(void) {
	i2clcd_Config();
    LCDRST_LO();
    delayMs(50);
    LCDRST_HI();    
	delayMs(50);
	XCMD(0xE2); // reset
	XCMD(0xEA);
    XCMD(0xC8); // reverse scan direction
	XCMD(0x81); // volume set
	XCMD(0x5E); // volume
	XCMD(0x20); // resistor ratio set
	XCMD(0xAF); // display on
	XCMD(0x40); // display line address set
	XCMD(0xAF); // display on
	lcd_Clear();
	}

void lcd_Clear(void) {
    for (uint8_t row = 0; row < 6; row++) {
		XCMD(0xB0 + row);
		XCMD(0x10);
		XCMD(0x00);
	  	i2clcd_Start();
	  	if (i2clcd_XByte(DAT_ADDR)) {
			i2clcd_Stop();
			return;
			}
		int count = 80;
		while (count--) {
	  		if (i2clcd_XByte(0x00)) {
				i2clcd_Stop();
				return;
				}
			}
	   	i2clcd_Stop();
		}
	}


void lcd_PutChar(uint8_t ch){
	int i,inx;
	inx = 6*(ch - 32);
  	i2clcd_Start();
  	if (i2clcd_XByte(DAT_ADDR)) {
		i2clcd_Stop();
		return;
		}
	for ( i = 0; i < 6; i++ ) {
  		if (i2clcd_XByte(gFont6x8[inx+i]))  {
			i2clcd_Stop();
			return;
			}
		}
   	i2clcd_Stop();
	}


void lcd_PutChar13x16(uint8_t ch){
	int i, inx;
    
    if (ch == ' ') inx = 0;
    else
    if (ch == '+') inx = 26;
    else
    if (ch == '-') inx = 52;
    else
    if (ch == '.') inx = 78;
    else
	if ((ch >= 48) && (ch <= 57)) inx = 104+ 13*2*(ch - 48);
    else return;
  	i2clcd_Start();
  	if (i2clcd_XByte(DAT_ADDR)) {
		i2clcd_Stop();
		return;
		}
	for ( i = 0; i < 13; i++ ) {
  		if (i2clcd_XByte(gFont13x16[inx+i]))  {
			i2clcd_Stop();
			return;
			}
		}
   	i2clcd_Stop();
	lcd_SetPos(gnPage+1, gnCol);
	inx += 13;
  	i2clcd_Start();
  	if (i2clcd_XByte(DAT_ADDR)) {
		i2clcd_Stop();
		return;
		}
	for ( i = 0; i < 13; i++ ) {
  		if (i2clcd_XByte(gFont13x16[inx+i]))  {
			i2clcd_Stop();
			return;
			}
		}
   	i2clcd_Stop();
	lcd_SetPos(gnPage-1, gnCol+13);		
	}



void lcd_PrintSz(u08 r, u08 c, char* sz)	{
	lcd_SetPos(r,c);
	while(*sz){
		lcd_PutChar(*sz++);
		}
	}

void lcd_PrintSz13x16(u08 r, u08 c, char* sz)	{
	lcd_SetPos(r,c);
	while(*sz){
		lcd_PutChar13x16(*sz++);
		}
	}


void lcd_Printf(int row, int col, char* format, ...)    {
    char szbuf[10];
    va_list args;
    va_start(args,format);
    vsprintf(szbuf,format,args);
    va_end(args);
    lcd_PrintSz(row,col,szbuf);
    }

void lcd_Printf13x16(int row, int col, char* format, ...)    {
    char szbuf[10];
    va_list args;
    va_start(args,format);
    vsprintf(szbuf,format,args);
    va_end(args);
    lcd_PrintSz13x16(row,col,szbuf);
    }
