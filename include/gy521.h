#ifndef __GY521_H__
#define __GY521_H__
/*
  mpu6050模块的驱动。
  采用IIC总线通信，这里把IIC的驱动函数也放在了这里。
  每个轴上的加速度或者角速度读取出来的是16位有符号数据（分高8位和低8位读取）。
  对外提供如下几个函数：
  void mpu6050_init();根据当前配置完成初始化
  int mpu6050_getAccelXData();返回x轴加速度的原始数据
  int mpu6050_getAccelYData();返回y轴加速度的原始数据
  int mpu6050_getAccelZData();返回z轴加速度的原始数据
  int mpu6050_getGyroXData();返回x轴角速度的原始数据
  int mpu6050_getGyroYData();返回y轴角速度的原始数据
  int mpu6050_getGyroZData();返回z轴角速度的原始数据
  int mpu6050_getTemperatureData();返回温度的原始数据

  float mpu6050_getAccelX();返回x轴加速度的物理值，单位是m/s^2
  float mpu6050_getAccelY();返回y轴加速度的物理值，单位是m/s^2
  float mpu6050_getAccelZ();返回z轴加速度的物理值，单位是m/s^2
  float mpu6050_getGyroX();返回x轴的角速度，单位是 deg/s，是度不是弧度
  float mpu6050_getGyroY();返回y轴的角速度，单位是 deg/s
  float mpu6050_getGyroZ();返回z轴的角速度，单位是 deg/s
  float mpu6050_getTemperature();返回温度，单位是摄氏度

  void mpu6050_setAddress(unsigned char);慎用！！！设置模块地址的搞6位。
*/
/************以下是配置项***************/
#define mpu6050_address 0x68/*mpu6050的默认地址是 WHO_AM_I + AD0，WHO_AM_I寄存器的默认值是
                              0x68，AD0引脚悬空时默认为0。*/
#define mpu6050_AFS_SEL 0/*设置加速度传感器的量程：
                           0+-2g
                           1+-4g
                           2+-8g
                           3+-16g
                           在这里设置好之后，后面的初始化等函数就不用再考虑了，那里会根据这里的配置调整。
                         */
#define mpu6050_FS_SEL 0/*设置陀螺仪的量程：
                          0+-250 deg/s
                          1+-500 deg/s
                          2+-1000 deg/s
                          3+-2000 deg/s
                          在这里设置好之后，后面的初始化等函数就不用再考虑了，那里会根据这里的配置调整。
                        */

/******************************以下是mpu6050内部地址定义**************************/
#define SMPLRT_DIV 0x19//陀螺仪采样率，典型值：0x07(125Hz)
/*采样频率=陀螺仪输出频率/（1+SMPLRT_DIV）
  关于陀螺仪输出频率，数据手册上说了但是没看明白。按照本驱动程序的设置方法，
  陀螺仪输出频率=1000Hz。*/

#define CONFIG 0x1A//低通滤波频率，典型值：0x06(5Hz)
/* 具体功能不太清楚*/

#define GYRO_CONFIG 0x1B//陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
/*D7D6D5 D4 D3 D2D1D0
  XG_STYG_STZG_ST|FS_SEL| ---
  高3位分别是设置陀螺仪自检，1-自检，0-不自检（默认）
  FS_SEL是设置陀螺仪满量程：
  FS_SEL量程输出的每个单位1代表
  0+-250 deg/s  250/32768 =  125/16384 ~= 0.007629394 deg/s
  1+-500 deg/s  500/32768 =  125/8192 ~=  0.015258789 deg/s
  2+-1000 deg/s  1000/32768 = 125/4096 ~=  0.030517578 deg/s
  3+-2000 deg/s  2000/32768 = 125/2048 ~=  0.061035156 deg/s*/

#define ACCEL_CONFIG 0x1C//加速计自检、测量范围及高通滤波频率，典型值：0x00(不自检，2G，5Hz)
/*D7D6D5 D4  D3D2D1D0
  XA_STYA_STZA_ST|AFS_SEL|---
  高3位分别是设置加速度传感器自检，1-自检，0-不自检（默认）
  AFS_SEL是设置陀螺仪满量程：
  AFS_SEL量程输出的每个单位1代表
  0+-2g  1/16384 g
  1+-4g  1/8192 g
  2+-8g  1/4096 g
  3+-16g  1/2048 g*/

#define ACCEL_XOUT_H 0x3B//x轴加速度的高2位注意：高位和低8位按位组合在一起是一个有符号int型数据
#define ACCEL_XOUT_L 0x3C//x轴加速度的低8位
#define ACCEL_YOUT_H 0x3D//y轴加速度的高2位注意：高位和低8位按位组合在一起是一个有符号int型数据
#define ACCEL_YOUT_L 0x3E//y轴加速度的低8位
#define ACCEL_ZOUT_H 0x3F//z轴加速度的高2位注意：高位和低8位按位组合在一起是一个有符号int型数据
#define ACCEL_ZOUT_L 0x40//z轴加速度的低8位

#define TEMP_OUT_H 0x41
#define TEMP_OUT_L 0x42
/*上面两个是温度，合在一起也是一个有符号16位数据。
  要换算成摄氏度用这个公式：
  Temperature in degrees C = (TEMP_OUT Register Value as a signed quantity)/340 + 36.53
*/

#define GYRO_XOUT_H 0x43//x轴角速度的高2位注意：高位和低8位按位组合在一起是一个有符号int型数据
#define GYRO_XOUT_L 0x44//x轴角速度的低8位
#define GYRO_YOUT_H 0x45//y轴角速度的高2位注意：高位和低8位按位组合在一起是一个有符号int型数据
#define GYRO_YOUT_L 0x46//y轴角速度的低8位
#define GYRO_ZOUT_H 0x47//z轴角速度的高2位注意：高位和低8位按位组合在一起是一个有符号int型数据
#define GYRO_ZOUT_L 0x48//z轴角速度的低8位
#define PWR_MGMT_1 0x6B//电源管理，典型值：0x00(正常启用)
#define WHO_AM_I 0x75//IIC地址寄存器(默认数值0x68，只读)

/**** 下面是接口函数 ****/
void mpu6050_init(void);
int mpu6050_getAccelXData(void);//返回x轴加速度的原始数据
int mpu6050_getAccelYData(void);//返回y轴加速度的原始数据
int mpu6050_getAccelZData(void);//返回z轴加速度的原始数据
int mpu6050_getGyroXData(void);//返回x轴角速度的原始数据
int mpu6050_getGyroYData(void);//返回y轴角速度的原始数
int mpu6050_getGyroZData(void);//返回z轴角速度的原始数据
int mpu6050_getTemperatureData(void);//返回温度的原始数据
/*******************************************************************
注意：上面对原始数据的读取都是正确的，但是下面把原始数据换算成实际物理值会
出现问题，应该是精度不够造成的，c语言在进行浮点数运算时老是会出错。
******************************************************************/
double mpu6050_getAccelX(void);//返回x轴加速度的物理值，单位是m/s^2
double mpu6050_getAccelY(void);//返回y轴加速度的物理值，单位是m/s^2
double mpu6050_getAccelZ(void);//返回z轴加速度的物理值，单位是m/s^2
double mpu6050_getGyroX(void);//返回x轴的角速度，单位是 deg/s
double mpu6050_getGyroY(void);//返回y轴的角速度，单位是 deg/s
double mpu6050_getGyroZ(void);//返回z轴的角速度，单位是 deg/s
float mpu6050_getTemperature(void);//返回温度，单位是摄氏度

#endif
