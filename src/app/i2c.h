#ifndef I2C_H_
#define I2C_H_


#define I2CERR_NAK          11


void i2c_Config(void);
u08  i2c_RcvByte(u08 devId, u08 addr);
int  i2c_XmtData(u08 devId, u08 cmd);
u08  i2c_RcvData(u08 devId);
int  i2c_XmtByte(u08 devId, u08 addr, u08 data);
int  i2c_RcvBuf(u08 devId, u08 addr, int nBytes, u08* pBuf);
int  i2c_XmtBuf(u08 devId, u08 addr, int nBytes, u08* pBuf );
u08  i2c_RByte(int ack);
int  i2c_XByte(u08 dat);
int  i2c_RBit(void);
int  i2c_XBit(int bit);
int  i2c_Stop(void);
int  i2c_Start(void);
int  i2c_XmtCommand(u08 devId, u08 cmd);
int  i2c_XmtCommandBuffer(u08 devId, int nBytes, u08* pCmdBuf);

unsigned char i2c_read8(unsigned char acknack);
unsigned char i2c_write8(unsigned char c);

#endif // end I2C_H_ 
