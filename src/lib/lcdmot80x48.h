#ifndef LCDMOT80X48_H_
#define LCDMOT80X48_H_

void lcd_Clear(void);
void lcd_Init(void);
void lcd_PutChar(uint8_t ch);
void lcd_PutCharX2(uint8_t ch);
void lcd_PrintSz(u08 r, u08 c, char* s);
void lcd_PrintSzX2(u08 r, u08 c,char* s);
void lcd_Printf(int row, int col, char* format, ...);
void lcd_PutChar9x16(uint8_t ch);
void lcd_SetPos(uint8_t r,uint8_t c);

#endif
