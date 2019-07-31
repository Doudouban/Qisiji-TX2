#ifndef LLSU_H
#define LLSU_H
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <cv.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <math.h>
#include <algorithm>
#include <PointCloudViewer.hpp>
#include <fstream>
#include "../include/mainwindow.h"



using namespace cv;
using namespace std;

//图漾新相机Depth内参,目前最准确的内参
const Mat intrinsic = ( Mat_<float> (3,3)<<
        581.836487, 0.000000, 328.943329,
        0.000000, 581.836487, 262.558716,
        0.0,        0.0,        1.0 );

//新相机内参
//1124.182129 0.000000 668.400024
//0.000000 1123.782227 470.168945
//0.000000 0.000000 1.000000


//R T矩阵的获取
void getRt();

//加速算法
extern int fix_fl;
void quantized();

//获取参数
void param();
//车筐高度获取
bool box_height_grub(const Mat& p3d);

//坐标转换 世界坐标到像素坐标
void co_transfer(const Point3f* corners,vector<Point>& corners_pixel);

void co_transfer_2(vector<Point3f>& corners,vector<Point>& corners_pixel);

//车筐点云获取
void box_grub(const Point3f* cloud_3f, vector<Point3f>& box_world, vector<Point2f>& linePoints, int num);

//角点获取函数
void corners_grub(const vector<Point2f>& linePoints, Point2f* vertex_original, Point3f* corners);

//落点寻找函数
void fallPointFind(Point2f* vertex_original, vector<Point2f>& upfallPoints_2D,vector<Point2f>& downfallPoints_2D, vector<Point2f>& midPoint );
void fallPointFind(Point2f* vertex_original, vector<Point2f>& fallPoints_2D);

//求取落点的高度
void fallPointWorldFind(const vector<Point3f>& box_world, const vector<Point2f>& fallPoints_2D, vector<Point3f>& fallPoints_world);

//绘制边框落点
void drawBox(Mat& depth,const vector<Point3f>& fallPoints_world, const vector<Point>& fallPoints_pixel,  const vector<Point>& corners_pixel );


//是否保存图像
void save_or_not(const Mat& depth_results, const Mat& depth_, const Mat& color, const Mat& p3d , const int frame_num);

void show(MainWindow &w);


#endif





