//
// Created by chao on 25/07/19.
//

// jetsonTX2 I2C 图为007载板
//    CAN口  ->  Signal     拓展接口    ADS1115模块
//----------->---------------------------------
//    i2c0  ->  I2C_CLK1  ->  Pin11 ->  SCL
//    i2c0  ->  I2C_DAT1  ->  Pin12 ->  SDA
//          ->    5V      ->  Pin20 ->  VCC
//          ->   GND      ->  Pin19 ->  GND
//---------------------------------------------
//    CAN1  ->  CAN1_TX  ->  Pin21  ->  TX
//    CAN1  ->  CAN1_RX  ->  Pin22  ->  RX

#include "ads1115_i2c.h"

using namespace std;

ADC_I2C_ADS1115::ADC_I2C_ADS1115() {
  //  kI2CBus = 0x01;           // 默认ads1115所在的i2c总线，
  //  I2CDevice_Address = 0x48; // 默认ads1115地址，
  config = 0;                // 配置寄存器的配置值
  configRegister = 0x01;     // 指向ads1115的配置寄存器
  conversionRegister = 0x00; // 指向ads1115的转换寄存器
}

ADC_I2C_ADS1115::~ADC_I2C_ADS1115() {
  if (i2c_status)
    close_I2CDevice();
}

//  // ADS1115 读取数据过程
//  1.配置值写入配置寄存器
//    1.1 写指针寄存器，指向配置寄存器
//      发起写请求  0x90
//      指向配置 config register  0x01
//    1.2写入配置值
//      配置值config MSB  0X84(例如)
//      配置值config LSB  0x83(例如)
//  2.读取转换寄存器的结果值
//    2.1写指针寄存器，指向转换寄存器
//       发起写请求 0x90
//       指向转换寄存器 conversion register 0x00
//    2.2.读取转换寄存器的值
//       发起读请求  0x91
//       读高八位MSB值
//       读低八位LSB值

// ADC 设备特指ADS1115
// i2c初始化 默认i2c_bus=0 ， i2c_address=0x48
int ADC_I2C_ADS1115::adc_i2c_init(unsigned int i2c_bus_ = 0,
                                  unsigned int i2c_address_ = 0x48) {
  unsigned int adc_i2c_bus = i2c_bus_;
  unsigned int adc_i2c_address = i2c_address_;
  // 初始化i2c_bus和i2c_address
  set_kI2CBus(adc_i2c_bus);
  set_I2CDevice_Address(adc_i2c_address);
  // 打开i2c
  int i2c_err = open_I2CDevice();
  if (i2c_err < 0) {
    cout << "Open i2c Error: %d" << error << endl;
    return error;
  } else {
    cout << "Successfully Open i2c ！\n"
         << "adc_i2c_bus: " << hex << adc_i2c_bus << " , "
         << "i2c_address: 0x" << hex << adc_i2c_address << endl;
    return 0;
  }
}

// 中间环节暂不可单独使用
void ADC_I2C_ADS1115::configADCSingleEnded(int channel) {

  config = ADS1115_REG_CONFIG_OS_SINGLE | ADS1115_REG_CONFIG_PGA_6_144V |
           ADS1115_REG_CONFIG_MODE_SINGLE | ADS1115_REG_CONFIG_DR_128SPS |
           ADS1115_REG_CONFIG_CMODE_TRAD | ADS1115_REG_CONFIG_CPOL_ACTV_LOW |
           ADS1115_REG_CONFIG_CLAT_NONLAT | ADS1115_REG_CONFIG_CQUE_NONE;

  switch (channel) {
  case 0:
    config |= ADS1115_REG_CONFIG_MUX_SINGLE_0;
    break;
  case 1:
    config |= ADS1115_REG_CONFIG_MUX_SINGLE_1;
    break;
  case 2:
    config |= ADS1115_REG_CONFIG_MUX_SINGLE_2;
    break;
  case 3:
    config |= ADS1115_REG_CONFIG_MUX_SINGLE_3;
    break;
  default:
    cout << "Give a channel between 0-3" << endl;
  }
  //  1.配置值写入配置寄存器
  // configRegister配置寄存器，config配置的值 2byte
  write_word_I2CDevice(configRegister, config);
}

// 仅适用于ads1115芯片
int ADC_I2C_ADS1115::readADCSingleEndedValue(int channel = 0) {
  int adcValue = 0;
  unsigned char adcValueMSB;
  unsigned char adcValueLSB;
  configADCSingleEnded(channel);
  usleep(135000);

  //  2.读取转换寄存器的结果值
  // conversionRegister转换寄存器，读取的转换结果2byte存到readBuf[]
  read_word_I2CDevice(conversionRegister);

  adcValueMSB = readBuf[0]; // ADC值高八位MSB
  adcValueLSB = readBuf[1]; // ADC值高八位MSB
  adcValue = adcValueMSB << 8 | adcValueLSB;
  return adcValue; // ADC转换的结果2byte
}

float ADC_I2C_ADS1115::readADCSingleEndedVoltage(int channel = 0) {
  int adcValue = 0;
  float adcVoltage = 0;
  // 配置的full-scale
  float FullScale = 6.144; // positive full-scale(+FS) = 6.144
                           // negative full-scale(-FS) = -6.144
  adcValue = readADCSingleEndedValue(channel);
  // 6.144为config配置的电压量程值，32767为2^15=32768
  //      电压值范围adcVoltage   ：    寄存器值范围adcValue
  // 超正限         >= +FS      ：       32767=0x7fff
  // 正压    +FS/2^15V -- +FS   ： 0=0x0001  -- 32767=0x7fff （从小到大）
  //  零             0V         ：          0=0x00
  // 负压    -FS -- -FS/2^15V   ： 32768=0x8000 -- 65535=0xffff （从小到大）
  // 超负限         <= -FS      ：       32768=0x8000
  if (adcValue == 0x7fff) // 超正限
    adcVoltage = FullScale * (0x7fff - 1) / 0x7fff;
  else if (adcValue >= 0 && adcValue <= 0x7fff)        // 正压
    adcVoltage = (float)FullScale * adcValue / 0x7fff; // 2^15=0x7fff=32767
  else if (adcValue == 0x0000)                         // 零
    adcVoltage = 0;
  else if (adcValue >= 0x8000 && adcValue <= 0xffff) // 负压
    adcVoltage = (float)FullScale * (adcValue - 0xffff - 1) /
                 0x7fff;       // 1^16=0xffff=65535
  else if (adcValue == 0x8000) // 超负限
    adcVoltage = -FullScale;
  else
    adcVoltage = 0; // 异常值

  return adcVoltage;
}

// 关闭打开的i2c设备
int ADC_I2C_ADS1115::adc_i2c_close() {
  if (i2c_status)
    close_I2CDevice();
}

//------------------------------------------------------------------------------
ADC_I2C_ADS1115 ads1115; // ads1115对象

// 初始化
int get_angle_init() {
  int init_error;
  // 所用的I2C bus
  // ads1115 的 I2C Address
  unsigned int ads1115_i2c_bus = ADS1115_I2C_BUSS;
  unsigned int ads1115_i2c_address = ADS1115_ADDRESS;
  init_error = ads1115.adc_i2c_init(ads1115_i2c_bus, ads1115_i2c_address);
  if (init_error < 0) {
    printf("Init ads1115 Error: %d", init_error);
    return -1;
  } else {
    printf("Init ads1115 Successfully！！\n ");
    return 0;
  }
}

// 获取ADC转换的值
int get_adc_value(int channel) {
  int adc_value = 0;
  adc_value = ads1115.readADCSingleEndedValue(channel);
  return adc_value;
}

// 获取ADC转换的电压值
float get_adc_voltage(int channel) {
  float adc_voltage = 0;
  adc_voltage = ads1115.readADCSingleEndedVoltage(channel);
  return adc_voltage;
}

// channel 2
//// 计算最终角度值,测量量为电阻量
float get_angle(int channel) {
  float adcResistanceValue;
  float baffleAngle = 0;
  float adcVoltage = 0;
  adcVoltage = ads1115.readADCSingleEndedVoltage(channel);
  // 1k-3k可变电阻12V供电，串联参考电阻753欧
  adcResistanceValue = (11.99 - adcVoltage) / adcVoltage * 753;
  baffleAngle = (adcResistanceValue-1000)/2000*90; // 电阻与角度函数关系
  return baffleAngle;
}

//// 计算最终角度值，测量量为电压
// float get_angle(int channel) {
//  float adcVoltage = 0;
//  ////零位电压偏移量,需测量的常量
//  float offsetVoltage = 0.513578; // 零位电压偏移量,需测量
//  float baffleAngle = 0;
//  adcVoltage = ads1115.readADCSingleEndedVoltage(channel);
//  // 计算挡板角度（角度为相对起始位置角度）
//  // 角度传感器0.5V--4.5V 对应 0--90度
//  baffleAngle =
//      (float)(adcVoltage - offsetVoltage) / (4.5 - 0.5) * 90; // 分辨率0.011度
//  // 角度传感器 0V--5V 对应 0--360度
//  //  baffleAngle = (float)(adcVoltage - offsetVoltage) / (5 - 0) * 360;   //
//  //  分辨率0.049度
//
//  return baffleAngle;
//}

// 关闭角度传感器设备
int get_angle_close() { ads1115.adc_i2c_close(); }
