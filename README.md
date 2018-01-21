
# 概述

这是一个自平衡自行车，用买来的自行车模型改装。

![balance_bike.jpg](https://code.aliyun.com/nicekwell/balance_bike/raw/master/balance_bike.jpg)

视频:   
<https://www.youtube.com/watch?v=wNYvMlPewzI>  
<http://www.bilibili.com/video/av11733477/>

加入了电机，驱动后轮前进；  
加入了舵机，控制转向。

通过控制转向使自行车能自己平衡前进。

# 硬件和引脚

单片机: stm32f103c8t6

引脚 | 功能 | 备注
:-: | :-: | :-: 
PA9 | 下载TXD | 
PA10 | 下载RXD | 串口1
PA2 | 通信TXD |
PA3 | 通信RXD | 串口2
PB0 | GY521 I2C SCL | 
PB1 | GY521 I2C SDA | 
PA6 | 电机PWM | TIM3 CH1，通过三极管驱动电机，高电平时导通电机，电机正极接的是升压模块的12V。
PB6 | 舵机PWM | TIM4 CH1，通过两个三极管连接舵机，输出端电平是5v，高低电平状态和这里同相。

20ms定时器：TIM2

# 程序结构

核心算法部分是main/balance.c文件，控制部分是main/control.c文件，main/main.c文件里的主循环进行信息显示。

其他各个模块提供接口给以上三个部分：

模块 | 文件 | 接口
:-: | :-: | :-:
延时 | main.c | delay_init();mdelay();udelay();
测试脚 | main.c | test_pin_init();GPIO_WriteBits(GPIOB, GPIO_Pin_12, );
串口和printf | uart/uart.c, include/uart.h | uart_init(),printf(),disp()
i2c | i2c/i2c.c, include/i2c.h | 
gy521 | gy521/gy521.c, include/gy521.h | 
motor | motor/motor.c, include/motor.h | motor_init(), motor_setzkb()
angle | angle/angle.c, include/angle.h | angle_init(), angle_setangle()




