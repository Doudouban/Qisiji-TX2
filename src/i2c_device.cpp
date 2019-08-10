////////////////////////////////////////////////////////////////////////////////
//  // jetsonTX2 I2C 原装载板
//    I2C口 ->  Signal  J21接口    I2C设备
//----------------------------------------------
//  I2CBus1 ->  SDA1  ->  Pin3  ->  SDA
//  I2CBus1 ->  SCL1  ->  Pin5  ->  SCL
//          ->  3V3   ->  Pin1  ->  VCC
//          ->  5V    ->  Pin2  ->
//          ->  GND   ->  Pin6  ->  GND
//----------------------------------------------
//  I2CBus0 ->  ID_SD ->  Pin27  ->  SDA
//  I2CBus0 ->  ID_SC ->  Pin28  ->  SCL

// jetsonTX2 I2C 图为007载板
//    I2C口  ->  Signal       接口    I2C设备
//----------->---------------------------------
//  I2CBus1 ->  I2C_DAT2  ->  Pin25  ->  SDA
//  I2CBus1 ->  I2C_CLK2  ->  Pin26  ->  SCL
//          ->   3V3      ->  Pin2   ->  VCC
//          ->   5V       ->  Pin20  ->
//          ->   GND      ->  Pin19  ->  GND
//---------------------------------------------
//  I2CBus0 ->  I2C_DAT2  ->  Pin12  ->  SDA
//  I2CBus0 ->  I2C_CLK2  ->  Pin11  ->  SCL
////////////////////////////////////////////////////////////////////////////////

#include "i2c_device.h"

// 指定I2C_Device 总线bus及Slave设备地址
I2C_Device::I2C_Device() {
  // Default I2C bus for device on Jetson TX2
  kI2CBus = 0;              // Desired I2C bus on Jetson TX2  # i2c0 or i2c1
  I2CDevice_Address = 0x00; // Desired I2C Address on Device
  error = 0;
  I2C_FileDescriptor = 0;
  i2c_status = false;
  i2c_data_status = false;
}

I2C_Device::~I2C_Device() {
  //  if (i2c_status)
  close_I2CDevice();
}

void I2C_Device::set_kI2CBus(unsigned char kI2CBus_) { kI2CBus = kI2CBus_; }

//
void I2C_Device::set_I2CDevice_Address(unsigned char I2CDevice_Address_) {
  I2CDevice_Address = I2CDevice_Address_;
}

unsigned int I2C_Device::get_kI2CBus() {
  cout << "kI2CBus: " << hex << kI2CBus << endl;
  return (int)kI2CBus;
}

unsigned int I2C_Device::get_I2CDevice_Address() {
  cout << "I2CDevice_Address: 0x" << hex << I2CDevice_Address << endl;
  return (int)I2CDevice_Address;
}

// 打开地址I2CDevice_Address的i2c设备
// Returns true if device file descriptor opens correctly, false otherwise
bool I2C_Device::open_I2CDevice() {
  char fileNameBuffer[32];
  sprintf(fileNameBuffer, "/dev/i2c-%d", kI2CBus);
  I2C_FileDescriptor = open(fileNameBuffer, O_RDWR);
  if (I2C_FileDescriptor < 0) {
    // Could not open the file
    cout << "Failed to open the i2c bus: " << kI2CBus << endl;
    error = errno;
    i2c_status = false;
    return false;
  }
  if (ioctl(I2C_FileDescriptor, I2C_SLAVE, I2CDevice_Address) < 0) {
    // Could not open the device on the bus
    cout << "Failed to acquire bus access and/or talk to slave: 0x" << hex
         << I2CDevice_Address << endl;
    error = errno;
    i2c_status = false;
    return false;
  }
  // 成功打开i2c
  i2c_status = true;
  return true;
}

// 关闭i2c设备
void I2C_Device::close_I2CDevice() {
  if (I2C_FileDescriptor > 0) { // ??
    close(I2C_FileDescriptor);
    // WARNING - This is not quite right, need to check for error first
    cout << "close_I2CDevice : WARNING - This is not quite right, need to "
            "check for error first."
         << endl;
    I2C_FileDescriptor = -1;
  }
  i2c_status = false;
}

// i2c一次写两个字节word
int I2C_Device::write_word_I2CDevice(unsigned int writeRegister,
                                     unsigned int writeWordValue) {
  int writeValueMSB = writeWordValue >> 8;
  int writeValueLSB = writeWordValue && 0xFF;
  // 写入配置
  writeBuf[0] = writeRegister; // ads1115 0x01指向配置寄存器
  writeBuf[1] = writeValueMSB; // 配置值高八位MSB??
  writeBuf[2] = writeValueLSB; // 配置值低八位LSB??
  // 向i2c文件写入三个字节，1byte寄存器+2byte数据
  error = write(I2C_FileDescriptor, writeBuf, 3); // 写入配置
  if (error < 0) {
    cout << "write error!! " << error << endl;
    return error;
  }
  usleep(25);
  return 0;
}

// i2c一次读两个字节word
int I2C_Device::read_word_I2CDevice(unsigned int readRegister) {
  int twoByteValue = 0;
  unsigned char highByteValueMSB;
  unsigned char lowByteValueLSB;

  writeBuf[0] = readRegister; // ads1115 0x00指向转换寄存器
  error = write(I2C_FileDescriptor, writeBuf, 1); // 指向转换寄存器
  if (error < 0) {
    cout << "write error!! " << error << endl;
    return error;
  }
  // 读取转换寄存器的值，两个字节
  // 读取的数据存储到readBuf[],read返回值error为读取的字节数
  // if (read(I2C_FileDescriptor, readBuf, 2) != 2) // read data and check error
  error = read(I2C_FileDescriptor, readBuf, 2); // read返回值为读取的字节数
  if (error != 2) {
    cout << "Error : ADS1115 Input/Output Error!! " << error << endl;
    return error;
  } else {
    highByteValueMSB = readBuf[0]; // 值高八位MSB
    lowByteValueLSB = readBuf[1];  // 值高八位MSB
    twoByteValue = highByteValueMSB << 8 | lowByteValueLSB;
  }

  return 0;
}

/*
//【1】__s32 i2c_smbus_write_quick(int file, __u8 value);
//发送一个写控制字，可用于测试I2C设备是否存在。
// S Addr Rd [A] [Data] NA P
//
//【2】__s32 i2c_smbus_read_byte(int file);
//发送一个读控制字，并从I2C设备中读取一个字节。
// S Addr Rd [A] [Data] NA P
//
//【3】__s32 i2c_smbus_write_byte(int file, __u8 value);
//发送一个写控制字，并向I2C设备中写入一个字节。
// S Addr Wr [A] Data [A] P
//
//【4】__s32 i2c_smbus_read_byte_data(int file, __u8 command);
//向I2C设备发送一个写控制字+控制指令（寄存器地址），再发送一个读控制字，此时I2C从设备内部的读写指针转移到指定的位置，并返回一个字节，最后返回一个无应答NA。
// S Addr Wr [A] Comm [A] S Addr Rd [A] [Data] NA P
//
//【5】__s32 i2c_smbus_write_byte_data(int file, __u8 command, __u8 value);
//向I2C设备发送一个写控制字+控制指令（寄存器地址），紧接着发送指令内容（寄存器内容，单字节）。
// S Addr Wr [A] Comm [A] Data [A] P
//
// i2c_smbus_read_word_data()
// i2c_smbus_write_word_data()
// 同上面类似，只是一次读写两个字节。
//
//【6】__s32 i2c_smbus_read_i2c_block_data(int file, __u8 command,__u8 *values);
//向I2C设备发送一个写控制字+控制指令（寄存器地址），再发送一个读控制字，此时I2C从设备内部的读写指针转移到指定的位置，并连续返回多个字节，I2C主机读取到一定数量字节内容之后发送无应答NA。
// S Addr Wr [A] Comm [A] S Addr Rd [A] [Data] A [Data] A ... A [Data] NA P
//
//【7】__s32 i2c_smbus_write_i2c_block_data(int file, __u8 command, __u8 length,
//__u8 *values);
//向I2C设备发送一个写控制字+控制指令（寄存器地址），接着发送发送指令内容（寄存器内容，多字节）
// S Addr Wr [A] Comm [A] Data [A] Data [A] ... [A] Data [A] P
*/

//// Write the the given value to the given register on the device
//// i2c_smbus_write_byte() to write single byte.
// int I2C_Device::write_SMBus_I2CDevice(int writeRegister, int writeValue) {
//  /* Using SMBus commands */
//  int toReturn =
//      i2c_smbus_write_byte_data(I2C_FileDescriptor, writeRegister,
//      writeValue);
//  // cmake # target_link_libraries(main i2c)  // cmake里必须连接i2c？？
//  //  i2c_smbus_write_byte_data(int file, __u8 command, __u8 value);
//  //
//  向I2C设备发送一个写控制字+控制指令（寄存器地址），紧接着发送一个字节指令内容（寄存器内容）。
//  //  ---- S Addr Wr [A] Comm [A] Data [A] P -----
//  // Wait a little bit to make sure it settles
//  usleep(1000);
//  if (toReturn < 0) {
//    error = errno;
//    i2c_data_status = false;
//    toReturn = -1;
//  }
//  return toReturn;
//}

//// Read the given register on the device
////i2c_smbus_read_byte_data() to read
// int I2C_Device::read_SMBus_I2CDevice(int readRegister) {
//  int toReturn;
//  toReturn = i2c_smbus_read_byte_data(I2C_FileDescriptor, readRegister);
//  //  i2c_smbus_read_byte_data(int file, __u8 command);
//  //
//  向I2C设备发送一个写控制字+控制指令（寄存器地址），再发送一个读控制字，此时I2C从设备内部的读写指针转移到指定的位置，并返回一个字节，最后返回一个无应答NA。
//  //  ---- S Addr Wr [A] Comm [A] S Addr Rd [A] [Data] NA P ----
//  if (toReturn < 0) {
//    error = errno;
//    i2c_data_status = false;
//    toReturn = -1;
//  }
//  return toReturn;
//}

// Return the last i/o error
int I2C_Device::getError() { return error; }
