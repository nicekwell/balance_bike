#include <gy521.h>
#include <i2c.h>
#include <stdio.h>

/******************以下是mpu6050的两个寄存器操作函数****************/
static void mpu6050_writeReg(u8 regAddress,u8 dat)//往mpu6050的指定寄存器写入数据
{
    /************ 这里添加i2c接口函数，往I2C设备（从机地址mpu6050_address)的regAddress写入一个字节数据dat *************/
    //wiringPiI2CWriteReg8(gy521, regAddress, dat);
    i2c_writeReg8(mpu6050_address, regAddress, dat);
}
static u8 mpu6050_readReg(u8 regAddress)//从mpu6050读取指定的寄存器
{
    /************ 这里添加i2c接口函数，从I2C设备（从机地址mpu6050_address)的regAddress读取一个字节数据并返回 *************/
    //return (u8)wiringPiI2CReadReg8(gy521, regAddress);
    return i2c_readReg8(mpu6050_address, regAddress);
}

/*********************************************以下是对外公开的函数*************************************/
void mpu6050_init(void)
{
    mpu6050_writeReg(PWR_MGMT_1,0x00);//解除休眠状态
    mpu6050_writeReg(SMPLRT_DIV,0x07);//设置采样率。采样频率=陀螺仪输出频率/（1+SMPLRT_DIV）
    mpu6050_writeReg(CONFIG,0x06);//设置低通滤波器，具体功能不清楚

//下面设置加速度传感器的量程
    switch(mpu6050_AFS_SEL)
    {
    case 0://+-2g
        mpu6050_writeReg(ACCEL_CONFIG,0x00);
        break;
    case 1://+-4g
        mpu6050_writeReg(ACCEL_CONFIG,0x08);
        break;
    case 2://+-8g
        mpu6050_writeReg(ACCEL_CONFIG,0x10);
        break;
    case 3://+-16g
        mpu6050_writeReg(ACCEL_CONFIG,0x18);
        break;
    }

//下面设置陀螺仪的量程
    switch(mpu6050_FS_SEL)
    {
    case 0://+-250 deg/s
        mpu6050_writeReg(GYRO_CONFIG,0x00);
        break;
    case 1://+-500 deg/s
        mpu6050_writeReg(GYRO_CONFIG,0x08);
        break;
    case 2://+-1000 deg/s
        mpu6050_writeReg(GYRO_CONFIG,0x10);
        break;
    case 3://+-2000 deg/s
        mpu6050_writeReg(GYRO_CONFIG,0x18);
        break;
    }
}

int mpu6050_getAccelXData(void)//返回x轴加速度的原始数据
{
    u8 h,l;
    u32 temp;
    h=mpu6050_readReg(ACCEL_XOUT_H);
    l=mpu6050_readReg(ACCEL_XOUT_L);
    temp = (h<<8)+l;
    if(temp&0x8000)     //是负数
    {
        temp=~temp;
        temp&=0xffff;
        temp+=1;
        return -((int)temp);
    }
    else
        return (int)temp;
}
int mpu6050_getAccelYData(void)//返回y轴加速度的原始数据
{
    u8 h,l;
    u32 temp;
    h=mpu6050_readReg(ACCEL_YOUT_H);
    l=mpu6050_readReg(ACCEL_YOUT_L);
    temp = (h<<8)+l;
    if(temp&0x8000)     //是负数
    {
        temp=~temp;
        temp&=0xffff;
        temp+=1;
        return -((int)temp);
    }
    else
        return (int)temp;
}
int mpu6050_getAccelZData(void)//返回z轴加速度的原始数据
{
    u8 h,l;
    u32 temp;
    h=mpu6050_readReg(ACCEL_ZOUT_H);
    l=mpu6050_readReg(ACCEL_ZOUT_L);
    temp = (h<<8)+l;
    if(temp&0x8000)     //是负数
    {
        temp=~temp;
        temp&=0xffff;
        temp+=1;
        return -((int)temp);
    }
    else
        return (int)temp;
}
int mpu6050_getGyroXData(void)//返回x轴角速度的原始数据
{
    u8 h,l;
    u32 temp;
    h=mpu6050_readReg(GYRO_XOUT_H);
    l=mpu6050_readReg(GYRO_XOUT_L);
    temp = (h<<8)+l;
    if(temp&0x8000)     //是负数
    {
        temp=~temp;
        temp&=0xffff;
        temp+=1;
        return -((int)temp);
    }
    else
        return (int)temp;
}
int mpu6050_getGyroYData(void)//返回y轴角速度的原始数据
{
    u8 h,l;
    u32 temp;
    h=mpu6050_readReg(GYRO_YOUT_H);
    l=mpu6050_readReg(GYRO_YOUT_L);
    temp = (h<<8)+l;
    if(temp&0x8000)     //是负数
    {
        temp=~temp;
        temp&=0xffff;
        temp+=1;
        return -((int)temp);
    }
    else
        return (int)temp;
}
int mpu6050_getGyroZData(void)//返回z轴角速度的原始数据
{
    u8 h,l;
    u32 temp;
    h=mpu6050_readReg(GYRO_ZOUT_H);
    l=mpu6050_readReg(GYRO_ZOUT_L);
    temp = (h<<8)+l;
    if(temp&0x8000)     //是负数
    {
        temp=~temp;
        temp&=0xffff;
        temp+=1;
        return -((int)temp);
    }
    else
        return (int)temp;
}
int mpu6050_getTemperatureData(void)//返回温度的原始数据
{
    u8 h,l;
    u32 temp;
    h=mpu6050_readReg(TEMP_OUT_H);
    l=mpu6050_readReg(TEMP_OUT_L);
    temp = (h<<8)+l;
    if(temp&0x8000)     //是负数
    {
        temp=~temp;
        temp&=0xffff;
        temp+=1;
        return -((int)temp);
    }
    else
        return (int)temp;
}

/*******************************************************************
注意：上面对原始数据的读取都是正确的，但是下面把原始数据换算成实际物理值会
出现问题，应该是精度不够造成的，c语言在进行浮点数运算时老是会出错。
*******************************************************************/
double mpu6050_getAccelX(void)//返回x轴加速度的物理值，单位是m/s^2
{
    double temp;
    switch(mpu6050_AFS_SEL)
    {
    case 0://+-2g
        temp=(double)mpu6050_getAccelXData()/16384;//此时得到的单位是g
        temp*=9.8;//得到单位是m/s^2
        break;
    case 1://+-4g
        temp=(double)mpu6050_getAccelXData()/8192;//此时得到的单位是g
        temp*=9.8;//得到单位是m/s^2
        break;
    case 2://+-8g
        temp=(double)mpu6050_getAccelXData()/4096;//此时得到的单位是g
        temp*=9.8;//得到单位是m/s^2
        break;
    case 3://+-16g
        temp=(double)mpu6050_getAccelXData()/2048;//此时得到的单位是g
        temp*=9.8;//得到单位是m/s^2
        break;
    }
    return temp;
}
double mpu6050_getAccelY(void)//返回y轴加速度的物理值，单位是m/s^2
{
    double temp;
    switch(mpu6050_AFS_SEL)
    {
    case 0://+-2g
        temp=(double)mpu6050_getAccelYData()/16384;//此时得到的单位是g
        temp*=9.8;//得到单位是m/s^2
        break;
    case 1://+-4g
        temp=(double)mpu6050_getAccelYData()/8192;//此时得到的单位是g
        temp*=9.8;//得到单位是m/s^2
        break;
    case 2://+-8g
        temp=(double)mpu6050_getAccelYData()/4096;//此时得到的单位是g
        temp*=9.8;//得到单位是m/s^2
        break;
    case 3://+-16g
        temp=(double)mpu6050_getAccelYData()/2048;//此时得到的单位是g
        temp*=9.8;//得到单位是m/s^2
        break;
    }
    return temp;
}
double mpu6050_getAccelZ(void)//返回z轴加速度的物理值，单位是m/s^2
{
    double temp;
    switch(mpu6050_AFS_SEL)
    {
    case 0://+-2g
        temp=(double)mpu6050_getAccelZData()/16384;//此时得到的单位是g
        temp*=9.8;//得到单位是m/s^2
        break;
    case 1://+-4g
        temp=(double)mpu6050_getAccelZData()/8192;//此时得到的单位是g
        temp*=9.8;//得到单位是m/s^2
        break;
    case 2://+-8g
        temp=(double)mpu6050_getAccelZData()/4096;//此时得到的单位是g
        temp*=9.8;//得到单位是m/s^2
        break;
    case 3://+-16g
        temp=(double)mpu6050_getAccelZData()/2048;//此时得到的单位是g
        temp*=9.8;//得到单位是m/s^2
        break;
    }
    return temp;
}
double mpu6050_getGyroX(void)//返回x轴的角速度，单位是 deg/s
{
    double temp;
    switch(mpu6050_FS_SEL)
    {
    case 0://+-250 deg/s
        temp=(double)mpu6050_getGyroXData()*0.007629394;//得到的单位是 deg/s
        break;
    case 1://+-500 deg/s
        temp=(double)mpu6050_getGyroXData()*0.015258789;//得到的单位是 deg/s
        break;
    case 2://+-1000 deg/s
        temp=(double)mpu6050_getGyroXData()*0.030517578;//得到的单位是 deg/s
        break;
    case 3://+-2000 deg/s
        temp=(double)mpu6050_getGyroXData()*0.061035156;//得到的单位是 deg/s
        break;
    }
    return temp;
}
double mpu6050_getGyroY(void)//返回y轴的角速度，单位是 deg/s
{
    double temp;
    switch(mpu6050_FS_SEL)
    {
    case 0://+-250 deg/s
        temp=(double)mpu6050_getGyroYData()*0.007629394;//得到的单位是 deg/s
        break;
    case 1://+-500 deg/s
        temp=(double)mpu6050_getGyroYData()*0.015258789;//得到的单位是 deg/s
        break;
    case 2://+-1000 deg/s
        temp=(double)mpu6050_getGyroYData()*0.030517578;//得到的单位是 deg/s
        break;
    case 3://+-2000 deg/s
        temp=(double)mpu6050_getGyroYData()*0.061035156;//得到的单位是 deg/s
        break;
    }
    return temp;
}
double mpu6050_getGyroZ(void)//返回z轴的角速度，单位是 deg/s
{
    double temp;
    switch(mpu6050_FS_SEL)
    {
    case 0://+-250 deg/s
        temp=(double)mpu6050_getGyroZData()*0.007629394;//得到的单位是 deg/s
        break;
    case 1://+-500 deg/s
        temp=(double)mpu6050_getGyroZData()*0.015258789;//得到的单位是 deg/s
        break;
    case 2://+-1000 deg/s
        temp=(double)mpu6050_getGyroZData()*0.030517578;//得到的单位是 deg/s
        break;
    case 3://+-2000 deg/s
        temp=(double)mpu6050_getGyroZData()*0.061035156;//得到的单位是 deg/s
        break;
    }
    return temp;
}
float mpu6050_getTemperature(void)//返回温度，单位是摄氏度
{
    return (float)mpu6050_getTemperatureData()/340 + 36.53;
}

