#if BALANCE_TYPE == 1

extern int disp_type;      //0-电脑上显示界面，1-手柄显示界面
int focus=0;    //当前调节的焦点
                //0-kp，1-kd，2-L，3-Lm，4-V
void focus_plus()       //焦点+1
{
    focus++;
    if(focus == 5)
        focus = 0;
}
void focus_minus()      //焦点-1
{
    focus--;
    if(focus == -1)
        focus = 4;
}
void content_plus()     //当前选中的内容+
{
    switch(focus)
    {
    case 0:     //kp
        balance_data.kp+=0.01;
        break;
    case 1:     //kd
        balance_data.kd+=0.001;
        break;
    case 2:     //L
        balance_data.L+=0.001;
        break;
    case 3:     //Lm
        balance_data.Lm += 0.001;
        break;
    case 4:     //V
        balance_data.V += 0.001;
        break;
    }
}
void content_minus()    //当前选中的内容-
{
    switch(focus)
    {
    case 0:     //kp
        balance_data.kp-=0.01;
        break;
    case 1:     //kd
        balance_data.kd-=0.001;
        break;
    case 2:     //L
        balance_data.L-=0.001;
        break;
    case 3:     //Lm
        balance_data.Lm -= 0.001;
        break;
    case 4:     //V
        balance_data.V -= 0.001;
        break;
    }
}

void get_control_dat(u8 dat)
{
    //键盘控制部分
    switch(dat)
    {
    case 'u':
        balance_data.balance_angle+=1;
        break;
    case 'j':
        balance_data.balance_angle-=1;
        break;
    case 'i':
        balance_data.kp+=0.01;
        break;
    case 'k':
        balance_data.kp-=0.01;
        break;
    case 'I':
        balance_data.kp+=0.1;
        break;
    case 'K':
        balance_data.kp-=0.1;
        break;
    case 'o':
        balance_data.kd+=0.001;
        break;
    case 'l':
        balance_data.kd-=0.001;
        break;
    case 'O':
        balance_data.kd+=0.01;
        break;
    case 'L':
        balance_data.kd-=0.01;
        break;
    }
    //手柄控制部分
    {
        static u8 flag=0;      //0-当前数据什么也不是。1-当前数据是Lx值，2-当前数据是Ly值，3-当前数据是Rx值，4-当前数据是Ry值。
        s8 temp;
        switch(flag)
        {
        case 0:
            if(dat == 150)
                flag = 1;
            else if(dat == 151)
                flag = 2;
            else if(dat == 152)
                flag = 3;
            else if(dat == 153)
                flag = 4;
            else        //这里是一般的按键处理
            {
                switch(dat)
                {
                case 200:       //1(三角)
                    content_plus();
                    break;
                case 202:       //2 圆
                    disp_type = 1;
                    focus_plus();
                    break;
                case 204:       //3 叉
                    content_minus();
                    break;
                case 206:       //4 方块
                    focus_minus();
                    break;
                case 208:       //上
                    break;
                case 210:       //右按下
                    balance_data.balance_angle = 2;
                    break;
                case 211:       //右释放
                    balance_data.balance_angle = 0;
                    break;
                case 212:       //下
                    break;
                case 214:       //左按下
                    balance_data.balance_angle = -2;
                    break;
                case 215:       //左释放
                    balance_data.balance_angle = 0;
                    break;
                }
            }
            break;
        case 1: //Lx
            flag = 0;
            temp = (s8)dat;
            if(temp >= -100 && temp <= 100)
            {
            }
            break;
        case 2: //Ly
            flag = 0;
            break;
        case 3: //Rx
            flag = 0;
            break;
        case 4: //Ry
            flag = 0;
            break;
        }
    }
}

#elif BALANCE_TYPE == 2

extern int disp_type;      //0-电脑上显示界面，1-手柄显示界面
int focus=0;    //当前调节的焦点
                //0-kp，1-kd
void focus_plus()       //焦点+1
{
    focus++;
    if(focus == 2)
        focus = 0;
}
void focus_minus()      //焦点-1
{
    focus--;
    if(focus == -1)
        focus = 1;
}
void content_plus()     //当前选中的内容+
{
    switch(focus)
    {
    case 0:     //kp
        balance_data.kp+=0.1;
        break;
    case 1:     //kd
        balance_data.kd+=0.01;
        break;
    }
}
void content_minus()    //当前选中的内容-
{
    switch(focus)
    {
    case 0:     //kp
        balance_data.kp-=0.1;
        break;
    case 1:     //kd
        balance_data.kd-=0.01;
        break;
    }
}
void get_control_dat(u8 dat)
{
    //键盘控制部分
    switch(dat)
    {
    case 'u':
        balance_data.balance_angle+=1;
        break;
    case 'j':
        balance_data.balance_angle-=1;
        break;
    case 'i':
        balance_data.kp+=0.1;
        break;
    case 'k':
        balance_data.kp-=0.1;
        break;
    case 'I':
        balance_data.kp+=1;
        break;
    case 'K':
        balance_data.kp-=1;
        break;
    case 'o':
        balance_data.kd+=0.01;
        break;
    case 'l':
        balance_data.kd-=0.01;
        break;
    case 'O':
        balance_data.kd+=0.1;
        break;
    case 'L':
        balance_data.kd-=0.1;
        break;
    }
    //手柄控制部分
    {
        static u8 flag=0;      //0-当前数据什么也不是。1-当前数据是Lx值，2-当前数据是Ly值，3-当前数据是Rx值，4-当前数据是Ry值。
        s8 temp;
        switch(flag)
        {
        case 0:
            if(dat == 150)
                flag = 1;
            else if(dat == 151)
                flag = 2;
            else if(dat == 152)
                flag = 3;
            else if(dat == 153)
                flag = 4;
            else        //这里是一般的按键处理
            {
                switch(dat)
                {
                case 200:       //1(三角)
                    content_plus();
                    break;
                case 202:       //2 圆
                    disp_type = 1;
                    focus_plus();
                    break;
                case 204:       //3 叉
                    content_minus();
                    break;
                case 206:       //4 方块
                    focus_minus();
                    break;
                case 208:       //上
                    break;
                case 210:       //右按下
                    break;
                case 211:       //右释放
                    break;
                case 212:       //下
                    break;
                case 214:       //左按下
                    break;
                case 215:       //左释放
                    break;
                }
            }
            break;
        case 1: //Lx
            flag = 0;
            temp = (s8)dat;
            if(temp >= -100 && temp <= 100)
            {
            }
            break;
        case 2: //Ly
            flag = 0;
            break;
        case 3: //Rx
            flag = 0;
            break;
        case 4: //Ry
            flag = 0;
            break;
        }
    }
}

#endif
