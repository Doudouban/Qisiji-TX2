#include "common.hpp"
#include "ddban.h"
#include "llsu.hpp"
#include "can_jetson.hpp"
#include "jetsonGPIO.hpp"
#include "key_jetson.hpp"
#include "mainwindow.h"
#include "ads1115_i2c.h"
#include "i2c_device.h"
#include <QApplication>
#include <cassert>
#include <cmath>
#include <limits>
#include <thread>
#include <time.h>


static char buffer[1024 * 1024 * 20];
static int n;
static volatile bool exit_main;
static volatile bool save_frame;

//主函数全局变量声明
// vector<Point2f> fallPoints_2D;//2D点阵图中不加平移的落点集
//车斗相关参数mm(默认值)
extern int height_of_basket; //车筐的高度
extern int range_low;
extern int range_high; //作为高度是否正确的判断条件,同时作为车筐边沿提取的上下边界值
extern int new_rang_low;
extern int new_rang_high;
extern int range_down;
extern int range_up; // 作为车斗上下高度截取的范围
extern int ds;

extern int fall_step;    //落点之间的间距
extern int thick_of_box; //箱子的边缘厚度
extern int z_range_x;
extern int z_range_y;   //求取落点高度平均值的范围
extern int plu;         //二次寻找落点平均高度时拓宽的范围倍数
extern int x_axisFallPoint; //根据满溢程度标记落点
//extern int window_size; //窗口大小
extern int pipe_range; //绘制喷洒的范围离中心列的距离 cols/pipe_range
// int first_frame=8;//起始帧 debug param
// int last_frame=99;//尾帧数 debug param
extern int results_save;
extern int depth_save;
extern int color_save;
extern int cloud_save;

bool key_detection = false;
bool key_height = false;
bool key_background = false;
bool has_Color = false;
bool get_height = true;
bool key_ = true;
int detection = 1;

static TY_CAMERA_INTRINSIC m_colorIntrinsic;

static int linepoints_mistime = 0;

struct CallbackData {
  int index;
  TY_DEV_HANDLE hDevice;
  DepthRender *render;

  TY_CAMERA_DISTORTION color_dist;
  TY_CAMERA_INTRINSIC color_intri;
};

void while_key() {
    while (key_) {
        key();
    }
    gpio_close();
}

void while_control(control &a) {
    while (key_) {
        a.receive_control();
    }
}

void handleFrame(TY_FRAME_DATA *frame, void *userdata, MainWindow &w,
                 control &contr1) {
    cout << "llsu3" << endl;
    CallbackData *pData = (CallbackData *) userdata;
    LOGD("=== Get frame %d", ++pData->index);

    Mat depth_, p3d, color, irl, irr;
    Mat depth_1;
    parseFrame(*frame, &depth_, &irl, &irr, &color, &p3d);
/**************************************************************************************/

    depth_1 = depth_;
    depth_1.convertTo(depth_1, CV_8UC1, 32.0 / 255); //进行位深度的转换,2->1字节
    //转换为8位无符号单通道数值，然后乘32.0再除以255；
    Mat depth = Mat::zeros(depth_1.rows, depth_1.cols, CV_8UC3);
    vector<Mat> channels;
    for (int i = 0; i < 3; i++) {
        channels.push_back(depth_1); //进行通道数的转换,从1->3
    }
    merge(channels, depth); //合并三个通道，
/**********************************************************************************/

    cv::Mat background_pic;
    cv::Mat resizedColor;
    if (!color.empty()) {
        cv::Mat undistort_result(color.size(), CV_8UC3);
        TY_IMAGE_DATA dst;
        dst.width = color.cols;
        dst.height = color.rows;
        dst.size = undistort_result.size().area() * 3;
        dst.buffer = undistort_result.data;
        dst.pixelFormat = TY_PIXEL_FORMAT_RGB;
        TY_IMAGE_DATA src;
        src.width = color.cols;
        src.height = color.rows;
        src.size = color.size().area() * 3;
        src.pixelFormat = TY_PIXEL_FORMAT_RGB;
        src.buffer = color.data;
        // undistort camera image
        // TYUndistortImage accept TY_IMAGE_DATA from TY_FRAME_DATA , pixel format
        // RGB888 or MONO8 you can also use opencv API cv::undistort to do this job.
        ASSERT_OK(TYUndistortImage(&pData->color_intri, &pData->color_dist, NULL,
                                   &src, &dst));
        color = undistort_result;
        cv::resize(color, resizedColor, depth_.size(), 0, 0, CV_INTER_LINEAR);
    }

    // do Registration
    cv::Mat newDepth;
    Mat depth_mid;
    if (!p3d.empty() && !color.empty()) {
        ASSERT_OK(TYRegisterWorldToColor2(
            pData->hDevice, (TY_VECT_3F *) p3d.data, 0, p3d.cols * p3d.rows,
            color.cols, color.rows, (uint16_t *) buffer, sizeof(buffer)));
        newDepth = cv::Mat(color.rows, color.cols, CV_16U, (uint16_t *) buffer);
        cv::Mat resized_color;
        cv::Mat temp;
        // you may want to use median filter to fill holes in projected depth image
        // or do something else here
        cv::medianBlur(newDepth, temp, 5); //中值滤波
        newDepth = temp;
        // resize to the same size for display
        cv::resize(newDepth, newDepth, depth_.size(), 0, 0, 0);
        cv::resize(color, resized_color, depth_.size());
        cv::Mat depthColor = pData->render->Compute(newDepth);
        depth_mid = depthColor;
        // depthColor = depthColor / 2 + resized_color / 2;
        // cv::imshow("projected depth", depthColor);
    }
/****************************************************************************************/
    if (key_background && w.detec == 0)
        key_background = false;
    if (key_background && w.detec) {
        background_pic = depth;
    } else {
        background_pic = resizedColor;
    }

    w.direction = 0;
    w.key_detect();
    if(!w.detec){
        saveHeight(height_of_basket);
    }
    if (w.detec) {
        if (w.get_height) {
            bool error;
            cout << "进入获取高度函数" << endl;
            error= box_height_grub(p3d); ////抓取箱子的高度////
            if(!error){
                height_of_basket=0;
            }
            cout << "------------------>"
                 << "height_of_basket = " << height_of_basket
                 << endl; /// height_of_basket变量为车厢的最终高度///
            if (height_of_basket < range_high && height_of_basket > range_low) {
                w.get_height = false;                         ////
                new_rang_low = height_of_basket - range_down; ////rang_down=30;
                new_rang_high = height_of_basket + range_up;  ////rang_up=30
            }

            w.height = height_of_basket;
        } ///从新设置一下用来截取高度的范围rang_low   high;/////

        //------------从相机抓取数据----------------------///

        Point3f *cloud_3f = (Point3f *) p3d.data;
        int num = p3d.rows * p3d.cols / ds;

        //车筐点云的获取
        vector<Point3f> box_world;
        vector<Point3f> box_UPworld;
        vector<Point2f> linePoints;
        box_grub(cloud_3f, box_world, box_UPworld, linePoints,
                 num); ///抓取点云图的点，三维放在box_world,二维放在linepoint;////
        //边沿识别以及交点定位算法,可视化显示
        if (linePoints.size() > 50) {
            w.index = 1;
            linepoints_mistime = 0;
            cout << linePoints.size() << endl;
            //俯视图下的车筐
            // Mat box_grub=Mat::zeros(length_y,length_x,CV_8UC3);
            //-----对给定的2D点集，寻找最小包围面积-----////
            Point2f vertex_original[4]; //用于存储不加平移二维的角点值
            Point3f corners[4];         //用于存储三维角点值
            //将找到的角点赋给三维点阵，为坐标转换做准备
            corners_grub(linePoints, vertex_original, corners);

            //角点坐标转换, 从世界坐标到相机坐标再到像素坐标
            vector<Point> corners_pixel;
            //co_transfer(corners, corners_pixel);把角点的左边转换成像素坐标

            //--------------2D图中寻找饲料落点并绘制----------------///
            vector<Point2f> fallPoints_2D;
            vector<Point2f> upfallPoints_2D; // 2D点阵图中不加平移的落点集
            vector<Point2f> downfallPoints_2D;
            //加一个正落点,测试用绘制,也用于最主要的喷洒距离计算
            Point2f fallPoint_right = Point(0, 0); //世界坐标系下的落点,未经平移
            fallPointFind(vertex_original, fallPoints_2D);

            //--------------在depth图中找到饲料落点并绘制---------------///
            //找到落点的Z坐标
            vector<Point3f> fallPoints_world;

            fallPointWorldFind(box_world, fallPoints_2D, fallPoints_world);
            // fallPointWorldFind(box_world, downfallPoints_2D,fallPoints_world);
            /*thread t1(fallPointWorldFind, ref(box_world), ref(upfallPoints_2D),
            ref(fallPoints_world)); thread t2(fallPointWorldFind, ref(box_world),
            ref(downfallPoints_2D), ref(fallPoints_world)); t1.join(); t2.join();*/
            //喷撒规划
            // fall_rule(fallPoints_world);
            //饲料落点坐标转换, 从世界坐标到相机坐标再到像素坐标
            vector<Point> fallPoints_pixel;
            // co_transfer_2(fallPoints_world,fallPoints_pixel);///将落点的坐标转换成像素坐标系///
            thread t3(co_transfer, corners, ref(corners_pixel));
            thread t4(co_transfer_2, ref(fallPoints_world), ref(fallPoints_pixel));
            t3.join();
            t4.join();
            //绘制边框,落点
            drawBox(background_pic, fallPoints_world, fallPoints_pixel,
                    corners_pixel);
            contr1.SensorAngle=get_angle();
            contr1.traversal_control(fallPoints_world, height_of_basket, range_up, w, fall_step); //heng向控制
            //contr1.vertical_control(fallPoints_2D);//zong向控制
            contr1.vertical_control_Vision(fallPoints_2D, box_UPworld, x_axisFallPoint);

        } else {
            w.index = 0;
            if (++linepoints_mistime > 50) {
                // exit_main = 1;
                w.get_height = true;
            }
        }

        int depth_h = depth_.rows;
        int depth_w_left = depth_.cols / 2 - depth_.cols / pipe_range;
        int depth_w_right = depth_.cols / 2 + depth_.cols / pipe_range;

        line(background_pic, Point(depth_w_left, 0),
             Point(depth_w_left, depth_h / 4), Scalar(0, 255, 0), 2,
             CV_AA); ///画左边的饲料下落边界
        line(background_pic, Point(depth_w_right, 0),
             Point(depth_w_right, depth_h / 4), Scalar(0, 255, 0), 2,
             CV_AA); ///画右边的饲料下落边界
    }
    contr1.can_state();
    w.pic = background_pic;
    show(w);


    save_or_not(background_pic, depth_, color, p3d, pData->index);

    int key = cv::waitKey(1);
    switch (key) {
    case -1:break;
    case 'q':
    case 1048576 + 'q':exit_main = true;
        break;
    case 's':
    case 1048576 + 's':save_frame = true;
        break;
    default:LOGD("Pressed key %d", key);
    }

    LOGD("=== Callback: Re-enqueue buffer(%p, %d)", frame->userBuffer,
         frame->bufferSize);
    ASSERT_OK(
        TYEnqueueBuffer(pData->hDevice, frame->userBuffer, frame->bufferSize));
}

int main(int argc, char *argv[]) {
    const char *IP = NULL;
    const char *ID = NULL;
    TY_DEV_HANDLE hDevice;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-id") == 0) {
            ID = argv[++i];
        } else if (strcmp(argv[i], "-ip") == 0) {
            IP = argv[++i];
        } else if (strcmp(argv[i], "-h") == 0) {
            LOGI("Usage: SimpleView_Callback [-h] [-ip <IP>]");
            return 0;
        }
    }

    LOGD("=== Init lib");
    ASSERT_OK(TYInitLib());
    TY_VERSION_INFO *pVer = (TY_VERSION_INFO *) buffer;
    ASSERT_OK(TYLibVersion(pVer));
    LOGD("     - lib version: %d.%d.%d", pVer->major, pVer->minor, pVer->patch);

    if (IP) {
        LOGD("=== Open device %s", IP);
        ASSERT_OK(TYOpenDeviceWithIP(IP, &hDevice));
    } else {
        if (ID == NULL) {
            LOGD("=== Get device info");
            ASSERT_OK(TYGetDeviceNumber(&n));
            LOGD("     - device number %d", n);

            TY_DEVICE_BASE_INFO *pBaseInfo = (TY_DEVICE_BASE_INFO *) buffer;
            ASSERT_OK(TYGetDeviceList(pBaseInfo, 100, &n));

            if (n == 0) {
                LOGD("=== No device got");
                return -1;
            }
            ID = pBaseInfo[0].id;
        }

        LOGD("=== Open device: %s", ID);
        ASSERT_OK(TYOpenDevice(ID, &hDevice));
    }

    int32_t allComps;
    ASSERT_OK(TYGetComponentIDs(hDevice, &allComps));
    if (!(allComps & TY_COMPONENT_RGB_CAM)) {
        LOGE("=== Has no RGB camera, cant do registration");
        return -1;
    }

    LOGD("=== Configure components");
    int32_t componentIDs = TY_COMPONENT_POINT3D_CAM | TY_COMPONENT_RGB_CAM;
    ASSERT_OK(TYEnableComponents(hDevice, componentIDs));

    LOGD("=== Prepare image buffer");
    int32_t frameSize;

    // frameSize = 1280 * 960 * (3 + 2 + 2);
    ASSERT_OK(TYGetFrameBufferSize(hDevice, &frameSize));
    LOGD("     - Get size of framebuffer, %d", frameSize);
    LOGD("     - Allocate & enqueue buffers");
    char *frameBuffer[2];
    frameBuffer[0] = new char[frameSize];
    frameBuffer[1] = new char[frameSize];
    LOGD("     - Enqueue buffer (%p, %d)", frameBuffer[0], frameSize);
    ASSERT_OK(TYEnqueueBuffer(hDevice, frameBuffer[0], frameSize));
    LOGD("     - Enqueue buffer (%p, %d)", frameBuffer[1], frameSize);
    ASSERT_OK(TYEnqueueBuffer(hDevice, frameBuffer[1], frameSize));

    LOGD("=== Register callback"); ///寄存器回调
    LOGD("Note: Callback may block internal data receiving,");
    LOGD("so that user should not do long time work in callback.");
    LOGD("To avoid copying data, we pop the framebuffer from buffer queue and");
    LOGD("give it back to user, user should call TYEnqueueBuffer to re-enqueue it.");
    DepthRender render;
    CallbackData cb_data;
    cb_data.index = 0;
    cb_data.hDevice = hDevice;
    cb_data.render = &render;
    // ASSERT_OK( TYRegisterCallback(hDevice, frameCallback, &cb_data) );

    LOGD("=== Register event callback");
    LOGD("Note: Callback may block internal data receiving,");
    LOGD("      so that user should not do long time work in callback.");
    // ASSERT_OK(TYRegisterEventCallback(hDevice, eventCallback, NULL));

    LOGD("=== Disable trigger mode"); ///禁用触发模式
    ASSERT_OK(
        TYSetBool(hDevice, TY_COMPONENT_DEVICE, TY_BOOL_TRIGGER_MODE, false));

    LOGD("=== Start capture"); ///开始捕获
    ASSERT_OK(TYStartCapture(hDevice));

    LOGD("=== Read color rectify matrix"); ///读取彩色矫正矩阵
    {
        TY_CAMERA_DISTORTION color_dist;
        TY_CAMERA_INTRINSIC color_intri;
        TY_STATUS ret =
            TYGetStruct(hDevice, TY_COMPONENT_RGB_CAM, TY_STRUCT_CAM_DISTORTION,
                        &color_dist, sizeof(color_dist));
        ret |= TYGetStruct(hDevice, TY_COMPONENT_RGB_CAM, TY_STRUCT_CAM_INTRINSIC,
                           &color_intri, sizeof(color_intri));
        if (ret == TY_STATUS_OK) {
            cb_data.color_intri = color_intri;
            cb_data.color_dist = color_dist;
        } else { // reading data from device failed .set some default values....
            memset(cb_data.color_dist.data, 0, 12 * sizeof(float));
            memset(cb_data.color_intri.data, 0, 9 * sizeof(float));
            cb_data.color_intri.data[0] = 1000.f;
            cb_data.color_intri.data[4] = 1000.f;
            cb_data.color_intri.data[2] = 600.f;
            cb_data.color_intri.data[5] = 450.f;
        }
    }

    LOGD("=== Wait for callback");
    exit_main = false;
    gpio_init();
    can_init();
    get_angle_init();
    // pre work
    param();     ////从文件中读取各种参数////
    getRt();     ///从RT文件中读取RT矩阵////
    quantized(); ////把folat型转化为int型，提高算法的速度////
    cout << "llsu1" << endl;

    control contr1;

    thread tc(while_control, ref(contr1)); ///单独踢出一个线程用来做CAN接受
    tc.detach();

    thread tt(while_key);
    tt.detach(); ///单独踢出一个线程用来做按键检测

    QApplication a(argc, argv);
    MainWindow w; /// QT界面开发

    while (!exit_main) {
        clock_t while_start = clock();
        TY_FRAME_DATA frame;
        cout << "llsu2" << endl;
        int err = TYFetchFrame(hDevice, &frame, -1);
        if (err != TY_STATUS_OK) {
            LOGE("Fetch frame error %d: %s", err, TYErrorString(err));
            break;
        } else {
            clock_t get_frame_start = clock();
            handleFrame(&frame, &cb_data, w, contr1);
            clock_t get_frame_end = clock();

            cout << "get_frame_time=" << (get_frame_end - get_frame_start) / 1000
                 << "ms" << endl;
        }
        clock_t while_end = clock();
        cout << "while_time=" << (while_end - while_start) / 1000 << "ms" << endl;
    }
    key_ = false;
    can_closed();
    ASSERT_OK(TYStopCapture(hDevice));
    ASSERT_OK(TYCloseDevice(hDevice));
    ASSERT_OK(TYDeinitLib());
    delete frameBuffer[0];
    delete frameBuffer[1];

    LOGD("=== Main done!");

    return 0;
}
