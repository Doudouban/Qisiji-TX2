// references :  https://www.jetsonhacks.com/
//
// The MIT License (MIT)
//
// Copyright (c) 2015 Jetsonhacks
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef I2C_DEVICE_H
#define I2C_DEVICE_H

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <fcntl.h> //Needed for I2C port
#include <iostream>
#include <linux/i2c-dev.h> //Needed for I2C port
#include <linux/i2c.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h> //Needed for I2C port
#include <unistd.h>
#include <unistd.h> //Needed for I2C port
#include <vector>

using namespace std;

// I2C_Device 类
class I2C_Device {
public:
  unsigned char kI2CBus;           // I2C bus of the Device
  unsigned char I2CDevice_Address; // I2C Slave Address
  int error;
  bool i2c_status;      // i2c开关状态
  bool i2c_data_status; // i2c数据是否正常

  int I2C_FileDescriptor; // File Descriptor

  unsigned char writeBuf[3] = {0}; // 写文件时缓存
  unsigned char readBuf[2] = {0};  // 读文件时缓存

  I2C_Device();
  ~I2C_Device();

  void set_kI2CBus(unsigned char _kI2CBus);
  void set_I2CDevice_Address(unsigned char _I2CDevice_Address_);

  unsigned int get_kI2CBus();
  unsigned int get_I2CDevice_Address();
  int getError();

  int write_word_I2CDevice(unsigned int writeRegister,
                           unsigned int writeWordValue);
  int read_word_I2CDevice(unsigned int readRegister);
  int write_SMBus_I2CDevice(int writeRegister, int writeValue);
  int read_SMBus_I2CDevice(int readRegister);

  //   private:
  bool open_I2CDevice();  // Open the I2C bus
  void close_I2CDevice(); // Close the I2C bus
};

#endif // I2C_DEVICE_H
