#include "common.h"
#include "board.h"
#include "stm32f103xb.h"
#include "i2c.h"



// 24000000 / 30*4 = 400khz
#define I2C_CLK_STRETCH() 	{volatile int cntr = 4; while (cntr--) NOP();}


#define SDA_HI()   { I2CPORT->BSRR = PIN_SDA;  }
#define SDA_LO()   { I2CPORT->BSRR = (PIN_SDA<<16); }
#define SCL_HI()   { I2CPORT->BSRR = PIN_SCL; }
#define SCL_LO()   { I2CPORT->BSRR = (PIN_SCL<<16); }

#define SDA    ((I2CPORT->IDR & PIN_SDA ) ? 1 : 0)


int gi2c_error;


inline void i2c_SetSCLOutput() {
    // CNF : MODE
    //  01   10  == 0x6, output OD, mode output < 2MHz
    I2CPORT->CRL &= 0xFFFF0FFF; // clear CNF and MODE bits for pin 3 
    I2CPORT->CRL |= 0x00006000; 
    }
	
inline void i2c_SetSCLInput() {
    // CNF : MODE
    //  01   00  == 0x4, input floating, mode input
    I2CPORT->CRL &= 0xFFFF0FFF; // clear CNF and MODE bits for pin 3
    I2CPORT->CRL |= 0x00004000; 
    }

inline void i2c_SetSDAOutput() {
    // CNF : MODE
    //  01   10  == 0x6, output OD, mode output < 2MHz
    I2CPORT->CRL &= 0xFFFFF0FF; // clear CNF and MODE bits for pin 2
    I2CPORT->CRL |= 0x00000600; 
    }
    
inline void i2c_SetSDAInput() {
    // CNF : MODE
    //  01   00  == 0x4, input floating, mode input
    I2CPORT->CRL &= 0xFFFFF0FF; // clear CNF and MODE bits for pin 2 
    I2CPORT->CRL |= 0x00000400; 
    }

/// Initialize the i2c interface state (both SDA and SCL outputs and logic 1)
void i2c_Config(void)	{
  // scl/sda pins are initialized in gpio.c
  
  i2c_SetSCLOutput();
  i2c_SetSDAOutput();
  I2C_CLK_STRETCH();
  SDA_HI();
  SCL_HI();
  I2C_CLK_STRETCH();
  gi2c_error = 0;
  }


/// Generate i2c start condition, from SCL and SDA high, pull SDA low
int i2c_Start(void)    {
  i2c_SetSDAOutput();
  SDA_HI();
  SCL_HI();
  I2C_CLK_STRETCH();
  I2C_CLK_STRETCH();
  SDA_LO();  // Pull SDA down while SCL is high ... start condition
  I2C_CLK_STRETCH();
  I2C_CLK_STRETCH();
  SCL_LO(); //  Pull SCL low for next bit
  I2C_CLK_STRETCH();
  return 0;
  }


/// Generate i2c stop condition
int i2c_Stop(void)    {
  i2c_SetSDAOutput();
  I2C_CLK_STRETCH();
  SDA_LO();
  I2C_CLK_STRETCH();
  SCL_HI();
  I2C_CLK_STRETCH();
  SDA_HI();
  I2C_CLK_STRETCH();
  return 0;
  }


/// Clock out a single bit
/// @param bit to send
/// @return always 0
int i2c_XBit(int bit)   {
  if (bit) {
   SDA_HI();
  }
  else {
   SDA_LO();
  }
  I2C_CLK_STRETCH();
  SCL_HI();
  I2C_CLK_STRETCH();
  SCL_LO();
  I2C_CLK_STRETCH();
  return 0;
  }

/// Clock in a single bit
/// @return the received bit
int i2c_RBit(void)   {
  int bit;
  SCL_HI();
  I2C_CLK_STRETCH();
  bit = SDA;
  SCL_LO();
  I2C_CLK_STRETCH();
  return bit;
  }

/// Transmit a single byte
/// @param data the byte to transmit
/// @return 0 success, 1 on no acknowledge
int i2c_XByte(u08 dat)    {
  int bitCount = 8;
  i2c_SetSDAOutput();
  I2C_CLK_STRETCH();
  
  while(bitCount--)        {
      i2c_XBit((dat & 0x80)? 1 : 0);
      dat <<= 1;
      }

  i2c_SetSDAInput();
  I2C_CLK_STRETCH();
  if (i2c_RBit())        {
      gi2c_error = I2CERR_NAK;
      return 1;
      }
  return 0;
  }

/// Receive a single byte
/// @param ack  bit to transmit as acknowledgement
/// @return received byte
u08 i2c_RByte(int ack)    {
  u08 inByte = 0;
  int bitCount = 8;
  i2c_SetSDAInput();
  I2C_CLK_STRETCH();

  while(bitCount--)        {
    inByte <<= 1;
    inByte |= i2c_RBit();
    }
  
  i2c_SetSDAOutput();
  I2C_CLK_STRETCH();
  // acknowledge
  i2c_XBit(ack);
  return inByte;
  }

/// Transmit a buffer of characters to assigned device and address
/// @param devID   device ID (unique address for each i2c device in system
/// @param addr   address within device where data buffer needs to be written
/// @param nBytes number of bytes in buffer
/// @param pBuf pointer to byte buffer
/// @return 0 on success, 0xF0 or 0xF1 on error
int i2c_XmtBuf(u08 devId, u08 addr, int nBytes, u08* pBuf )    {
  int cnt;

  gi2c_error = 0;
  i2c_Start();
  if (i2c_XByte(devId))        {
    //return 0xF0;
    }
  if (i2c_XByte(addr))        {
    //return 0xF1;
    }
  for (cnt = 0; cnt < nBytes; cnt++)
    if (i2c_XByte(pBuf[cnt]))            {
        //return 0xF2;
        }
  i2c_Stop();
  return 0;
  }

/// Receive a buffer of characters from assigned device and address
/// @param devID   device ID (unique address for each i2c device in system
/// @param addr   address within device where data buffer needs to be read from
/// @param nBytes number of bytes in buffer
/// @param pBuf pointer to byte buffer
/// @return 0 on success, 0xF0 or 0xF1 or 0xF2 or 0x01 on error
int i2c_RcvBuf(u08 devId, u08 addr, int nBytes, u08* pBuf)    {
  int cnt;

  gi2c_error = 0;
  i2c_Start();
  if (i2c_XByte(devId))        {
    //return 0xF0;
    }
  if (i2c_XByte(addr))        {
    if (gi2c_error == I2CERR_NAK) i2c_Stop();
    //return 0xF1;
    }
  i2c_SetSDAOutput();
  SDA_HI(); // Set SDA high while SCL is low
  I2C_CLK_STRETCH();
  // Restart
  i2c_Start();
  if (i2c_XByte(devId|0x1))        {
    //return 0xF2;
    }

  for (cnt = 0; cnt < nBytes-1; cnt++)        {
    pBuf[cnt] = i2c_RByte(0);
    }
  // Master NAK
  pBuf[nBytes-1] = i2c_RByte(1);
  i2c_Stop();
  return (gi2c_error ? 1 : 0);
  }

/// Transmit a single byte to assigned device and address
/// @param devID   device ID (unique address for each i2c device in system
/// @param addr   address within device where data buffer needs to be written
/// @param data  byte to write
/// @return 0 on success, 0xF0 or 0xF1 or 0xF2 on error
int i2c_XmtByte(u08 devId, u08 addr, u08 data)    {
  gi2c_error = 0;
  i2c_Start();
  if (i2c_XByte(devId))        {
    return 0xF0;
    }
  if (i2c_XByte(addr))        {
    return 0xF1;
    }
  if (i2c_XByte(data))        {
    return 0xF2;
    }
  i2c_Stop();
  return 0;
  }

/// Transmit a single byte command to assigned device
/// @param devID   device ID (unique address for each i2c device in system
/// @param cmd   command
/// @return 0 on success, 0xF0 or 0xF1 or 0xF2 on error
int i2c_XmtData(u08 devId, u08 cmd)    {
  gi2c_error = 0;
  i2c_Start();
  if (i2c_XByte(devId))        {
    i2c_Stop();
    //return 0xF0;
    }
  if (i2c_XByte(cmd))        {
    i2c_Stop();
    //return 0xF1;
    }
  i2c_Stop();
  return 0;
  }

u08 i2c_RcvData(u08 devId)    {
  u08 data;

  gi2c_error = 0;
  i2c_Start();
  if (i2c_XByte(devId))        {
    gi2c_error = 1;
    //return 0xF0;
    }
  i2c_SetSDAOutput();
  SDA_HI(); // Set SDA high while SCL is low
  // Restart
  I2C_CLK_STRETCH();
  i2c_Start();
  if (i2c_XByte(devId|0x1))        {
    gi2c_error = 1;
    //return 0xF2;
    }
  data = i2c_RByte(1);
  i2c_Stop();
  return data;
  }

/// Receive a single byte from assigned device and address
/// @param devID   device ID (unique address for each i2c device in system
/// @param addr   address within device where data buffer needs to be written
/// @return data on success, 0xF0 or 0xF1 or 0xF2 on error with gi2c_error non-zero
u08 i2c_RcvByte(u08 devId, u08 addr)    {
  u08 data;

  gi2c_error = 0;
  i2c_Start();
  if (i2c_XByte(devId))        {
    gi2c_error = 1;
    return 0xF0;
    }
  if (i2c_XByte(addr))        {
    if (gi2c_error == I2CERR_NAK) i2c_Stop();
    return 0xF1;
    }
    i2c_SetSDAOutput();
  SDA_HI(); // Set SDA high while SCL is low
  // Restart
  I2C_CLK_STRETCH();
  i2c_Start();
  if (i2c_XByte(devId|0x1))        {
    gi2c_error = 1;
    return 0xF2;
    }
  data = i2c_RByte(1);
  i2c_Stop();
  return data;
  }

int i2c_XmtCommand(u08 devId, u08 cmd)    {
  i2c_Start();
  if (i2c_XByte(devId))        {
    i2c_Stop();
    return 0xF0;
    }
  if (i2c_XByte(cmd))        {
    i2c_Stop();
    return 0xF1;
    }
  i2c_Stop();
  return 0;
  }

int i2c_XmtCommandBuffer(u08 devId, int nBytes, u08* pCmdBuf)    {
  int inx;

  i2c_Start();
  if (i2c_XByte(devId)) {
    i2c_Stop();
    return 0xF0;
    }
  for (inx = 0; inx < nBytes; inx++) {
    if (i2c_XByte(pCmdBuf[inx])) {
        i2c_Stop();
        return 0xF1;
        }
    }
  i2c_Stop();
  return 0;
  }

unsigned char i2c_write8(unsigned char c) {
  int bitCount = 8;
  i2c_SetSDAOutput();
  I2C_CLK_STRETCH();
  
  while(bitCount--)        {
      i2c_XBit((c & 0x80)? 1 : 0);
      c <<= 1;
      }

  i2c_SetSDAInput();
  I2C_CLK_STRETCH();
  if (i2c_RBit())        {
      gi2c_error = I2CERR_NAK;
      return 1;
      }
  return 0; 
  }

unsigned char i2c_read8(unsigned char acknack){
    u08 inByte = 0;
  int bitCount = 8;
  i2c_SetSDAInput();
  I2C_CLK_STRETCH();

  while(bitCount--)        {
    inByte <<= 1;
    inByte |= i2c_RBit();
    }
  
  i2c_SetSDAOutput();
  I2C_CLK_STRETCH();
  // acknowledge
  i2c_XBit(acknack ? 1 : 0);
  return inByte;
}