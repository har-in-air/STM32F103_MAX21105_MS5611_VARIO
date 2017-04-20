#ifndef I2CLCD_H_
#define I2CLCD_H_


#define I2CLCDERR_NAK          11


void i2clcd_Config(void);
u08  i2clcd_RByte(int ack);
int  i2clcd_XByte(u08 dat);
int  i2clcd_RBit(void);
int  i2clcd_XBit(int bit);
int  i2clcd_Stop(void);
int  i2clcd_Start(void);
int  i2clcd_XmtCommand(u08 devId, u08 cmd);


#endif // end I2C_H_ 
