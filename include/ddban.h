//
// Created by doudouban on 18-10-4.
//

#ifndef BOX_RUNTIME_VERTION_DDBAN_H
#define BOX_RUNTIME_VERTION_DDBAN_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <cmath>
#include <math.h>
#include <algorithm>
#include <array>
#include <fstream>
#include "can_jetson.hpp"
#include <mainwindow.h>

using namespace cv;
using namespace std;

class control {
public:
  double car_up;
  float angle_reslut;
  double SensorAngle =0;

  void vertical_control(const vector<Point2f> &fallPoints_2D);

  void traversal_control(vector<Point3f> &fallpoint_world, int height_of_basket, const int& symbol_step,
                         MainWindow &w, const int& fall_step);

  void receive_control();
  void vertical_control_Vision(const vector<Point2f>& fallPoints_2D, const vector<Point3f>& box_UPworld, const int& x_axisFallPoint );

  void can_state();

protected:

  const float g1 = 9.8;
  const float Pi = 3.1415926;
  const float arm_r = 0.7;
  const float v = 1.0;
  const double angle_a = 40;///dangbanyuzhixiande jiajiao
  const double angle_b = 30;///shuipingmian yu zhixian de jiajiao
  long long CAN_id;
  const int CAN_dlc = 8;
  int count_r =0 ;
  int count_l =0 ;

  array<int, 8> CAN_data = {{0, 0, 0, 0, 0, 0, 0, 0}};
  array<int, 8> receive_data = {{0, 0, 0, 0, 0, 0, 0, 0}};

  int receive_id;
};

#endif //BOX_RUNTIME_VERTION_DDBAN_H
