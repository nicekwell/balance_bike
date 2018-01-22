#include <stm32f10x_lib.h>
#include <stdio.h>
#include <i2c.h>
#include <gy521.h>
#include <uart.h>
#include <disp.h>
#include <motor.h>
#include <angle.h>
#include "balance.c"
#include "control.c"

int flag_disp=0;        //定时器通过此变量通知主循环刷新显示，1-刷新
int disp_type = 0;      //0-电脑上显示界面，1-手柄显示界面

int fputc(int ch, FILE *f)
{
    USART_SendData(USART2, (u8)ch);
    while (SET!=USART_GetFlagStatus(USART2, USART_FLAG_TC));
    return (ch);
}

void delay_init() //用系统定时器查询方式延时
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); //设置成AHB的8分频，也就是9MHz
    SysTick_ITConfig(DISABLE);
}
void udelay(u32 n) //延时us，输入范围 1~1864135
{
    //由于SysTick时钟设置为9MHz，所以延时1us需要设置的寄存器是 9
    u32 reg;
    reg = 9*n;
    SysTick_CounterCmd(SysTick_Counter_Clear); //清空计数器
    SysTick_SetReload(reg); //设置寄存器
    SysTick_CounterCmd(SysTick_Counter_Enable); //开启倒计数
    while(SysTick_GetFlagStatus(SysTick_FLAG_COUNT) == RESET); //等待计时结束
    SysTick_CounterCmd(SysTick_Counter_Disable); //停止计时器
}
void mdelay(u16 n) //延时ms，输入范围 1~1864
{
    //由于SysTick时钟设置为9MHz，所以延时1ms需要设置的寄存器是 9000
    u32 reg;
    reg = 9000*n;
    SysTick_CounterCmd(SysTick_Counter_Clear); //清空计数器
    SysTick_SetReload(reg); //设置寄存器
    SysTick_CounterCmd(SysTick_Counter_Enable); //开启倒计数
    while(SysTick_GetFlagStatus(SysTick_FLAG_COUNT) == RESET); //等待计时结束
    SysTick_CounterCmd(SysTick_Counter_Disable); //停止计时器
}
int test_pin = 0;
void test_pin_init()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    {
        GPIO_InitTypeDef temp;
        temp.GPIO_Speed = GPIO_Speed_50MHz;
        temp.GPIO_Mode = GPIO_Mode_Out_PP;
        temp.GPIO_Pin = GPIO_Pin_12;
        GPIO_Init(GPIOB, &temp);
    }
}
void set_test_pin(int a)
{
    if(a)
    {
        GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
        test_pin = 1;
    }
    else
    {
        GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);
        test_pin = 0;
    }
}
void turn_test_pin()
{
    if(!test_pin)
    {
        GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
        test_pin = 1;
    }
    else
    {
        GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);
        test_pin = 0;
    }
}

void RCC_init()
{
    ErrorStatus HSEStartUpStatus;    //作为存储HSE初始化状态的临时变量
    RCC_DeInit();    //复位
    RCC_HSEConfig(RCC_HSE_ON);    //开启外部高速晶振HSE
    HSEStartUpStatus = RCC_WaitForHSEStartUp();    //等待HSE启动ok或失败
    if(HSEStartUpStatus == SUCCESS)    //启动ok
    {
        //PLL
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);    //设置PLL时钟输入为HSE/1，9倍频。由于HSE是8MHz，所以PLL输出为72MHz
        RCC_PLLCmd(ENABLE);        //使能PLL
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);    //等待PLL输出稳定
     
        //Flash，注意！！在SYSCLK初始化之前一定要设置flash延时周期和预取指令缓存
        FLASH_SetLatency(FLASH_Latency_2);    //设置延时周期
        /*FLASH时序延迟几个周期，等待总线同步操作。
          推荐按照单片机系统运行频率，—MHz时，取Latency=0；
          24—MHz时，取Latency=1；
          48~72MHz时，取Latency=2*/
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);    //预取指缓存使能
     
        //SYSCLK
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);    //选择PLL时钟作为SYSCLK时钟源
        while(RCC_GetSYSCLKSource() != 0x08);        //等待PLL时钟成为SYSCLK时钟源
        //AHB
        RCC_HCLKConfig(RCC_SYSCLK_Div1);    //设置AHB的时钟源为SYSCLK 1分频，也就是72MHz
        RCC_PCLK1Config(RCC_HCLK_Div2);        //设置APB1为HCLK 2分频，也就是36MHz
        RCC_PCLK2Config(RCC_HCLK_Div1);        //设置APB2为HCLK 1分频，也就是72MHz
    }
}

void timer2_init()
{
    /* 使能定时器时钟模块。
       高级定时器TIM1和TIM8挂在高速时钟线APB2上，通用定时器TIM2~TIM7挂在低速时钟线APB1上。
       虽然不同的定时器可能挂在不同的APB总线上，但在通常的APB方案中，传到定时器的时钟都是72MHz。（详细原因见笔记） */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    /* 配置定时器时钟源 */
    // 定时器默认时钟是通过APB过来的，所以这里不需要配置
    /* 配置定时器基本计数单元 */
    {
        TIM_TimeBaseInitTypeDef temp;
        temp.TIM_Prescaler = 72-1;  /* 预分频寄存器，[0, 65535]。不管是那个tim，APB过来的频率都是72MHz。
                                       72Mhz/(TIM_Prescaler+1) = CK_CNT（计数器的时钟频率），TIM_Prescaler = 72MHz/CK_CNT - 1 */
        temp.TIM_Period = 20000-1;     /* 自动装载寄存器，[0, 65535]。
                                          上面预分频设置好计数器周期后(1/CK_CNT)，这个周期的(TIM_Period+1)倍就是计数器的溢出周期。*/
        //结合上面两个，有一个溢出周期公式：溢出周期 = (TIM_Prescaler+1)*(TIM_Period+1)/72000000，单位是s。
        temp.TIM_CounterMode = TIM_CounterMode_Up; /* 计数模式：
                                                      TIM_CounterMode_Up                TIM 向上计数模式
                                                      TIM_CounterMode_Down              TIM 向下计数模式
                                                      TIM_CounterMode_CenterAligned1    TIM 中央对齐模式 1 计数模式
                                                      TIM_CounterMode_CenterAligned2    TIM 中央对齐模式 2 计数模式
                                                      TIM_CounterMode_CenterAligned3    TIM 中央对齐模式 3 计数模式  */
        temp.TIM_ClockDivision = TIM_CKD_DIV1; /* 采样分频，输入捕获时使用。 */
        TIM_TimeBaseInit(TIM2, &temp);
    }
    /* 配置定时器中断，打开溢出中断 */
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    /* 配置nvic，前提是已经在程序最开始执行了 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2) 设置了分组 */
    {
        NVIC_InitTypeDef temp;
        temp.NVIC_IRQChannel = TIM2_IRQChannel;
        temp.NVIC_IRQChannelPreemptionPriority = 1;
        temp.NVIC_IRQChannelSubPriority = 1;
        temp.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&temp);
    }
    /* 使能定时器 */
    TIM_Cmd(TIM2, ENABLE);
}
void TIM2_IRQHandler(void)      //20ms一次中断
{
    set_test_pin(1);
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)     //发生的是溢出事件
    {
        static int count = 0;
        count ++;
        if(count == 5)  //100ms
        {
            count = 0;
            flag_disp = 1;
        }
        balance_tick();
        
        TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);  // 结束后一定要清除标志位
        //哪怕中断里什么也不做，只要tim的中断打开了，在中断处理函数中一定要调用这一个函数把标志位清除，否则可能会影响到其他模块都不能正常工作。
    }
    set_test_pin(0);
}

int main(int argc, char *argv[])
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    RCC_init();
    delay_init();
    test_pin_init();
    uart2_init();
    i2c_init();
    mdelay(500);mpu6050_init();
    motor_init();
    angle_init();
    timer2_init();
    balance_init();

    motor_setzkb(100);
    disp_clean();
    while(1)
    {
        if(flag_disp)
        {
            flag_disp = 0;
            if(disp_type == 0)  //电脑上显示
            {
                disp_gotoxy(1,1);
                disp_clean_line();
                printf("sensordata: ax=%.3f \tay=%.3f \taz=%.3f \tgx=%.3f \tgy=%.3f \tgz=%.3f\n\r", sensor_raw_data.ax, sensor_raw_data.ay, sensor_raw_data.az, sensor_raw_data.gx, sensor_raw_data.gy, sensor_raw_data.gz);
                disp_gotoxy(1, 2);
                disp_clean_line();
                printf("attitude: angle_a=%.3f \tangle_g=%.3f   \tangle=%.3f \tangle_car=%.3f\n\r", attitude_data.angle_a, attitude_data.angle_g, attitude_data.angle, attitude_data.angle_car);
#if BALANCE_TYPE == 1
                disp_gotoxy(1,3);
                disp_clean_line();
                printf("balance: balance_angle=%.3f \tkp=%.3f \tkd=%.3f \td_angle=%.3f \tax=%.3f \tangle=%.3f \n\r", balance_data.balance_angle, balance_data.kp, balance_data.kd, balance_data.d_angle, balance_data.ax, balance_data.angle);
#elif BALANCE_TYPE == 2
                disp_gotoxy(1,3);
                disp_clean_line();
                printf("balance: balance_angle=%.3f \tkp=%.3f \tkd=%.3f \td_angle=%.3f \tax=%.3f \tangle=%.3f \n\r", balance_data.balance_angle, balance_data.kp, balance_data.kd, balance_data.d_angle, balance_data.ax, balance_data.angle);
#endif
            }
            else if(disp_type == 1)     //手柄上显示
            {
#if BALANCE_TYPE == 1
                disp_gotoxy(8, 1);
                disp_clean_line();
                switch(focus)
                {
                case 0: printf("kp"); break;
                case 1: printf("kd"); break;
                case 2: printf("L"); break;
                case 3: printf("Lm"); break;
                case 4: printf("V"); break;
                }
                disp_gotoxy(1,2);
                printf("kp=%.3f  ", balance_data.kp);
                disp_gotoxy(8,2);
                printf("kd=%.3f  ", balance_data.kd);
                disp_gotoxy(1,3);
                printf("L=%.3f  ", balance_data.L);
                disp_gotoxy(8,3);
                printf("Lm=%.3f  ", balance_data.Lm);
                disp_gotoxy(1,4);
                printf("V=%.3f  ", balance_data.V);
                disp_gotoxy(8,4);
                printf("j=%.3f", balance_data.balance_angle);
#elif BALANCE_TYPE == 2
                disp_gotoxy(8, 1);
                disp_clean_line();
                switch(focus)
                {
                case 0: printf("kp"); break;
                case 1: printf("kd"); break;
                }
                disp_gotoxy(1,2);
                printf("kp=%.3f  ", balance_data.kp);
                disp_gotoxy(1,3);
                printf("kd=%.3f  ", balance_data.kd);
#endif
            }
        }
    }
}
