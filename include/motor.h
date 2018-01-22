#ifndef __MOTOR_H__
#define __MOTOR_H__
#include <stm32f10x_lib.h>

void motor_init(void);
void motor_setzkb(int zkb);      //设置占空比，范围是0~100

#endif
