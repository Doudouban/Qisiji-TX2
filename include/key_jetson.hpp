#ifndef KEY_JETSON_HPP
#define KEY_JETSON_HPP

// 定义引脚
// 常变量
// const unsigned int  LED1_id 398
// const unsigned int  KEY1_id 397
// const unsigned int  KEY2_id 389
// const unsigned int  KEY3_id 481

// gpio引脚需定义为全局变量,在头文件里定义会报重复定义错误
// jetsonTX2GPIONumber LED1 = gpio398;
// jetsonTX2GPIONumber KEY1 = gpio397;
// jetsonTX2GPIONumber KEY2 = gpio389;
// jetsonTX2GPIONumber KEY3 = gpio481;

// 读取引脚状态
//#define KEY1_val bcm2835_gpio_lev(KEY1)
//#define KEY2_val bcm2835_gpio_lev(KEY2)
//#define KEY3_val bcm2835_gpio_lev(KEY3)
// int KEY1_val,KEY2_val,KEY3_val;
// gpioGetValue(KEY1, &KEY1_val);

int key();
int key_scan(int);
// int led(int);
void delay_ms(int);
int gpio_init();
int gpio_close();

#endif // KEY_JETSON_HPP
