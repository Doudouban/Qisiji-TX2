//  // jetsonTX2 GPIO 原装载板
//    CAN口  ->  Signal     J26接口    CAN收发器
//----------------------------------------------
//    CAN0  ->  CAN0_RX  ->  Pin5   ->  RX
//    CAN0  ->  CAN0_TX  ->  Pin7   ->  TX
//          ->  VDD_3V3  ->  Pin2   ->  VCC
//          ->  GND      ->  Pin10  ->  GND
//----------------------------------------------
//    CAN1  ->  CAN1_RX  ->  Pin15  ->  RX
//    CAN1  ->  CAN1_TX  ->  Pin17  ->  TX

// jetsonTX2 GPIO 图为007载板
//    CAN口  ->  Signal     J26接口    CAN收发器
//----------->---------------------------------
//    CAN0  ->  CAN0_TX  ->  Pin23  ->  TX
//    CAN0  ->  CAN0_RX  ->  Pin24  ->  RX
//          ->  3V3      ->  Pin1   ->  VCC
//          ->  GND      ->  Pin19  ->  GND
//---------------------------------------------
//    CAN1  ->  CAN1_TX  ->  Pin21  ->  TX
//    CAN1  ->  CAN1_RX  ->  Pin22  ->  RX

#include "can_jetson.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

// 初始化canX口
// 默认 can0口 波特率 500k
int can_init(unsigned int can_device, unsigned int baudRate) {
    int error = 0;
    char dev[8] = {0};
    char cmd[128] = {0};
    // 安装canbus模块
    bzero(cmd, sizeof(cmd));
    sprintf(cmd, "sudo modprobe can");
    if (system(cmd) < 0) {
        printf("sudo modprobe can failed \n");
        error = -1;
    }
    bzero(cmd, sizeof(cmd));
    sprintf(cmd, "sudo modprobe can_dev");
    if (system(cmd) < 0) {
        printf("sudo modprobe can_dev failed \n");
        error = -1;
    }
    bzero(cmd, sizeof(cmd));
    sprintf(cmd, "sudo modprobe can-raw");
    if (system(cmd) < 0) {
        printf("sudo modprobe can-raw failed \n");
        error = -1;
    }
    bzero(cmd, sizeof(cmd));
    sprintf(cmd, "sudo modprobe can-bcm");
    if (system(cmd) < 0) {
        printf("sudo modprobe can-bcm failed \n");
        error = -1;
    }
    bzero(cmd, sizeof(cmd));
    sprintf(cmd, "sudo modprobe can-gw");
    if (system(cmd) < 0) {
        printf("sudo modprobe can-gw failed \n");
        error = -1;
    }
    bzero(cmd, sizeof(cmd));
    sprintf(cmd, "sudo modprobe mttcan");
    if (system(cmd) < 0) {
        printf("sudo modprobe mttcan failed \n");
        error = -1;
    }

    // 安装canbus模块
    //    system("sudo modprobe can");     // 插入can总线子系统
    //    system("sudo modprobe can_dev"); // 插入can_dev模块
    //    system("sudo modprobe can-raw"); // 插入can原始协议模块
    //    system("sudo modprobe can-bcm"); // the broadcast manager (BCM)
    //    system("sudo modprobe can-gw");  // ？？
    //    system("sudo modprobe mttcan");  // 真正的can口支持(Real CAN interface support)
    //    system("lsmod");                 // 检查canbus模块是否安装成功

    // 配置can总线
    bzero(cmd, sizeof(cmd));
    sprintf(dev, "can%d", can_device);
    printf("can dev : %s \n", dev);
    //关闭can设备
    sprintf(cmd, "sudo ifconfig %s down", dev);
    if (system(cmd) < 0) {
        printf("can device shut down failed  \n");
        error = -1;
    }
    //设置can设备波特率
    bzero(cmd, sizeof(cmd));
    sprintf(cmd, "sudo ip link set %s type can bitrate %d ", dev, baudRate);
    printf("can bitrate : %d \n", baudRate);
    if (system(cmd) < 0) {
        printf("set can device baud rate failed  \n");
        error = -1;
    }
    //打开can设备
    bzero(cmd, sizeof(cmd));
    sprintf(cmd, "sudo ifconfig %s up", dev);
    if (system(cmd) < 0) {
        printf("can device open failed  \n");
        error = -1;
    }

    // 配置can总线
    //    system("sudo ip link set can0 down");                    // 关闭can0
    //    system("sudo ip link set can0 type can bitrate 250000"); // 设置can0波特率500k
    //    system("sudo ip link set can0 up");               // 挂载can0
    //    system("ifconfig");                               // # 检查can是否挂载成功
    //    system("ip -details -statistics link show can0"); // 检查can口状态

    if (error == 0)
        printf("CAN bus init OK !\r\n");

    return error;
}

// 关闭 canX 口
int can_closed(unsigned int can_device) {
    char dev[8] = {0};
    char cmd[128] = {0};
    bzero(cmd, sizeof(cmd));
    sprintf(dev, "can%d", can_device);
    bzero(cmd, sizeof(cmd));
    sprintf(cmd, "sudo ifconfig %s down", dev);
    if (system(cmd) < 0) {
        printf("can device close failed  \n");
        return -1;
    }
    //    system("sudo ip link set can0 down"); // 关闭can0
    return 0;
}

//--------------------------------------------------------------------------
// TODO
//  Usage: can_send <device> <can_frame>
//  <can_frame>: <can_id>#{R|data}          //for CAN 2.0 frames
//  can_send(<device>,<can_id>#{R|data})
//  <can_id> ：3位或8位16进制数，3位标准数据帧，8位扩展帧
//  {data}： 0..8字节，最多16位数据，16进制数， (可以使用.作为分隔符)
//  {R}：发送远程帧
//--------------------------------------------------------------------------
//  #标准数据帧 <can_id> 3 hex chars
//  cansend( can0 123#1122334455667788 )
//  # <can_id>为0x123，data数据内容为0x1122334455667788
//  #扩展帧     <can_id> 8 hex chars
//  cansend( can0 12345678#aabbccdd )
//  # <can_id>为0x12345678，<data>内容为0xaabbccdd
//  #远程帧
//  cansend( can0 123#R7 )
//  # <can_id>为0x123，长度为7
//--------------------------------------------------------------------------
// 终端命令 cansend can0 5A1#1122334455667788
// 函数调用 cansend(can0，5A1#1122334455667788)

// 发送 can_id 优先级ID，can_dlc 数据长度，a 数据内容数组
// unsigned int can_id 为CAN帧标识符(CAN_id+帧标志位)
void can_send(unsigned int &can_id, unsigned int &can_dlc, const array<int, 8> &a) {
    int ret;
    int s;
    int nbytes; // write结果 error
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame; // CAN数据帧
    memset(&frame, 0, sizeof(struct can_frame));

    printf("### CAN send function ###\r\n");

    // 1.Create socket
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW); //创建 SocketCAN 套接字
    if (s < 0) {
        perror("socket PF_CAN failed");
    }

    // 2.Specify can0 device
    strcpy(ifr.ifr_name, "can0");
    ret = ioctl(s, SIOCGIFINDEX, &ifr); //指定 can0 设备
    if (ret < 0) {
        perror("ioctl failed");
    }

    // 3.Bind the socket to can0
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    ret = bind(s, (struct sockaddr *)&addr, sizeof(addr)); //将套接字与 can0 绑定
    if (ret < 0) {
        perror("bind failed");
    }

    // 禁用过滤规则,只发送,不接收报文
    // 4.Disable filtering rules, do not receive packets, only send
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

    // 关闭/开启本地回环
    int loopback = 0; // 0 表示关闭, 1 表示开启( 默认)
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));
    // 发送套接字上的回环功能
    // int ro = 1; // 0 表示关闭( 默认), 1 表示开启
    // setsockopt(s, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &ro, sizeof(ro));

    // 合成 frame.can_id 帧的标识符
    // frame.can_id 为帧的标识符,0-28bitCAN优先级，高3位bit是帧的标识位
    // 标准帧标识符，标识符长度为11位，0～10位bit；2^11个CAN_id优先级
    // 扩展帧标识符，标识符长度为29位，0～28位bit。2^29个CAN_id优先级
    //             * bit 0-28 : CAN识别符 (11/29 bit)
    //             * bit 29 : 错误帧标志 (0 = data frame, 1 = error frame)
    //             * bit 30 : 远程发送请求标志 (1 = rtr frame)
    //             * bit 31 :帧格式标志 (0 = standard 11 bit, 1 = extended 29 bit
    // can_id 的第 29、 30、 31 位是帧的标志位，用来定义帧的类型，定义如下：
    // #define CAN_EFF_FLAG 0x80000000U //扩展帧的标识
    // #define CAN_RTR_FLAG 0x40000000U //远程帧的标识
    // #define CAN_ERR_FLAG 0x20000000U //错误帧的标识，用于错误检查
    unsigned int can_id_flag = 0;
    // 数据帧：包含用于传输的节点数据的帧
    // 标准帧 2^11=0x7ff
    if (can_id <= 0x7ff) // 标准帧 2^11=0x7ff
    {
        can_id_flag = can_id;
        // cout << "can_id <= 0x7ff" << endl;
    }
    // 扩展帧 2^29=0x1fffffff
    else if (can_id <= 0x1fffffff) // 扩展帧 2^29=0x1fffffff
    {
        can_id_flag = can_id | CAN_EFF_FLAG;
        // cout << "can_id <= 0x1fffffff" << endl;
    } else {
        // can_id_flag = can_id;
        cout << "can_send can_id error  " << endl;
    }
    // 远程帧：请求传输特定标识符的帧
    //        can_id = can_id | CAN_RTR_FLAG ;
    // 错误帧：由任何检测到错误的节点发送的帧
    //        can_id = can_id | CAN_ERR_FLAG ;
    // 过载帧：在数据帧或远程帧之间插入延迟的帧

    // 5.Set send data
    frame.can_id = can_id_flag; // CAN帧标识符(CAN_id+帧标志位)
    frame.can_dlc = can_dlc;    //数据场的长度
    frame.data[0] = a[0];       //数据
    frame.data[1] = a[1];
    frame.data[2] = a[2];
    frame.data[3] = a[3];
    frame.data[4] = a[4];
    frame.data[5] = a[5];
    frame.data[6] = a[6];
    frame.data[7] = a[7];

    printf("can_send : \ncan_id = 0x%X\r\ncan_dlc = %d \r\n", can_id, can_dlc);
    for (int i = 0; i < 8; i++)
        printf("data[%d] = %X\r\n", i, frame.data[i]);

    // 6.Send message
    nbytes = write(s, &frame, sizeof(frame)); //发送数据
    if (nbytes != sizeof(frame)) {            //如果 nbytes 不等于帧长度，就说明发送失败
        printf("Send Error frame[0]!\r\n");
    }

    // 7.Close the socket and can0
    close(s);
    //  system("sudo ifconfig can0 down");
}

// 接收can数据帧  id CAN优先级ID，a 数据内容数组
void can_receive(unsigned int &id, array<int, 8> &a) {
    int ret;
    int s;
    int nbytes;
    unsigned long len;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame; // CAN数据帧

    // printf("CAN receive function wait for can_frame \r\n");

    memset(&frame, 0, sizeof(struct can_frame)); // 清空frame内存

    // 1.Create socket
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW); //创建套接字
    if (s < 0) {
        perror("socket PF_CAN failed");
    }

    // 2.Specify can0 device
    strcpy(ifr.ifr_name, "can0");
    ret = ioctl(s, SIOCGIFINDEX, &ifr); //指定 can0 设备
    if (ret < 0) {
        perror("ioctl failed");
    }

    // 3.Bind the socket to can0
    addr.can_family = PF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    ret = bind(s, (struct sockaddr *)&addr, sizeof(addr)); //将套接字与 can0 绑定
    if (ret < 0) {
        perror("bind failed");
    }

    // 过滤规则设置
    // 4.Define receive rules
    //     struct can_filter rfilter[1];
    //    rfilter[0].can_id = 0x123;  // 只接收指定CAN_id,只接收表示符等于 0x123 的报文
    //    //rfilter[0].can_mask = CAN_SFF_MASK; // 标准帧
    //    rfilter[0].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
    //    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));  //设置规则

    // 通过错误掩码可以实现对错误帧的过滤
    // can_err_mask_t err_mask = ( CAN_ERR_TX_TIMEOUT | CAN_ERR_BUSOFF );
    // setsockopt(s, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, err_mask, sizeof(err_mask));

    // 5.Receive data and exit
    // while (true) {
    // read接收报文,会阻塞
    // cout << "------------------wait for can_frame ------------------- " << endl;
    nbytes = read(s, &frame, sizeof(frame)); //接收报文,返回值nbytes为读取的数据长度
    printf("### CAN receive function ###\r\n");
    // cout << "can_receive : nbytes : " << nbytes << endl;
    if (nbytes < 0)
        printf("can_receive error !\n!");
    else if (nbytes == 0) { // 存在=0？？
        printf("can_receive : nbytes == 0 \n!");
    } else if (nbytes < (int)sizeof(struct can_frame))
        printf("can_receive read incomplete CAN frame !\n");
    else { // 正确收到数据帧
        // 数据帧：包含用于传输的节点数据的帧
        // 解析CAN_id,高3位为标志位,MSB
        // #define CAN_EFF_FLAG 0x80000000U //扩展帧的标识
        // #define CAN_RTR_FLAG 0x40000000U //远程帧的标识
        // #define CAN_ERR_FLAG 0x20000000U //错误帧的标识，用于错误检查
        unsigned int can_id = frame.can_id;          // MSB
        if ((can_id & CAN_EFF_FLAG) == CAN_EFF_FLAG) // 扩展帧 2^29
        {
            can_id = can_id ^ CAN_EFF_FLAG; // 去除最高位标志位
                                            // cout << "can_receive : can_id = can_id ^ CAN_EFF_FLAG " << endl;
        } else {
            can_id = can_id;
        }
        //            if ((can_id & CAN_RTR_FLAG) == CAN_RTR_FLAG) // 远程帧
        //            {
        //                can_id = can_id ^ CAN_RTR_FLAG; // 去除标志位
        //            }
        //            if ((can_id & CAN_ERR_FLAG) == CAN_ERR_FLAG) // 错误帧
        //            {
        //                can_id = can_id ^ CAN_ERR_FLAG; // 去除标志位
        //            }
        //            else if ((0x7ff - can_id) >= 0) // 标准帧 2^11=0x7ff  ??
        //                can_id = can_id;
        printf("can_receive : \ncan_id = 0x%X\r\ncan_dlc = %d \r\n", can_id, frame.can_dlc);
        id = can_id;
        int i = 0;
        for (i = 0; i < 8; i++) { //显示报文
            printf("data[%d] = %X\r\n", i, frame.data[i]);
            a[i] = frame.data[i];
        }
    }
    // break;
    //}

    // 6.Close the socket and can0
    close(s);
    //  system("sudo ifconfig can0 down");
}
