#include "common.h"
#include "config.h"
#include "board.h"
#include "i2clcd.h"



// 48000000 / 30*4 = 400khz
//#define I2CLCD_CLK_STRETCH() 	{volatile int cntr = 1; while (cntr--) NOP();}
//#define I2CLCD_CLK_STRETCH() 	{NOP(); NOP();}
#define I2CLCD_CLK_STRETCH() 	{}


#define LCDSDA_HI()   { I2CLCDPORT->BSRR = PIN_LCDSDA;  }
#define LCDSDA_LO()   { I2CLCDPORT->BSRR = (PIN_LCDSDA<<16); }
#define LCDSCL_HI()   { I2CLCDPORT->BSRR = PIN_LCDSCL; }
#define LCDSCL_LO()   { I2CLCDPORT->BSRR = (PIN_LCDSCL<<16); }

#define LCDSDA    ((I2CLCDPORT->IDR & PIN_LCDSDA ) ? 1 : 0)


int gi2clcd_error;


inline void i2clcd_SetSDAOutput() {
    // CNF : MODE
    //  01   10  == 0x6, output OD, mode output < 2MHz
    I2CLCDPORT->CRL &= 0xFFF0FFFF; // clear CNF and MODE bits for pin 4
    I2CLCDPORT->CRL |= 0x00060000; 
    }
	
inline void i2clcd_SetSDAInput() {
    // CNF : MODE
    //  01   00  == 0x4, input floating, mode input
    I2CLCDPORT->CRL &= 0xFFF0FFFF; // clear CNF and MODE bits for pin 4
    I2CLCDPORT->CRL |= 0x00040000; 
    }

inline void i2clcd_SetSCLOutput() {
    // CNF : MODE
    //  01   10  == 0x6, output OD, mode output < 2MHz
    I2CLCDPORT->CRL &= 0xFF0FFFFF; // clear CNF and MODE bits for pin 5
    I2CLCDPORT->CRL |= 0x00600000; 
    }
    
inline void i2clcd_SetSCLInput() {
    // CNF : MODE
    //  01   00  == 0x4, input floating, mode input
    I2CLCDPORT->CRL &= 0xFF0FFFFF; // clear CNF and MODE bits for pin 5 
    I2CLCDPORT->CRL |= 0x00400000; 
    }

/// Initialize the i2c interface state (both SDA and SCL outputs and logic 1)
void i2clcd_Config(void)	{
  i2clcd_SetSCLOutput();
  i2clcd_SetSDAOutput();
  I2CLCD_CLK_STRETCH();
  LCDSDA_HI();
  LCDSCL_HI();
  I2CLCD_CLK_STRETCH();
  gi2clcd_error = 0;
  }


/// Generate i2c start condition, from SCL and SDA high, pull SDA low
int i2clcd_Start(void)    {
  i2clcd_SetSDAOutput();
  LCDSDA_HI();
  LCDSCL_HI();
  I2CLCD_CLK_STRETCH();
  //I2CLCD_CLK_STRETCH();
  LCDSDA_LO();  // Pull SDA down while SCL is high ... start condition
  I2CLCD_CLK_STRETCH();
  //I2CLCD_CLK_STRETCH();
  LCDSCL_LO(); //  Pull SCL low for next bit
  I2CLCD_CLK_STRETCH();
  return 0;
  }


/// Generate i2c stop condition
int i2clcd_Stop(void)    {
  i2clcd_SetSDAOutput();
  I2CLCD_CLK_STRETCH();
  LCDSDA_LO();
  I2CLCD_CLK_STRETCH();
  LCDSCL_HI();
  I2CLCD_CLK_STRETCH();
  LCDSDA_HI();
  I2CLCD_CLK_STRETCH();
  return 0;
  }


/// Clock out a single bit
/// @param bit to send
/// @return always 0
int i2clcd_XBit(int bit)   {
  if (bit) {
   LCDSDA_HI();
  }
  else {
   LCDSDA_LO();
  }
  I2CLCD_CLK_STRETCH();
  LCDSCL_HI();
  I2CLCD_CLK_STRETCH();
  LCDSCL_LO();
  I2CLCD_CLK_STRETCH();
  return 0;
  }

/// Clock in a single bit
/// @return the received bit
int i2clcd_RBit(void)   {
  int bit;
  LCDSCL_HI();
  I2CLCD_CLK_STRETCH();
  bit = LCDSDA;
  LCDSCL_LO();
  I2CLCD_CLK_STRETCH();
  return bit;
  }

/// Transmit a single byte
/// @param data the byte to transmit
/// @return 0 success, 1 on no acknowledge
int i2clcd_XByte(u08 dat)    {
  int bitCount = 8;
  i2clcd_SetSDAOutput();
  I2CLCD_CLK_STRETCH();
  
  while(bitCount--)        {
      i2clcd_XBit((dat & 0x80)? 1 : 0);
      dat <<= 1;
      }

  i2clcd_SetSDAInput();
  I2CLCD_CLK_STRETCH();
  if (i2clcd_RBit())        {
      gi2clcd_error = I2CLCDERR_NAK;
      return 1;
      }
  return 0;
  }








int i2clcd_XmtCommand(u08 devId, u08 cmd)    {
  i2clcd_Start();
  if (i2clcd_XByte(devId))        {
    i2clcd_Stop();
    return 0xF0;
    }
  if (i2clcd_XByte(cmd))        {
    i2clcd_Stop();
    return 0xF1;
    }
  i2clcd_Stop();
  return 0;
  }


