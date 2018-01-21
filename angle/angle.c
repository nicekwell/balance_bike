/* 正前方是0度，往左是负，往右是正 */
#include <angle.h>

int mk_mid = 1500+140;        //脉宽中间值，也就是0度的脉宽值，单位是us

void angle_init()
{
    /* 各个定时器的通道引脚如下：
       定时器  TIMx_CH1    TIMx_CH2    TIMx_CH3    TIMx_CH4
       TIM1    PA8         PA9         PA10        PA11
       TIM2    PA0         PA1         PA2         PA3
       TIM3    PA6         PA7         PB0         PB1
       TIM4    PB6         PB7         PB8         PB9
       TIM5    PA0         PA1         PA2         PA3
       TIM6    -           -           -           -
       TIM7    -           -           -           -
       TIM8    PC6         PC7         PC8         PC9     */
    /* 使能定时器时钟模块。
       高级定时器TIM1和TIM8挂在高速时钟线APB2上，通用定时器TIM2~TIM7挂在低速时钟线APB1上。
       虽然不同的定时器可能挂在不同的APB总线上，但在通常的APB方案中，传到定时器的时钟都是72MHz。（详细原因见笔记） */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    /* 配置定时器时钟源 */
    // 定时器默认时钟是通过APB过来的，所以这里不需要配置
    /* 使能将要作为pwm输出的GPIO口时钟，根据上面表格查到 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    /* 配置将要作为pwm输出的引脚的输出模式为"推挽复用输出" */
    {
        GPIO_InitTypeDef temp;
        temp.GPIO_Speed = GPIO_Speed_50MHz;
        temp.GPIO_Mode = GPIO_Mode_AF_PP;
        temp.GPIO_Pin = GPIO_Pin_6;     //根据上面表格设置对应的引脚
        GPIO_Init(GPIOB, &temp);
    }
    /* 配置定时器基本计数单元 */
    {
        TIM_TimeBaseInitTypeDef temp;
        temp.TIM_Prescaler = 72-1;  /* 预分频寄存器，[0, 65535]。不管是那个tim，APB过来的频率都是72MHz。这里72-1设置的是1MHz。
                                       72Mhz/(TIM_Prescaler+1) = CK_CNT（计数器的时钟频率），TIM_Prescaler = 72MHz/CK_CNT - 1 */
        temp.TIM_Period = 20000-1;     /* 自动装载寄存器，[0, 65535]。这里100-1设置的是100个计数器周期作为溢出周期。
                                          上面预分频设置好计数器周期后(1/CK_CNT)，这个周期的(TIM_Period+1)倍就是计数器的溢出周期。*/
        //结合上面两个，有一个溢出周期公式：溢出周期 = (TIM_Prescaler+1)*(TIM_Period+1)/72000000，单位是s。
        temp.TIM_CounterMode = TIM_CounterMode_Up; /* 计数模式：
                                                      TIM_CounterMode_Up                TIM 向上计数模式
                                                      TIM_CounterMode_Down              TIM 向下计数模式
                                                      TIM_CounterMode_CenterAligned1    TIM 中央对齐模式 1 计数模式
                                                      TIM_CounterMode_CenterAligned2    TIM 中央对齐模式 2 计数模式
                                                      TIM_CounterMode_CenterAligned3    TIM 中央对齐模式 3 计数模式  */
        temp.TIM_ClockDivision = TIM_CKD_DIV1; /* 采样分频，输入捕获时使用。 */
        TIM_TimeBaseInit(TIM4, &temp);
    }
    /* 初始化指定定时器指定通道的输出比较参数 */
    {
        TIM_OCInitTypeDef temp;
        temp.TIM_OCMode = TIM_OCMode_PWM1;      //此模式会在占空比为100%时无法输出完全的高电平，会在最后有一个周期的低电平
        temp.TIM_OutputState = TIM_OutputState_Enable;  //使能
        temp.TIM_Pulse = mk_mid;    //比较寄存器的值
        temp.TIM_OCPolarity = TIM_OCPolarity_High;      //有效电平为高电平
        TIM_OC1Init(TIM4, &temp);
    }
    /* 使能指定定时器指定通道的预装载寄存器 */
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
    /* 使能指定定时器的自动重装载的预装载寄存器允许位 */
    TIM_ARRPreloadConfig(TIM4, DISABLE); //ENABLE-比较寄存器值修改后立即生效；DISABLE-比较寄存器值修改后下个周期生效
    /* 使能定时器 */
    TIM_Cmd(TIM4, ENABLE);
}

static void tim4_setmk(int mk)  //设置脉宽，单位是us，范围是0~20000
{
    TIM_SetCompare1(TIM4, mk);
}

void angle_setangle(int deg)    //设置角度，单位是度。正前方是0，向左是负，向右是正
{
    #define ANGLE_LIMIT 40
    double mkPdeg = 2000/180;   //每一度角度对应的脉宽变化量
    if(deg>ANGLE_LIMIT) deg = ANGLE_LIMIT;
    else if(deg < -ANGLE_LIMIT) deg = -ANGLE_LIMIT;
    tim4_setmk(mk_mid - mkPdeg*deg);
}

