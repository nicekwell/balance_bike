#include <uart.h>
#include <stm32f10x_lib.h>

void uart2_init(void)
{
    /*启动uart所在GPIO的时钟，各个uart所用的引脚详见下表：
      串口号    RXD     TXD
      1         PA10    PA9
      2         PA3     PA2
      3         PB11    PB10
      4         PC11    PC10
      5         PD2     PC12     */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /*启动串口时钟。
      串口1在APB2总线上，串口2~5在APB1总线上。  */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    /* 分别设置用到的两个GPIO引脚的模式 */
    {
        GPIO_InitTypeDef inittype_GPIOA;
        //TXD设置位推挽复用模式
        inittype_GPIOA.GPIO_Speed=GPIO_Speed_50MHz;
        inittype_GPIOA.GPIO_Mode=GPIO_Mode_AF_PP;
        inittype_GPIOA.GPIO_Pin = GPIO_Pin_2; //TXD引脚
        GPIO_Init(GPIOA, &inittype_GPIOA);
        //RXD设置为浮空输入或上拉输入
        inittype_GPIOA.GPIO_Speed=GPIO_Speed_50MHz;
//        inittype_GPIOA.GPIO_Mode=GPIO_Mode_IN_FLOATING; //设置为上拉输入的话则是 GPIO_Mode_IPU
        inittype_GPIOA.GPIO_Mode=GPIO_Mode_IPU;
        inittype_GPIOA.GPIO_Pin = GPIO_Pin_3; //RXD引脚
        GPIO_Init(GPIOA, &inittype_GPIOA);
    }
    /* 初始化串口 */
    {
        USART_InitTypeDef temp;
        temp.USART_BaudRate = 115200;
        temp.USART_WordLength = USART_WordLength_8b;
        temp.USART_StopBits = USART_StopBits_1;
        temp.USART_Parity = USART_Parity_No;
        temp.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //发送和接收都使能
        temp.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_Init(USART2, &temp);
    }
    /* 如果开启中断的话则配置这一项。
       在这一项配置前，应当包含nvic的头文件和c文件编译，并在程序最开始设置好了分组。*/
    {
        NVIC_InitTypeDef temp;
        temp.NVIC_IRQChannel = USART2_IRQChannel; //这是USART1的中断，2、3、4、5只要改个序号即可
        temp.NVIC_IRQChannelPreemptionPriority = 3;
        temp.NVIC_IRQChannelSubPriority = 3;    //优先级最低
        temp.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&temp);
    }
    /* 如果开启中断的话则配置这一项。
       配置串口各个中断开关，第二个参数是中断类型，有如下中断：
       USART_IT_PE      奇偶错误中断
       USART_IT_TXE     发送中断
       USART_IT_TC      传输完成中断
       USART_IT_RXNE    接收中断
       USART_IT_IDLE    空闲总线中断
       USART_IT_LBD     LIN 中断检测中断
       USART_IT_CTS     CTS 中断
       USART_IT_ERR     错误中断
       以上各种类型中断可分别配置开关，所有的中断都在一个中断处理函数里USART1_IRQHandler。
    */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); //接收中断
    /* 使能串口 */
    USART_Cmd(USART2, ENABLE);
}
void uart2_sendB(u8 dat)
{
    USART_SendData(USART2, dat);
    while(SET!=USART_GetFlagStatus(USART2, USART_FLAG_TC));     //等待发送完成
}
void uart2_sendStr(u8 *p)
{
    for(;*p!='\0';p++)
        uart2_sendB(*p);
}

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE)) //本次中断是接收数据中断
    {
        u8 get;
        void get_control_dat(u8 dat);
        get = USART_ReceiveData(USART2);
        //收到数据在这里处理
        get_control_dat(get);
    }
}

