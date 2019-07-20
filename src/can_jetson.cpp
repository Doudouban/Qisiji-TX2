// g++ -Wall main.cpp can_bus_jetson.cpp -o main
// sudo ./main

//    CAN口  ->  Signal     J26接口    CAN收发器
//----------->----------------------------------
//    CAN0  ->  CAN0_RX  ->  Pin5   ->  RX
//    CAN0  ->  CAN0_TX  ->  Pin7   ->  TX
//          ->  VDD_3V3  ->  Pin2   ->  VCC
//          ->  GND      ->  Pin10  ->  GND
//    CAN1  ->  CAN1_RX  ->  Pin15  ->  RX
//    CAN1  ->  CAN1_TX  ->  Pin17  ->  TX

#include "can_jetson.hpp"
#include <iostream>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

using namespace std;

// TODO

//默认can0波特率500k
// int can_init(int device = 0,int bitrate=500000,int mode=0) {
int can_init() {
  // 安装canbus模块
  system("sudo modprobe can");    // 插入can总线子系统
  system("sudo modprobe can_dev"); // 插入can_dev模块
  system("sudo modprobe can-raw"); // 插入can原始协议模块
  system("sudo modprobe can-bcm"); // the broadcast manager (BCM)
  system("sudo modprobe can-gw");  // ？？
  system("sudo modprobe mttcan"); // 真正的can口支持(Real CAN interface support)
  system("lsmod");                // 检查canbus模块是否安装成功
  // 配置can总线
  system("sudo ip link set can0 down");                    // 关闭can0
  system("sudo ip link set can0 type can bitrate 500000"); // 设置can0波特率500k
  system("sudo ip link set can0 up");                      // 挂载can0
  system("ifconfig"); // # 检查can是否挂载成功
  system("ip -details -statistics link show can0"); // 检查can口状态
  // TODO 异常捕获
  printf("CAN bus init !!\r\n");
  //  system("sudo bash ../sys_can_init.sh");

  return 0;
}
// TODO
int can_closed() {
  system("sudo ip link set can0 down"); // 关闭can0
  return 0;
}

//-----------------------------------------------------------------------------------------
// TODO
//  Usage: can_send <device> <can_frame>
//  <can_frame>: <can_id>#{R|data}          //for CAN 2.0 frames
//  can_send(<device>,<can_id>#{R|data})
//  <can_id> ：3位或8位16进制数，3位标准数据帧，8位扩展帧
//  {data}： 0..8字节，最多16位数据，16进制数， (可以使用.作为分隔符)
//  {R}：发送远程帧
//-----------------------------------------------------------------------------------------
//  #标准数据帧
//  cansend( can0 5A1#1122334455667788 ) #
//  <can_id>为0x5A1，data数据内容为0x1122334455667788 #扩展帧 cansend( can0
//  12345678#aabbccdd ) # <can_id>为0x12345678，<data>内容为0xaabbccdd #远程帧
//  cansend( can0 123#R7 )  # <can_id>为0x123，长度为7
//-----------------------------------------------------------------------------------------

// 终端命令 cansend can0 5A1#1122334455667788
// 函数调用 cansend(can0，5A1#1122334455667788)

// int can_send(int device = 0) {
void can_send(const int& can_id, const int& can_dlc, const array<int,8>& a) {
  int ret;
  int s, nbytes;
  struct sockaddr_can addr;
  struct ifreq ifr;
  struct can_frame frame;
  memset(&frame, 0, sizeof(struct can_frame));

  //  system("sudo ip link set can0 type can bitrate 100000");
  //  system("sudo ifconfig can0 up");
  printf("this is a can send demo\r\n");

  // 1.Create socket
  s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (s < 0) {
    perror("socket PF_CAN failed");

  }

  // 2.Specify can0 device
  strcpy(ifr.ifr_name, "can0");
  ret = ioctl(s, SIOCGIFINDEX, &ifr);
  if (ret < 0) {
    perror("ioctl failed");

  }

  // 3.Bind the socket to can0
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  ret = bind(s, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    perror("bind failed");

  }

  // 4.Disable filtering rules, do not receive packets, only send
  setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

  // 5.Set send data
  frame.can_id = can_id;
  frame.can_dlc = can_dlc;
  frame.data[0] = a[0];
  frame.data[1] = a[1];
  frame.data[2] = a[2];
  frame.data[3] = a[3];
  frame.data[4] = a[4];
  frame.data[5] = a[5];
  frame.data[6] = a[6];
  frame.data[7] = a[7];

  printf("can_id  = 0x%X\r\n", frame.can_id);
  printf("can_dlc = %d\r\n", frame.can_dlc);
  int i = 0;
  for (i = 0; i < 8; i++)
    printf("data[%d] = %d\r\n", i, frame.data[i]);

  // 6.Send message
  nbytes = write(s, &frame, sizeof(frame));
  if (nbytes != sizeof(frame)) {
    printf("Send Error frame[0]!\r\n");
    //    system("sudo ifconfig can0 down");
  }

  // 7.Close the socket and can0
  close(s);
  //  system("sudo ifconfig can0 down");
}

//-----------------------------------------------------------
// TODO
// int can_receive(int device=0,int can_id=0) {
void can_receive(int& id, array<int,8>& a) {
  int ret;
  int s, nbytes;
  unsigned long len;
  struct sockaddr_can addr;
  struct ifreq ifr;
  struct can_frame frame;

  memset(&frame, 0, sizeof(struct can_frame));

  //  system("sudo ip link set can0 type can bitrate 100000");
  //  system("sudo ifconfig can0 up");
  printf("this is a can receive demo\r\n");

  // 1.Create socket
  s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (s < 0) {
    perror("socket PF_CAN failed");

  }

  // 2.Specify can0 device
  strcpy(ifr.ifr_name, "can0");
  ret = ioctl(s, SIOCGIFINDEX, &ifr);
  if (ret < 0) {
    perror("ioctl failed");

  }

  // 3.Bind the socket to can0
  addr.can_family = PF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  ret = bind(s, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    perror("bind failed");

  }


  // 4.Define receive rules
  /*struct can_filter rfilter[1];
  rfilter[0].can_id = 0x123;
  rfilter[0].can_mask = CAN_SFF_MASK;
  setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));*/

  // 5.Receive data and exit
  while (1) {
    nbytes = read(s, &frame, sizeof(frame));
    if (nbytes > 0) {
      printf("can_id = 0x%X\r\ncan_dlc = %d \r\n", frame.can_id, frame.can_dlc);
      id=frame.can_id;
      int i = 0;
      for (i = 0; i < 8; i++){
          printf("data[%d] = %d\r\n", i, frame.data[i]);
          a[i]=frame.data[i];
      }


      break;
    }
  }

  // 6.Close the socket and can0
  close(s);
  //  system("sudo ifconfig can0 down");

}


