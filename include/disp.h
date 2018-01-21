#ifndef __DISP_H__
#define __DISP_H__
/* 在主函数已经完成串口初始化，并且重定向printf函数之后，
   这里提供一些列函数用于操作终端工具显示，可在终端工具里构建界面。 */
#include <stm32f10x_lib.h>

void disp_clean(void);
void disp_clean_line(void);      //清除当前行，并复位光标到行首
void disp_cursor_reset(void);        //复位光标位置，回到左上角（1行1列）
void disp_gotoxy(int x, int y);  //跳转到y行x列，x和y都是从1开始数。
void disp_cursor_up(int y);      //上移x行
void disp_cursor_down(int y);    //下移y行
void disp_cursor_left(int x);    //左移x列
void disp_cursor_right(int x);   //右移x列
void disp_cursor_hide(void);     //隐藏光标，在secureCRT中测试无效。
void disp_cursor_show(void);     //显示光标，在secureCRT中测试无效。

#endif
