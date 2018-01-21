/*
  舵机那边提供的接口是 angle_setangle()，传入角度：正前方是0，向右是正，向左是负。
  坐标系：认为水平向右方向是x轴正方向，竖直向上方向是y轴正方向。
  自行车角度：竖直方向是0度，向右是正，向左是负。
  陀螺仪测出的角速度也是，向右旋转为正，向左旋转为负。
 */

#include <math.h>
/**************************** 传感器部分 *************************/
typedef struct {
    double ax_cali;
    double ay_cali;
    double az_cali;
    double gx_cali;
    double gy_cali;
    double gz_cali;
} sensor_raw_data_cali_t;
sensor_raw_data_cali_t sensor_raw_data_cali;
typedef struct {        //这是原始数据，没有经过映射，但经过了校准
    double ax;
    double ay;
    double az;
    double gx;
    double gy;
    double gz;
} sensor_raw_data_t;
sensor_raw_data_t sensor_raw_data;
void get_sensor_data()  //读取数据并校准，数据保存在sensor_raw_data里
{
    sensor_raw_data.ax = -mpu6050_getAccelX() + sensor_raw_data_cali.ax_cali;
//    sensor_raw_data.ay = -mpu6050_getAccelY() + sensor_raw_data_cali.ay_cali;
    sensor_raw_data.az = -mpu6050_getAccelZ() + sensor_raw_data_cali.az_cali;
//    sensor_raw_data.gx = mpu6050_getGyroX() + sensor_raw_data_cali.gx_cali;
    sensor_raw_data.gy = mpu6050_getGyroY() + sensor_raw_data_cali.gy_cali;
    sensor_raw_data.gz = mpu6050_getGyroZ() + sensor_raw_data_cali.gz_cali;
}
/*************************** 姿态部分 **********************/
#define PI 3.141592654
typedef struct {
    double ax;  //映射后的ax，水平向右是正方向
    double ay;  //映射后的ay，竖直向上是正方向
    double palstance;   //映射后的角速度，向右是正，向左是负

    //角度，竖直向上是0，向右是正，向左是负
    double angle_a;     //加速度计算出的角度
    double angle_g;     //角速度叠加计算出的角度
    double angle;       //互补平衡滤波后得到的角度

    /* 小车倾角
       上面算出来的是姿态传感器相对于重力竖直方向的角度，而小车由于重心偏差，平衡位置不一定是竖直向上方向。
       这里叠加重心所在角度，得出小车偏离重心的角度。
     */
    double angle_core;  //重心所在角度，对于一个小车而言是固定值
    double angle_car;   //偏离重心的角度，这才是我们进行PID算法时的偏差角
} attitude_data_t;
attitude_data_t attitude_data;
void attitude_1()
{
    static double angle_last;
    attitude_data.ax = -sensor_raw_data.ax;
    attitude_data.ay = sensor_raw_data.az/0.97; //由于姿态传感器安装偏差，用0.97这个系数和近似消除偏差
    attitude_data.palstance = -sensor_raw_data.gy/0.97 - sensor_raw_data.gz*0.25;  //单位是度每秒
    //加速度计算角度
    attitude_data.angle_a = -atan(attitude_data.ax/attitude_data.ay)*180/PI;    //得到的单位是角度每秒
    //陀螺仪计算角度
    attitude_data.angle_g = angle_last + attitude_data.palstance*0.02;
    //互补平衡滤波
#define A_GYRO 0.99 //陀螺仪所占的比例
    attitude_data.angle = A_GYRO*attitude_data.angle_g + (1-A_GYRO)*attitude_data.angle_a;
    angle_last = attitude_data.angle;

    //计算偏离小车重心的角度
    attitude_data.angle_car = attitude_data.angle - attitude_data.angle_core;
}
/*************************** 平衡部分 ***************************/
#define BALANCE_TYPE 1  //1-物理模型较为详细。2-没有详细的物理模型，角度直接对应加速度。

#if BALANCE_TYPE == 1
typedef struct {
    double balance_angle;       //目标平衡角度
    double kp, kd;
    double L, Lm, V;    //L是前后轮触底点距离，Lm是重心距离后轮的距离，V是前进速度。都是国际单位
    double d_angle;     //角度偏差，目标角度-当前角度，向右倾斜偏差为正，向左倾斜偏差为负
    double ax;          //倒立摆横向加速度，向右是正，向左是负
    double angle;       //方向盘的角度，正前方是0，向右是正，向左是负
} balance_data_t;
balance_data_t balance_data;
#define limit_angle     40      //限幅大小，单位是角度
void balance_1()
{
    double temp;
    balance_data.d_angle = balance_data.balance_angle - attitude_data.angle_car;

    balance_data.ax = balance_data.kp*balance_data.d_angle - balance_data.kd*attitude_data.palstance;
    temp = -balance_data.ax/balance_data.V/balance_data.V*balance_data.Lm;
    if(temp < -1) temp=-1;
    else if(temp > 1) temp=1;
    balance_data.angle = atan((balance_data.L/balance_data.Lm)*tan(asin(temp)))*180/PI;
    if(balance_data.angle > limit_angle) balance_data.angle = limit_angle;
    else if(balance_data.angle < -limit_angle) balance_data.angle = -limit_angle;
    angle_setangle((int)balance_data.angle);
}
#elif BALANCE_TYPE == 2
typedef struct {
    double balance_angle;       //目标平衡角度
    double kp, kd;
    double d_angle;     //角度偏差，目标角度-当前角度，向右倾斜偏差为正，向左倾斜偏差为负
    double ax;          //倒立摆横向加速度，向右是正，向左是负
    double angle;       //方向盘的角度，正前方是0，向右是正，向左是负
} balance_data_t;
balance_data_t balance_data;
#define limit_angle     40      //限幅大小，单位是角度
void balance_2()
{
    balance_data.d_angle = balance_data.balance_angle - attitude_data.angle_car;
    balance_data.angle = -balance_data.kp*balance_data.d_angle + balance_data.kd*attitude_data.palstance;
    if(balance_data.angle > limit_angle) balance_data.angle = limit_angle;
    else if(balance_data.angle < -limit_angle) balance_data.angle = -limit_angle;
    angle_setangle((int)balance_data.angle);
}
#endif

void balance_tick()  //20ms一次
{
    get_sensor_data();
    attitude_1();
#if BALANCE_TYPE == 1
    balance_1();
#elif BALANCE_TYPE == 2
    balance_2();
#endif
}

void balance_init()
{
    sensor_raw_data_cali.ax_cali=0.61;
    sensor_raw_data_cali.ay_cali=0;
    sensor_raw_data_cali.az_cali=0.3;
    sensor_raw_data_cali.gx_cali=5.85;
    sensor_raw_data_cali.gy_cali=-1.91;
    sensor_raw_data_cali.gz_cali=-19.9;

    attitude_data.ax = 0;
    attitude_data.ay = 0;
    attitude_data.palstance = 0;
    attitude_data.angle_a = 0;
    attitude_data.angle_g = 0;
    attitude_data.angle = 0;
    attitude_data.angle_core = -1.22;       //重心角度
    attitude_data.angle_car = 0;

#if BALANCE_TYPE == 1
    balance_data.balance_angle = 0;
    balance_data.kp = 0.1;
    balance_data.kd = 0.01;
    balance_data.L=0.145;
    balance_data.Lm=0.035;
    balance_data.V=0.3625;
    balance_data.d_angle = 0;
    balance_data.ax = 0;
    balance_data.angle = 0;
#elif BALANCE_TYPE == 2
    balance_data.balance_angle = 0;
    balance_data.kp = 6;
    balance_data.kd = 0.5;
    balance_data.d_angle = 0;
    balance_data.ax = 0;
    balance_data.angle = 0;
#endif
}


