#ifndef __ANGLE_H__
#define __ANGLE_H__
#include <stm32f10x_lib.h>

void angle_init(void);
void angle_setangle(int deg);    //设置角度，单位是度。正前方是0，向左是负，向右是正

#endif
