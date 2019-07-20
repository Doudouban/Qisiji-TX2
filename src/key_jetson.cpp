// g++ -Wall main.cpp key_jetson.cpp jetsonGPIO.c -o main
// sudo ./main
//  按键按,点亮LED
// 按键IO需下拉，检测高电平

// function / sysfs_gpio / Pin  / Name
//--------------------------------------------------
//    LED  ->  gpio398  / Pin29 / GPIO5     -- GND
//    KEY1 ->  gpio397  / Pin13 / GPIO_GEN2 -- GND
//    KEY2 ->  gpio389  / Pin33 / GPIO13    -- GND
//    KEY3 ->  gpio481  / Pin18 / GPIO_GEN5 -- GND

#include "key_jetson.hpp"
#include "jetsonGPIO.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/time.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
using namespace std;

// gpio引脚需定义为全局变量
jetsonTX2GPIONumber LED1 = gpio398;
jetsonTX2GPIONumber KEY1 = gpio397;
jetsonTX2GPIONumber KEY2 = gpio389;
jetsonTX2GPIONumber KEY3 = gpio481;

extern bool detec;
extern bool key_detection;
extern bool key_hegih;
extern bool key_background;
// 定义引脚常变量
const unsigned int LED1_id = 398;
const unsigned int KEY1_id = 397;
const unsigned int KEY2_id = 389;
const unsigned int KEY3_id = 481;


int key() {

  int key_num = key_scan(0);
  if (key_num) {
    switch (key_num) {
    case KEY1_id:
      cout << " ---- KEY1 PRESS ---- \n";
      key_detection = true;
      break;
    case KEY2_id:
      cout << " ---- KEY2 PRESS ---- \n";
      key_hegih = true;
      break;
    case KEY3_id:
      cout << " ---- KEY3 PRESS ---- \n";
      key_background = !key_background;
      break;
    default:;
    }
  } else
    delay_ms(10); // 毫秒级

  return 0;
}

// key_mode:
//    0,不支持连续按，前一次必须为松开的状态，这次检测到低点平才有效,按键按下了没有松开只算一次
//    1,支持连续按,不管前一次的状态，只要检测到低点平就有效,按键按下了没有松开算多次
int key_scan(int key_mode = 0) { // 默认模式
  //  printf("key_scan()\n");
  // 读取引脚状态
  unsigned int KEY1_val, KEY2_val, KEY3_val;
  gpioGetValue(KEY1, &KEY1_val);
  //  printf("KEY1_val = %d\n", KEY1_val);
  gpioGetValue(KEY2, &KEY2_val);
  //  printf("KEY2_val = %d\n", KEY2_val);
  gpioGetValue(KEY3, &KEY3_val);
  //  printf("KEY3_val = %d\n", KEY3_val);

  // 键盘扫描
  // 静态变量，只初始化一次,调用之后值仍然保留，用于记录上一次按键的状态,
  static int key_up =
      1; // 按键按松开状态标志,记录上一次按键的状态,一开始是松开的
  if (key_mode ==
      1) // 若支持连按mode=1，则每次开始键盘扫描时按键状态都设为未按下,只要捕获到按下就为有效按下
    key_up = 1; //  按键状态为未按下

  // 检测高电平，检测到高电平为按键按下
  // 读取管脚状态,是否有按键按下
  if ((key_up == 1) &&
      (KEY1_val == 1 || KEY2_val == 1 || KEY3_val == 1)) { // 有按键按下
    // 延时消抖
    delay_ms(10);
    key_up = 0; // 记录这次按键状态为按下还没松开，按键没有按松开标志
    if (KEY1_val == 1) // 消抖后再判断一次确实有按键按下
      return KEY1_id;  // 按键按下有效
    else if (KEY2_val == 1)
      return KEY2_id;
    else if (KEY3_val == 1)
      return KEY3_id;

  } else if (KEY1_val == 0 && KEY2_val == 0 &&
             KEY3_val == 0) { // 按键松开了或根本没有被按下
    key_up = 1; // 按键松开，按键松开状态标志置１或者没有按下
    //      printf("key up / free  \n");
  }

  return 0;
}

int led(int n) {
  //  printf("led()\n");
  // n 为亮的次数
  for (int i = 0; i < n; i++) {
    gpioSetValue(LED1, high);
    delay_ms(300);
    gpioSetValue(LED1, low);
    delay_ms(300);
  }

  return 0;
}

void delay_ms(int ms) {
  //  printf("delay_ms()\n");
  // sleep(ms??);  // 秒s
  //  select(ms);  // 毫秒ms
  // pselect()  // 纳秒
  usleep(ms * 1000); // 微妙us
  // nanosleep(ms*1000000);  // 纳秒
}

int gpio_init() {
  //  printf("gpio_init()\n");

  // 初始化gpio端口
  // 指定gpio端口
  // gpio引脚需定义为全局变量
  //  jetsonTX1GPIONumber LED1 = gpio398;
  //  jetsonTX1GPIONumber KEY1 = gpio8;
  //  jetsonTX1GPIONumber KEY2 = gpio9;
  //  jetsonTX1GPIONumber KEY3 = gpio10;
  // 全局变量声明
  //  extern jetsonTX1GPIONumber LED1;
  //  extern jetsonTX1GPIONumber KEY1;
  //  extern jetsonTX1GPIONumber KEY2;
  //  extern jetsonTX1GPIONumber KEY3;

  // led_gpio初始化
  // 查询引脚是否已经开启，开启则注销重启
  if (gpioOpen(LED1) >= 0)
    gpioUnexport(LED1); // 注销led_gpio
  gpioExport(LED1);     // 注册gpio
  gpioSetDirection(
      LED1,
      outputPin); // 设置gpio方向，同时设为输出引脚初始化电平为低0！！
                  //  gpioSetValue(LED1, low); // 置初值，置0
  //  gpioSetValue(LED1, low);  // 置初值

  // key_GPIO初始化
  // 查询引脚是否已经开启，开启则注销重启
  if (gpioOpen(KEY1) >= 0)
    gpioUnexport(KEY1); // 注销key_gpio
  if (gpioOpen(KEY2) >= 0)
    gpioUnexport(KEY2);
  if (gpioOpen(KEY3) >= 0)
    gpioUnexport(KEY3);
  gpioExport(KEY1); // 注册gpio
  gpioExport(KEY2);
  gpioExport(KEY3);
  // 设置gpio方向，设为输入引脚初始化电平不确定!!!，为上一次输入电平（开机时为0）！！,作为输入引脚必须上拉或下拉,上拉1k电阻小了100欧电阻不能省，此处下拉
  gpioSetDirection(KEY1, inputPin); // 设置gpio方向
  gpioSetDirection(KEY2, inputPin);
  gpioSetDirection(KEY3, inputPin);
  // 查看初始化后引脚的状态
  //  unsigned int init_LED1_val, init_KEY1_val, init_KEY2_val, init_KEY3_val;
  //  gpioGetValue(LED1, &init_LED1_val);
  //  gpioGetValue(KEY1, &init_KEY1_val);
  //  gpioGetValue(KEY2, &init_KEY2_val);
  //  gpioGetValue(KEY3, &init_KEY3_val);
  //  printf("init_LED1_val = %d\n", init_LED1_val); // 初始化后为0
  //  printf("init_KEY1_val = %d\n", init_KEY1_val); // 下拉0
  //  printf("init_KEY2_val = %d\n", init_KEY2_val); //
  //  printf("init_KEY3_val = %d\n", init_KEY3_val); //

  //  置初值，置1,作为输入引脚是不允许赋值的，默认引脚状态浮空??
  //  gpioSetValue(KEY1, high);  // 输入引脚不允许赋值

  // Reverse the button wiring; this is for when the button is wired
  // with a pull up resistor
  // 可直接设置检测电平相反
  //  gpioActiveLow(KEY1, true); // 设置低点平有效
  //  gpioActiveLow(KEY2, true); // 设置低点平有效
  //  gpioActiveLow(KEY3, true); // 设置低点平有效

  return 0;
}

int gpio_close() {
  //  printf("gpio_close()\n");
  // 关闭gpio端口
  // 注销led_gpio
  gpioUnexport(LED1);
  // 注销key_gpio
  gpioUnexport(KEY1);
  gpioUnexport(KEY2);
  gpioUnexport(KEY3);

  return 0;
}
