#include <disp.h>
#include <uart.h>
#include <stdio.h>

void disp_clean(void)
{
    printf("\033[2J");
}
void disp_clean_line(void)      //清除当前行，并复位光标到行首
{
    printf("\33[2K\r");
}
void disp_cursor_reset(void)        //复位光标位置，回到左上角
{
    printf("\033[H");
}
void disp_gotoxy(int x, int y)  //跳转到y行x列
{
    printf("\033[%d;%dH", y, x);
}
void disp_cursor_up(int y)      //上移x行
{
    printf("\033[%dA", y);
}
void disp_cursor_down(int y)    //下移y行
{
    printf("\033[%dB", y);
}
void disp_cursor_left(int x)    //左移x列
{
    printf("\033[%dD", x);
}
void disp_cursor_right(int x)   //右移x列
{
    printf("\033[%dC", x);
}
void disp_cursor_hide(void)     //隐藏光标
{
    printf("\033[?25l");
}
void disp_cursor_show(void)     //显示光标
{
    printf("\033[?25h");
}

