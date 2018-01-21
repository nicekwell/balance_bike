#ifndef __I2C_IO_H__
#define __I2C_IO_H__
#include <stm32f10x_lib.h>

void i2c_init(void);    //完成IO的初始化
/*
  起始 + 从机地址写 + 内部地址或cmd + length个数据 + 结束
  device_addr是7位的i2c从机地址。
  length指的是data的数据长度，如果仅仅想发送cmd_addr后就结束，把length指为0即可。
*/
void i2c_master_write(u8 device_addr, u8 cmd_addr, u8 *dat, u8 length);

/*  只写一个命令，不写入数据。 */
void i2c_writeCmd(u8 device_addr, u8 cmd);

/*  往内部指定寄存器写一个字节数据。 */
void i2c_writeReg8(u8 device_addr, u8 cmd_addr, u8 dat);

/*  往内部指定寄存器写2个字节数据，低8位在低地址，高8位在高地址。 */
void i2c_writeReg16(u8 device_addr, u8 cmd_addr, u16 dat);

/*
  起始 + 从机地址写 + 内部地址或cmd + 起始 + 从机地址读 + 读取length个数据 + 结束
  读取从机地址指定寄存器、指定长度的数据。
  device_addr是7位的i2c从机地址。
  cmd_addr是内部地址，第一个字节有时也被认为是cmd。
  读取length个数据放到dat指向的地方
*/
void i2c_master_read(u8 device_addr, u8 cmd_addr, u8 *dat, u8 length);

/*  读取从机地址指定寄存器的一个字节数据。 */
u8 i2c_readReg8(u8 device_addr, u8 cmd_addr);

/*
  读取指定寄存器2个字节，返回一个16位数，地地址在低8位，高地址在高8位。 */
u16 i2c_readReg16(u8 device_addr, u8 cmd_addr);
#endif
