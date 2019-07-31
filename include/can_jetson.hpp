

#ifndef CAN_CODE_CAN_BUS_JETSON_HPP
#define CAN_CODE_CAN_BUS_JETSON_HPP

#include "key_jetson.hpp"
#include <array>

using namespace std;

/*class can_jetson{
public:
    int can_init();
    int can_closed();
    int
};*/

// TODO
enum can_bitrate {
  _100k = 100000,
  _500k = 500000,

};

enum canX {

};

int can_init();
int can_closed();
void can_send(const int &can_id, const int &can_dlc, const array<int, 8> &a);
void can_receive(int &id, array<int, 8> &a);

#endif // CAN_CODE_CAN_BUS_JETSON_HPP

// TODO
// parse CAN frame  // 解析命令是否符合规范

// can_send函数使用提示信息
//  Usage: cansend - simple command line tool to send CAN-frames via CAN_RAW
//  sockets. Usage: cansend <device> <can_frame> <can_frame>:
//  <can_id>#{R|data}          for CAN 2.0 frames
//  <can_id>##<flags>{data}    for CAN FD frames
//  <can_id>:
//  can have 3 (SFF) or 8 (EFF) hex chars
//  {data}:
//  has 0..8 (0..64 CAN FD) ASCII hex-values (optionally separated by '.')
//  <flags>:
//  a single ASCII Hex value (0 .. F) which defines canfd_frame.flags
//  Examples:
//  5A1#11.2233.44556677.88 / 123#DEADBEEF / 5AA# / 123##1 / 213##311
//  1F334455#1122334455667788 / 123#R for remote transmission request.
