//
// Created by chao on 25/07/19.
//

#ifndef I2C_JETSONTX2_SRC_ANGLE_ADC_H
#define I2C_JETSONTX2_SRC_ANGLE_ADC_H

#include "i2c_device.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

using namespace std;

////@formatter:off
//    ---- I2C BUSS ----  ADS1115所在的i2c总线
#define ADS1115_I2C_BUSS                0x00    // i2c-0 或 i2c-1
//    ---- I2C ADDRESS/BITS ----  ADS1115的i2c地址
#define ADS1115_ADDRESS                 0x48    // 1001 000 (ADDR = GND)

// ADS10115的配置
//    ---- CONVERSION DELAY (in mS) ----
#define ADS1115_CONVERSIONDELAY         (8)

//    ---- POINTER REGISTER ----
#define ADS1115_REG_POINTER_MASK        (0x03)
#define ADS1115_REG_POINTER_CONVERT     (0x00)
#define ADS1115_REG_POINTER_CONFIG      (0x01)
#define ADS1115_REG_POINTER_LOWTHRESH   (0x02)
#define ADS1115_REG_POINTER_HITHRESH    (0x03)

//    ---- CONFIG REGISTER -----
#define ADS1115_REG_CONFIG_OS_MASK      (0x8000)
#define ADS1115_REG_CONFIG_OS_SINGLE    (0x8000)  // Write: Set to start a single-conversion
#define ADS1115_REG_CONFIG_OS_BUSY      (0x0000)  // Read: Bit = 0 when conversion is in progress
#define ADS1115_REG_CONFIG_OS_NOTBUSY   (0x8000)  // Read: Bit = 1 when device is not performing a conversion

#define ADS1115_REG_CONFIG_MUX_MASK     (0x7000)
#define ADS1115_REG_CONFIG_MUX_DIFF_0_1 (0x0000)  // Differential P = AIN0, N = AIN1 (default)
#define ADS1115_REG_CONFIG_MUX_DIFF_0_3 (0x1000)  // Differential P = AIN0, N = AIN3
#define ADS1115_REG_CONFIG_MUX_DIFF_1_3 (0x2000)  // Differential P = AIN1, N = AIN3
#define ADS1115_REG_CONFIG_MUX_DIFF_2_3 (0x3000)  // Differential P = AIN2, N = AIN3
#define ADS1115_REG_CONFIG_MUX_SINGLE_0 (0x4000)  // Single-ended AIN0
#define ADS1115_REG_CONFIG_MUX_SINGLE_1 (0x5000)  // Single-ended AIN1
#define ADS1115_REG_CONFIG_MUX_SINGLE_2 (0x6000)  // Single-ended AIN2
#define ADS1115_REG_CONFIG_MUX_SINGLE_3 (0x7000)  // Single-ended AIN3

#define ADS1115_REG_CONFIG_PGA_MASK     (0x0E00)
#define ADS1115_REG_CONFIG_PGA_6_144V   (0x0000)  // +/-6.144V range = Gain 2/3
#define ADS1115_REG_CONFIG_PGA_4_096V   (0x0200)  // +/-4.096V range = Gain 1
#define ADS1115_REG_CONFIG_PGA_2_048V   (0x0400)  // +/-2.048V range = Gain 2 (default)
#define ADS1115_REG_CONFIG_PGA_1_024V   (0x0600)  // +/-1.024V range = Gain 4
#define ADS1115_REG_CONFIG_PGA_0_512V   (0x0800)  // +/-0.512V range = Gain 8
#define ADS1115_REG_CONFIG_PGA_0_256V   (0x0A00)  // +/-0.256V range = Gain 16

#define ADS1115_REG_CONFIG_MODE_MASK    (0x0100)
#define ADS1115_REG_CONFIG_MODE_CONTIN  (0x0000)  // Continuous conversion mode
#define ADS1115_REG_CONFIG_MODE_SINGLE  (0x0100)  // Power-down single-shot mode (default)

#define ADS1115_REG_CONFIG_DR_MASK      (0x00E0)
#define ADS1115_REG_CONFIG_DR_128SPS    (0x0000)  // 128 samples per second
#define ADS1115_REG_CONFIG_DR_250SPS    (0x0020)  // 250 samples per second
#define ADS1115_REG_CONFIG_DR_490SPS    (0x0040)  // 490 samples per second
#define ADS1115_REG_CONFIG_DR_920SPS    (0x0060)  // 920 samples per second
#define ADS1115_REG_CONFIG_DR_1600SPS   (0x0080)  // 1600 samples per second (default)
#define ADS1115_REG_CONFIG_DR_2400SPS   (0x00A0)  // 2400 samples per second
#define ADS1115_REG_CONFIG_DR_3300SPS   (0x00C0)  // 3300 samples per second
//------不一致存在错误-----------------------------------------------
//#define CONFIG_REG_DR_8SPS		(0x0000)
//#define CONFIG_REG_DR_16SPS		(0x0020)
//#define CONFIG_REG_DR_32SPS		(0x0040)
//#define CONFIG_REG_DR_64SPS		(0x0060)
//#define CONFIG_REG_DR_128SPS		(0x0080) // default
//#define CONFIG_REG_DR_250SPS		(0x00A0)
//#define CONFIG_REG_DR_475SPS		(0x00C0)
//#define CONFIG_REG_DR_860SPS		(0x00E0)

#define ADS1115_REG_CONFIG_CMODE_MASK   (0x0010)
#define ADS1115_REG_CONFIG_CMODE_TRAD   (0x0000)  // Traditional comparator with hysteresis (default)
#define ADS1115_REG_CONFIG_CMODE_WINDOW (0x0010)  // Window comparator

#define ADS1115_REG_CONFIG_CPOL_MASK    (0x0008)
#define ADS1115_REG_CONFIG_CPOL_ACTV_LOW (0x0000)  // ALERT/RDY pin is low when active (default)
#define ADS1115_REG_CONFIG_CPOL_ACTVHI  (0x0008)  // ALERT/RDY pin is high when active ??
//------不一致存在错误-----------------------------------------------
//#define CONFIG_REG_CPOL_ACTIV_LOW	(0x0000) // default
//#define CONFIG_REG_CPOL_ACTIV_HIGH	(0x0080

#define ADS1115_REG_CONFIG_CLAT_MASK    (0x0004)  // Determines if ALERT/RDY pin latches once asserted
#define ADS1115_REG_CONFIG_CLAT_NONLAT  (0x0000)  // Non-latching comparator (default)
#define ADS1115_REG_CONFIG_CLAT_LATCH   (0x0004)  // Latching comparator
//------不一致存在错误-----------------------------------------------
//#define CONFIG_REG_CLATCH_NONLATCH	(0x0000) // default
//#define CONFIG_REG_CLATCH_LATCH	(0x0040

#define ADS1115_REG_CONFIG_CQUE_MASK    (0x0003)
#define ADS1115_REG_CONFIG_CQUE_1CONV   (0x0000)  // Assert ALERT/RDY after one conversions
#define ADS1115_REG_CONFIG_CQUE_2CONV   (0x0001)  // Assert ALERT/RDY after two conversions
#define ADS1115_REG_CONFIG_CQUE_4CONV   (0x0002)  // Assert ALERT/RDY after four conversions
#define ADS1115_REG_CONFIG_CQUE_NONE    (0x0003)  // Disable the comparator and put ALERT/RDY in high state (default)
////@formatter:on

// I2C_Device 类
class ADC_I2C_ADS1115 : I2C_Device {
public:
  unsigned int config ;             // 配置寄存器的配置值
  unsigned int configRegister;      // 指向ads1115的配置寄存器
  unsigned int conversionRegister;  // 指向ads1115的转换寄存器

  ADC_I2C_ADS1115();
  ~ADC_I2C_ADS1115();

  int adc_i2c_init(unsigned int i2c_bus_, unsigned int i2c_address_);
  void configADCSingleEnded(int channel);
  int readADCSingleEndedValue(int channel);
  float readADCSingleEndedVoltage(int channel);
  //  TODO
  //  float readADC_Differential_0_1(void);
  //  float readADC_Differential_2_3(void);
  int adc_i2c_close();

private:
};

//------------------------------------------------------------------------------
// ADC_I2C_ADS1115 ads1115; // ads1115对象
// API接口函数
int get_angle_init();                   // 初始化角度传感器设备
int get_adc_value(int channel = 2);     // 获取ADC转换的值
float get_adc_voltage(int channel = 2); // 获取ADC转换的电压值
float get_angle(int channel = 2);       // 计算最终角度值
int get_angle_close(); // 关闭角度传感器设备，析构时自动关闭，可不用

#endif // I2C_JETSONTX2_SRC_ANGLE_ADC_H
