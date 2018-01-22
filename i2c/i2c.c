#include <i2c.h>

/* 引脚定义 */
#define SDA_PORT_RCC RCC_APB2Periph_GPIOB
#define SDA_PORT GPIOB
#define SDA_PIN GPIO_Pin_1
#define SCL_PORT_RCC RCC_APB2Periph_GPIOB
#define SCL_PORT GPIOB
#define SCL_PIN GPIO_Pin_0

static void set_scl_output_mode(void) //设置scl作为输出模式
{
    GPIO_InitTypeDef temp;        //定义一个端口属性结构体
    //下面指定端口的各个属性
    temp.GPIO_Speed=GPIO_Speed_50MHz;
    temp.GPIO_Mode=GPIO_Mode_Out_OD;
    temp.GPIO_Pin=SCL_PIN;    //设置这个端口的所有引脚
    GPIO_Init(SCL_PORT,&temp);    //进行初始化
}
static void set_sda_output_mode(void) //设置sda作为输出模式
{
    GPIO_InitTypeDef temp;        //定义一个端口属性结构体
    //下面指定端口的各个属性
    temp.GPIO_Speed=GPIO_Speed_50MHz;
    temp.GPIO_Mode=GPIO_Mode_Out_OD;
    temp.GPIO_Pin=SDA_PIN;    //设置这个端口的所有引脚
    GPIO_Init(SDA_PORT,&temp);    //进行初始化
}
static void set_sda_input_mode(void) //设置sda作为输入模式
{
    GPIO_InitTypeDef temp;        //定义一个端口属性结构体
    //下面指定端口的各个属性
    temp.GPIO_Speed=GPIO_Speed_50MHz;
    temp.GPIO_Mode=GPIO_Mode_IN_FLOATING;
    temp.GPIO_Pin=SDA_PIN;    //设置这个端口的所有引脚
    GPIO_Init(SDA_PORT,&temp);    //进行初始化
}

static void i2c_delay()    //在stm32中，这里的延时是很有必要的，有些设备不加延时无法使用！！！
{
//    udelay(1);
    int i;
    for(i=20;i>0;i--);    //20大约2us
}

static void i2c_ack(u8 ack) //应答信号，给接收函数用，主机每接收到一个字节要向从机发送一位应答信号，但是最后一个字节不需要应答
{
//进入此函数之前i2c_scl是低电平
    set_sda_output_mode();
    if(ack) GPIO_WriteBit(SDA_PORT, SDA_PIN, Bit_SET);
    else GPIO_WriteBit(SDA_PORT, SDA_PIN, Bit_RESET);
    i2c_delay();
    GPIO_WriteBit(SCL_PORT, SCL_PIN, Bit_SET);
    i2c_delay();
    GPIO_WriteBit(SCL_PORT, SCL_PIN, Bit_RESET);
    i2c_delay();
}

static void i2c_start() //起始信号
{
//进入此函数前i2c_scl和i2c_sda应都是高电平
    set_sda_output_mode();
    GPIO_WriteBit(SDA_PORT, SDA_PIN, Bit_SET);
    i2c_delay();
    GPIO_WriteBit(SCL_PORT, SCL_PIN, Bit_SET);
    i2c_delay();i2c_delay();
    GPIO_WriteBit(SDA_PORT, SDA_PIN, Bit_RESET);
    i2c_delay();i2c_delay();
    GPIO_WriteBit(SCL_PORT, SCL_PIN, Bit_RESET); //钳住i2c_scl，这样其它设备就不会占用I2C总线了
    i2c_delay();
}//出了start函数i2c_scl是低电平，以方便i2c_sda随时可以变化

static void i2c_stop() //停止信号
{
//进入此函数之前i2c_scl应为低电平
    set_sda_output_mode();
    GPIO_WriteBit(SDA_PORT, SDA_PIN, Bit_RESET);
    i2c_delay();
    GPIO_WriteBit(SCL_PORT, SCL_PIN, Bit_SET);
    i2c_delay();i2c_delay();i2c_delay();
    GPIO_WriteBit(SDA_PORT, SDA_PIN, Bit_SET);
    i2c_delay();i2c_delay();
}//出了这个函数后i2c_scl和i2c_sda都是高电平，都被释放了

/*主机发送一个字节，返回应答信号，0是应答。*/
/*本函数不会等待从机的应答位，如果从机不应答的话这里只是返回一个不应答的值1，但是不会等待，不能自适应速度。
 */
static u8 i2c_sendB(u8 dat) //根据I2C协议，先发高位后发低位
{
    void delay5(u8);
    u8 ack; //应答信号
    u8 i;
    set_sda_output_mode();
    for(i=0;i<=7;i++)
    {
        if(dat&0x80)
            GPIO_WriteBit(SDA_PORT, SDA_PIN, Bit_SET);
        else
            GPIO_WriteBit(SDA_PORT, SDA_PIN, Bit_RESET);
        dat<<=1;
        i2c_delay();
        GPIO_WriteBit(SCL_PORT, SCL_PIN, Bit_SET);
        i2c_delay();
        GPIO_WriteBit(SCL_PORT, SCL_PIN, Bit_RESET);
    } //数据发送完了，下面要等待应答信号
    i2c_delay();
    GPIO_WriteBit(SDA_PORT, SDA_PIN, Bit_SET);set_sda_input_mode(); //释放数据线用来接收应答信号
    i2c_delay();
    GPIO_WriteBit(SCL_PORT, SCL_PIN, Bit_SET);
    i2c_delay();
//    if(GPIO_ReadInputDataBit(SDA_PORT, SDA_PIN))
    if(dat)     //nicek
        ack=1;
    else
        ack=0;
    GPIO_WriteBit(SCL_PORT, SCL_PIN, Bit_RESET);
    i2c_delay();
/*------------------------------------
  这里可能要有一点延时，因为这个函数不会等待从机发送应答信号，如果从机在接受完一个字节数据后有较多的事情要处理的话，
  这里就需要一个合适的延时时间给从机处理数据，不然数据发送太快，从机忙不过来 ---------------------------------*/

    return ack;
}/*出了这个函数i2c_scl是低电平，i2c_sda可以随时变化*/

/*主机发送一个字节，返回应答信号，0是应答。*/
/*本函数会等待从机的应答位才结束本字节的传输，如果从机不应答的话这里会持续产生时钟信号，知道检测到应答信号才会结束本函数，
  从机接收到数据之后可以先不发应答信号，而是对数据进行处理，等处理完了之后返回一个应答信号给主机，主机将发送下一个字节。
  这样可以自适应速度。 *
  void i2c_sendB(u8 dat) //根据I2C协议，先发高位后发低位
  {
  void delay5(u8);
  bit ack; //应答信号
  u8 i;
  for(i=0;i<=7;i++)
  {
  if(dat&0x80)
  i2c_sda=1;
  else
  i2c_sda=0;
  dat<<=1;
  i2c_delay();
  i2c_scl=1;
  i2c_delay();
  i2c_scl=0;
  } //数据发送完了，下面要等待应答信号
  i2c_delay();
  i2c_sda=1; //释放数据线用来接收应答信号
  for(;;) //誓死等到应答信号，等不到就一直等
  {
  i2c_delay();
  i2c_scl=1;
  i2c_delay();
  if(i2c_sda==0) //说明进行了应答
  break;
  i2c_scl=0;
  }
  i2c_scl=0;
  i2c_delay();
  }出了这个函数i2c_scl是低电平，i2c_sda可以随时变化*/

//主机接收到的1个字节数据，形参是应答信号，0是应答
static u8 i2c_recB(u8 ack) //先传高位，后传低位
{
    u8 get=0;
    u8 i;
    GPIO_WriteBit(SDA_PORT, SDA_PIN, Bit_SET);set_sda_input_mode(); //释放数据线用来接收应答信号
    for(i=0;i<=7;i++)
    {
        i2c_delay();
        GPIO_WriteBit(SCL_PORT, SCL_PIN, Bit_SET);
        i2c_delay();
        get<<=1;
        if(GPIO_ReadInputDataBit(SDA_PORT, SDA_PIN))
            get|=0x01;
        else
            get&=0xfe;
        GPIO_WriteBit(SCL_PORT, SCL_PIN, Bit_RESET);
    }
    i2c_delay();
    i2c_ack(ack); //对从机进行应答，此时从机应该已经释放sda了

    return get;
}
/* 本程序适用于I2C主机。
   上面4个终极函数在调用时也要遵循规则，不像其它例程那样简单。
   在整个程序一开始要让i2c_scl和i2c_sda都置1，让I2C总线空闲。
   一个设备想要通信要先发出起始信号，然后发送数据，发送多少个数据无限制，但是起始信号后面的第一个字节数据代表的是从机地址和读写信号。
   主机发送数据时，从机每接收到一个字节就会给主机一个应答位，应答位由i2c_sendB函数返回。
   主机接收数据时，主机每接收到一个数据都应给从机一个应答位，应答位作为接收函数i2c_rec的形参在执行接收动作时同时指定是否应答。

   在所有数据传送完毕后主机发送停止信号释放I2C总线。
   在对一个设备操作完成后想要对另一个设备进行操作，主机可以不发送停止信号，而是再发送一个起始信号，在这个起始信号后面紧接着在发送一个从机地址和读写信息。
*/




void i2c_init(void)
{
    //首先，GPIO属于外设，要把该外设的时钟打开
    RCC_APB2PeriphClockCmd(SDA_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(SCL_PORT_RCC, ENABLE);

    set_scl_output_mode();
    set_sda_output_mode();
}

/*
  起始 + 从机地址写 + 内部地址或cmd + length个数据 + 结束
  device_addr是7位的i2c从机地址。
  length指的是data的数据长度，如果仅仅想发送cmd_addr后就结束，把length指为0即可。
*/
void i2c_master_write(u8 device_addr, u8 cmd_addr, u8 *dat, u8 length)
{
    i2c_start();
    i2c_sendB(device_addr<<1); //最低位是0，表示写
    i2c_sendB(cmd_addr); //真正写给从机的第一字节，一般作为内部地址或者cmd
    while(length>0)
    {
        i2c_sendB(*dat);
        dat++;
        length--;
    }
    i2c_stop();
}

/*  只写一个命令，不写入数据。 */
void i2c_writeCmd(u8 device_addr, u8 cmd)
{
    i2c_master_write(device_addr, cmd, 0, 0);
}

/*  往内部指定寄存器写一个字节数据。 */
void i2c_writeReg8(u8 device_addr, u8 cmd_addr, u8 dat)
{
    i2c_master_write(device_addr, cmd_addr, &dat, 1);
}

/*  往内部指定寄存器写2个字节数据，低8位在低地址，高8位在高地址。 */
void i2c_writeReg16(u8 device_addr, u8 cmd_addr, u16 dat)
{
    u8 dat_temp[2];
    dat_temp[0] = dat;
    dat_temp[1] = dat>>8;
    i2c_master_write(device_addr, cmd_addr, dat_temp, 2);
}

/*
  起始 + 从机地址写 + 内部地址或cmd + 起始 + 从机地址读 + 读取length个数据 + 结束
  读取从机地址指定寄存器、指定长度的数据。
  device_addr是7位的i2c从机地址。
  cmd_addr是内部地址，第一个字节有时也被认为是cmd。
  读取length个数据放到dat指向的地方
*/
void i2c_master_read(u8 device_addr, u8 cmd_addr, u8 *dat, u8 length)
{
    i2c_start();
    i2c_sendB(device_addr<<1); //从机地址，最低位是0，表示写
    i2c_sendB(cmd_addr);
    
    i2c_start();
    i2c_sendB(device_addr<<1 | 0x01); //从机地址，最低位是1，表示读
    while(length>1) //读取数据，不是最后一个字节
    {
        *dat = i2c_recB(0);
        dat++;
        length--;
    }
    //下面读取最后一个字节，不进行应答
    *dat = i2c_recB(1);
    i2c_stop();
}

/*  读取从机地址指定寄存器的一个字节数据。 */
u8 i2c_readReg8(u8 device_addr, u8 cmd_addr)
{
    u8 temp;
    i2c_master_read(device_addr, cmd_addr, &temp, 1);
    return temp;
}

/*
  读取指定寄存器2个字节，返回一个16位数，地地址在低8位，高地址在高8位。 */
u16 i2c_readReg16(u8 device_addr, u8 cmd_addr)
{
    u8 temp[2];
    u16 temp16;
    i2c_master_read(device_addr, cmd_addr, temp, 2);
    temp16 = temp[1]<<8;
    temp16 |= temp[0];
    return temp16;
}
