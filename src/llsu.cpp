#include "llsu.hpp"
#include <mutex>
//----------------参数设置--------------------////
//坐标转换相关参数
Mat R_float = (Mat_<float> ( 3,3 ) <<
                                   0.99966508, 0.010076982, -0.023836536,
                                    0.022556571, -0.79078507, 0.61167818,
                                    -0.012685708, -0.61201096, -0.79074752);
Mat t_float = (Mat_<float> ( 3,1 ) <<
                                   -9.2993746,
                                    359.92139,
                                    923.52026 );

Mat R = (Mat_<int> ( 3,3 ) <<
                           0, 0, 0,
                            0, 0, 0,
                            0, 0, 0);
Mat t = (Mat_<int> ( 3,1 ) <<
                           0,
                            0,
                            0);
Mat R_inv = R_float.t();
Mat t_inv=-R_float.t() * t_float;


int height_of_basket; //车筐的高度
int range_low ;
int range_high ;//作为高度是否正确的判断条件,同时作为车筐边沿提取的上下边界值
int new_rang_low;
int new_rang_high;
int range_down ;
int range_up ;// 作为车斗上下高度截取的范围
int ds;

int fall_step ;//落点之间的间距
int thick_of_box;//箱子的边缘厚度
int z_range_x;
int z_range_y;//求取落点高度平均值的范围
int plu; //二次寻找落点平均高度时拓宽的范围倍数
int x_axisFallPoint;//根据满溢程度标记落点

//int window_size;//窗口大小
int pipe_range ; //绘制喷洒的范围离中心列的距离 cols/pipe_range
//int first_frame=8;//起始帧 debug param
//int last_frame=99;//尾帧数 debug param

int results_save;
int depth_save ;
int color_save ;
int cloud_save ;

float angle_E;
mutex mutex_;

bool box_height_grub(const Mat& p3d)
{
    clock_t box_height_start = clock();
    int depth_h = p3d.rows;
    int depth_w_left = p3d.cols / 2 - p3d.cols / pipe_range; //范围可以做一些调整  把列分为16份
    int depth_w_right = p3d.cols / 2 + p3d.cols / pipe_range;//寻找合适的
    int x,y,z;
    int z_tem;
    float box_world_z;
    vector<int> box_height;
    for (int i = depth_w_left; i < depth_w_right; i++)
    {
        for(int j = p3d.rows/pipe_range; j < depth_h; j++)
        {

            if(!isnan(p3d.at<Vec3f>(j,i)[2]))
            {
                x = int(p3d.at<Vec3f>(j,i)[0]) ;
                y = int(p3d.at<Vec3f>(j,i)[1]);
                z = int(p3d.at<Vec3f>(j,i)[2]);
                z_tem = x * R.at<int>(2,0) + y * R.at<int>(2,1) + z * R.at<int>(2,2);
                box_world_z = z_tem * pow(2, (-1) * fix_fl) + t_float.at<float>(2,0);              ///世界坐标系X=Rx+t;////
                box_height.push_back((int)box_world_z);
            }
        }
    }

            cout <<"box_height = "<<box_height.size() << endl;

    if(box_height.size()==0)
        return false;
    else {
    sort(box_height.begin(), box_height.end(),greater<int>());            ///把高度信息进行从小到大排序///
    int final_height = 0;
    for(int k = 5; k < 15 ; k++)
    {
        final_height = final_height + box_height[k];
    }


    final_height = final_height / 10;               ///取其中10个值的平均为车高///
    height_of_basket = final_height;  //重置车斗 高度
    clock_t box_height_end = clock();

//    for(int k = 1; k < 20; k++)
//    {
//        cout << "num " << k << " = "<< box_height[k] << endl;
//    }

    cout << "LLSU === box_height_grub " << endl;
    //cout << "box_height_grub.size() = " << box_height.size() << endl;
    cout << "fianl_height = " <<final_height << endl;
    //cout << "box_height_grub_time = " << (box_height_end - box_height_start) / 1000 << " ms" << endl;
    cout << endl;
    return true;
    }
}

//坐标转换 世界坐标到像素坐标
void co_transfer(const Point3f* corners,vector<Point>& corners_pixel)
{
    for(int i=0; i< 4 ;i++)
    {
        Mat corner_camera = R_inv * (Mat_<float>(3,1)<<corners[i].x, corners[i].y, corners[i].z) +t_inv ;//world plane to camera plane
        float z_inv=1.0/corner_camera.at<float>(2,0);
        Mat corner_pixel =  z_inv*intrinsic*corner_camera;//camera plane to pixel plane

        //corner_pixel.at<float>(0,0) /=2;
        //corner_pixel.at<float>(1,0) /=2;
        //corner_pixel.at<float>(0,0) +=10;
        corners_pixel.push_back(Point(corner_pixel.at<float>(0,0),corner_pixel.at<float>(1,0)));
    }
}

void co_transfer_2(vector<Point3f>& corners,vector<Point>& corners_pixel)
{
    for(int i=0; i<corners.size() ;i++)
    {
        Mat corner_camera = R_inv * (Mat_<float>(3,1)<<corners[i].x, corners[i].y, corners[i].z) +t_inv ;//world plane to camera plane
        float z_inv=1.0/corner_camera.at<float>(2,0);
        Mat corner_pixel =  z_inv*intrinsic*corner_camera;//camera plane to pixel plane

        //corner_pixel.at<float>(0,0) /=2;
        //corner_pixel.at<float>(1,0) /=2;
        corners_pixel.push_back(Point(corner_pixel.at<float>(0,0),corner_pixel.at<float>(1,0)));
    }
}

//车筐点云获取
void box_grub(const Point3f* cloud_3f, vector<Point3f>& box_world, vector<Point3f>& box_UPworld, vector<Point2f>& linePoints, int num)
{
    clock_t box_grub_start = clock();
    int flag = 0;
    int x,y,z;
    int p_world_x, p_world_y, p_world_z;
    int low = (new_rang_low - t.at<int>(2, 0)) << fix_fl;
    int high = (new_rang_high - t.at<int>(2, 0)) << fix_fl;/////由于RT矩阵都已转化成整型，
    float box_world_x, box_world_y, box_world_z;
    if(num){
        for(int i = 0; i < num; i = i + ds ){
            if(!isnan(cloud_3f[i].x))
            {
                flag++;
                x = (int)(cloud_3f[i].x);
                y = (int)(cloud_3f[i].y);
                z = (int)(cloud_3f[i].z);
                int z_tem = x * R.at<int>(2,0) + y * R.at<int>(2,1) + z * R.at<int>(2,2);
                if(z_tem>=low){
                    p_world_x = x * R.at<int>(0,0) + y * R.at<int>(0,1) + z * R.at<int>(0,2);
                    p_world_y = x * R.at<int>(1,0) + y * R.at<int>(1,1) + z * R.at<int>(1,2);
                    p_world_z = z_tem;
                    box_world_x	= p_world_x * pow(2, (-1) * fix_fl) + t_float.at<float>(0,0);
                    box_world_y = p_world_y * pow(2, (-1) * fix_fl) + t_float.at<float>(1,0);
                    box_world_z = p_world_z * pow(2, (-1) * fix_fl) + t_float.at<float>(2,0);  ///重新转换成原浮点型，得实际世界坐标
                    if(z_tem <= high)//根据高度信息来得到车框
                    {
                        box_world.push_back(Point3f(box_world_x,box_world_y,box_world_z));
                        linePoints.push_back(Point2f(box_world_x,box_world_y));
                    }
                    else if(z_tem <=high+range_up/2) {
                        box_UPworld.push_back(Point3f(box_world_x,box_world_y,box_world_z));
                    }

                }

            }
        }
    }
    clock_t box_grub_end = clock();
    cout << "LLSU === box_grub" << endl;
    cout << "linePoints.size() = " << linePoints.size() <<endl;
    cout << "box_grub_time = " << (box_grub_end - box_grub_start)/1000 << " ms " << endl;
    cout << "num = " << num << " flag = " << flag << endl;
    cout << endl;
}

//角点获取函数
void corners_grub(const vector<Point2f>& linePoints, Point2f* vertex_original, Point3f* corners)
{
    cout << "LLSU === corners_grub" << endl;
    clock_t corners_grub_start = clock();
    RotatedRect box = minAreaRect(linePoints);    ////寻找闭合的矩形圈
    box.points(vertex_original);
    for(int j = 0; j < 4; j++ )
    {
        corners[j].x = vertex_original[j].x;
        corners[j].y = vertex_original[j].y;
        corners[j].z = height_of_basket;   ///为什么不能给这个点的实际高度///即像素点算出来的实际世界坐标
    }
    clock_t corners_grub_end = clock();
    cout << "corners_grub_time = " << (corners_grub_end - corners_grub_start)/1000 << " ms " << endl;
    cout << endl;
}

//落点寻找函数

void fallPointFind(Point2f* vertex_original, vector<Point2f>& fallPoints_2D )
{
    cout << "LLSU === fallPointFind " << endl;
    clock_t fallPFind_start=clock();
    for(int i = 0; i < 4; i++)
    {
        for (int j =i+1; j < 4; j++ )
        {
            if (vertex_original[i].x > vertex_original[j].x)
            {	Point tem = vertex_original[i];
                vertex_original[i] = vertex_original[j];
                vertex_original[j] = tem;
            }
        }
    }

    for(int i=0;i<4;i++)
    {
        cout<<vertex_original[i].x<<" "<<vertex_original[i].y<<endl;
    }
    Point midPoint_left = Point((vertex_original[0].x+vertex_original[1].x)/2 , (vertex_original[0].y+vertex_original[1].y)/2) ;
    Point midPoint_right = Point((vertex_original[2].x+vertex_original[3].x)/2 , (vertex_original[2].y+vertex_original[3].y)/2) ;

    int fallPoint_num = 10;
    int fall_start = int(2*300*(-1));//左2,右3情况
    for (int j = 0; j < fallPoint_num; j++)
    {
        //假设机械臂所在的直线投影是x = 0;
        float x_fall = fall_start+fall_step*j;
        if( x_fall > (midPoint_left.x +thick_of_box) && x_fall < (midPoint_right.x -thick_of_box))
        {
            Point fallPoint = Point (x_fall, 0);
            fallPoint.y = (x_fall - midPoint_left.x)*(midPoint_right.y - midPoint_left.y) / (midPoint_right.x - midPoint_left.x) + midPoint_left.y;     ///求出中间横线的方程，用两点式求落点y轴坐标//
            if((fallPoint.y>vertex_original[0].y && fallPoint.y<vertex_original[1].y)||(fallPoint.y<vertex_original[0].y && fallPoint.y>vertex_original[1].y))
            fallPoints_2D.push_back(Point(fallPoint.x , fallPoint.y));
        }
    }
    clock_t fallPFind_end=clock();
    cout<<"fallPointFind_time="<<(fallPFind_end-fallPFind_start)/1000<<"ms"<<endl;
    cout << endl;
}

void fallPointFind(Point2f* vertex_original, vector<Point2f>& upfallPoints_2D,vector<Point2f>& downfallPoints_2D, vector<Point2f>& midPoint )
{
    cout << "LLSU === fallPointFind " << endl;
    for(int i = 0; i < 4; i++)
    {
        for (int j =i+1; j < 4; j++ )
        {
            if (vertex_original[i].x > vertex_original[j].x)
            {	Point tem = vertex_original[i];
                vertex_original[i] = vertex_original[j];
                vertex_original[j] = tem;
            }
            else if (vertex_original[i].x == vertex_original[j].x)
            {
                if(vertex_original[i].y > vertex_original[j].y)
                {
                    Point tem = vertex_original[i];
                    vertex_original[i] = vertex_original[j];
                    vertex_original[j] = tem;
                }
            }
        }
    }
    Point midPoint_1 = Point((vertex_original[0].x+vertex_original[1].x)/2 , (vertex_original[0].y+vertex_original[1].y)/2) ;
    Point midPoint_2 = Point((vertex_original[2].x+vertex_original[3].x)/2 , (vertex_original[2].y+vertex_original[3].y)/2) ;
    midPoint.push_back(midPoint_1);
    midPoint.push_back(midPoint_2);

    Point uppoint_1 = Point((midPoint_1.x+vertex_original[0].x)/2 , (midPoint_1.y+vertex_original[0].y)/2);
    Point uppoint_2 = Point((midPoint_2.x+vertex_original[2].x)/2 , (midPoint_2.y+vertex_original[2].y)/2);

    ///车筐中线下方的两个点
    Point downpoint_1 = Point((midPoint_1.x+vertex_original[1].x)/2 , (midPoint_1.y+vertex_original[1].y)/2);
    Point downpoint_2 = Point((midPoint_2.x+vertex_original[3].x)/2 , (midPoint_2.y+vertex_original[3].y)/2);

    int fallPoint_num = 5;
    int fall_start = 0;//int(1.5*fall_step*(-1));//左2,右3情况
    for (int j = -2; j < fallPoint_num; j++)
    {
        //假设机械臂所在的直线投影是x = 0;
        float x_fall = fall_start+fall_step*j;
        if( x_fall > (uppoint_1.x +thick_of_box) && x_fall < (uppoint_2.x -thick_of_box))
        {
            Point fallPoint = Point (x_fall, 0);
            fallPoint.y = (x_fall - uppoint_1.x)*(uppoint_2.y - uppoint_1.y) / (uppoint_2.x - uppoint_1.x) + uppoint_1.y;     ///求出中间横线的方程，用两点式求落点y轴坐标//
            //if(fallPoint.y > vertex_original[0].y && fallPoint.y < midPoint_1.y)
            upfallPoints_2D.push_back(Point(fallPoint.x , fallPoint.y));
        }
        float x_fall_1 = fall_start+fall_step*j;
        if( x_fall_1 > (downpoint_1.x +thick_of_box) && x_fall_1 < (downpoint_2.x -thick_of_box))
        {
            Point fallPoint = Point (x_fall_1, 0);
            fallPoint.y = (x_fall_1 - downpoint_1.x)*(downpoint_2.y - downpoint_1.y) / (downpoint_2.x - downpoint_1.x) + downpoint_1.y;     ///求出中间横线的方程，用两点式求落点y轴坐标//
            //if(fallPoint.y > midPoint_1.y && fallPoint.y < vertex_original[1].y)
            downfallPoints_2D.push_back(Point(fallPoint.x , fallPoint.y));
        }
    }
    //	fallPoint_right.y = (0 - midPoint_1.x)*(midPoint_2.y - midPoint_1.y) / (midPoint_2.x - midPoint_1.x) + midPoint_1.y;
    //	cout << "fallPoint_right = " << "[ " << fallPoint_right.x << " , "  << fallPoint_right.y << " ]" << endl;
    cout << endl;
}
//求取落点的高度
void fallPointWorldFind(const vector<Point3f>& box_world, vector<Point2f>& fallPoints_2D, vector<Point3f>& fallPoints_world) {
    mutex_.lock();
    cout<<"LLSU===fallPointWroldFind"<<endl;
    clock_t fallPFWorld_start=clock();
    for (int j = 0; j < fallPoints_2D.size(); j++)
    {
        if(fallPoints_2D[j].x == 0)
            fallPoints_2D[j].x +=  x_axisFallPoint;
        vector <Point3f> box_world_choose;
        for (int i = 0; i < box_world.size(); i += 2)
        {
            if ((fallPoints_2D[j].x - box_world[i].x) < z_range_x &&
                (fallPoints_2D[j].x - box_world[i].x) > (-1) * z_range_x &&
                (fallPoints_2D[j].y - box_world[i].y) < z_range_y &&
                (fallPoints_2D[j].y - box_world[i].y) > (-1) * z_range_y) {
                box_world_choose.push_back(Point3f(box_world[i].x, box_world[i].y, box_world[i].z));///在i落点出找一片区域，存在box_world_choose
            }
        }
        //落点范围内不一定有点云数据,实际场景需要分类讨论,采取一些查找措施.1.将搜索区域变成以y轴为长边的矩形, 2.分级搜索
        if (box_world_choose.size())
        {
            float z_sum = 0.0;
            for (int k = 0; k < box_world_choose.size(); k++)
            {
                z_sum = z_sum + box_world_choose[k].z;
            }
            fallPoints_world.push_back(Point3f(fallPoints_2D[j].x, fallPoints_2D[j].y, z_sum / box_world_choose.size()));////去这一片区域的平均高度为落点的高度///
        } else
        {///如果小范围内没有值就扩大范围
            for (int i = 0; i < box_world.size(); i = i + 2)
            {
                if ((fallPoints_2D[j].x - box_world[i].x) < z_range_x * plu &&
                    (fallPoints_2D[j].x - box_world[i].x) > (-1) * z_range_x * plu &&
                    (fallPoints_2D[j].y - box_world[i].y) < z_range_y * plu &&
                    (fallPoints_2D[j].y - box_world[i].y) > (-1) * z_range_y * plu)
                {
                    box_world_choose.push_back(Point3f(box_world[i].x, box_world[i].y, box_world[i].z));
                }
            }
            if(box_world_choose.size())
            {
                float z_sum = 0.0;
                for (int k = 0; k < box_world_choose.size(); k++)
                {
                    z_sum =  z_sum + box_world_choose[k].z;
                }
                fallPoints_world.push_back(Point3f(fallPoints_2D[j].x, fallPoints_2D[j].y, z_sum/box_world_choose.size()));
            }
            else
            {
                fallPoints_world.push_back(Point3f(fallPoints_2D[j].x, fallPoints_2D[j].y, height_of_basket));///扩大范围后还是没有，就把车厢高度给落点///
            }
        }
    }
    clock_t fallPFWorld_end=clock();
    cout<<"fallPointWorldFind="<<(fallPFWorld_end-fallPFWorld_start)/1000<<"ms"<<endl;
    cout << endl;
    mutex_.unlock();
}

//绘制边框落点
void drawBox(Mat& depth,const vector<Point3f>& fallPoints_world, const vector<Point>& fallPoints_pixel,  const vector<Point>& corners_pixel )
{
    cout<<"LLSU===drawBox"<<endl;
    clock_t drawBox_start=clock();
    //绘制落点
   for (int i = 0; i < fallPoints_world.size(); i++) {
        if (fallPoints_world[i].z <= (height_of_basket)) {
            circle(depth, fallPoints_pixel[i], 2, cv::Scalar(0, 255, 0), 2);
        } else if (fallPoints_world[i].z <= (height_of_basket+range_up/2)) {
            circle(depth, fallPoints_pixel[i], 2, cv::Scalar(0, 255, 255), 2);
        } else {
            circle(depth, fallPoints_pixel[i], 2, cv::Scalar(0, 0, 255), 2);
        }

        //绘制边框
        for (int i = 0; i < corners_pixel.size(); i++) {
            circle(depth, corners_pixel[i], 5, cv::Scalar(0, 0, 255), 5);
            line(depth, corners_pixel[i], corners_pixel[(i + 1) % 4], Scalar(0, 255, 0), 3);
        }
    }
    clock_t drawBox_end=clock();
    cout<<"drawBox="<<(drawBox_end-drawBox_start)/1000<<"ms"<<endl;
    cout<<endl;
}

//是否保存图像
void save_or_not(const Mat& depth_results, const Mat& depth_, const Mat& color, const Mat& p3d , const int frame_num)
{
    if(results_save)
    {
        char path_depth[32];
        sprintf(path_depth, "../result/%d.png", frame_num);
        imwrite(path_depth, depth_results);
    }
    if(depth_save)
    {
        char path_depth_[32];
        sprintf(path_depth_, "../data/depth/%d.png", frame_num);
        imwrite(path_depth_, depth_);
    }
    if(color_save)
    {
        char path_color[32];
        sprintf(path_color, "../data/color/%d.png", frame_num);
        imwrite(path_color, color);
    }
    if(cloud_save)
    {
        char path_cloud[32];
        sprintf(path_cloud, "../data/cloud/%d.txt", frame_num);
        writePointCloud((Point3f*)p3d.data, p3d.total(), path_cloud, PC_FILE_FORMAT_XYZ);
    }
}


void getRt()
{
    double r[9];
    double t[3];
    char filename[20];
    sprintf(filename,"../Rt.txt");
    FILE *fp_1;
    fp_1 = fopen(filename,"r");
    if(9 == fscanf(fp_1,"%lf %lf %lf %lf %lf %lf %lf %lf %lf ",&r[0],&r[1],&r[2],&r[3],&r[4],&r[5],&r[6],&r[7],&r[8]))
    {
        for(int i = 0; i < 9; i++)
        {
            R_float.at<float>(i/3, i%3) = (float)r[i];
        }
    }
    if(3 == fscanf(fp_1,"%lf %lf %lf ",&t[0],&t[1],&t[2]))
    {
        t_float.at<float>(0,0) = (float)t[0];
        t_float.at<float>(1,0) = (float)t[1];
        t_float.at<float>(2,0) = (float)t[2];
    }
    fclose(fp_1);

    R_inv = R_float.t();////求R的转置矩阵///   R'X+R't=x;  X  世界坐标系   x  像素坐标系
    t_inv=-R_float.t() * t_float;
    cout << endl;

}
int fix_fl = 16;
void quantized()
{
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            R.at<int>(i,j) = round(R_float.at<float>(i,j) * pow(2, fix_fl));////把R矩阵向左移16位取整
        }
        t.at<int>(i,0) = round(t_float.at<float>(i,0));
    }
    cout<<"R ="<<endl<<R<<endl;
    cout<<"t ="<<endl<<t<<endl;
}


void param()
{
    clock_t param_start=clock();
    ifstream filename;
    filename.open("../param.txt");
    cout<<"LLSU === " << " param " << endl;
    //filename>>height_of_basket;
    //cout << "height_of_basket ---------------->" << height_of_basket << endl;
    filename>>range_low;
    filename>>range_high;
    cout << "range_low------>" << range_low << "   range_high------->" << range_high << endl;//700,1000
    filename>>range_down;
    filename>>range_up;
    cout << "range_down------>" << range_down << "   range_up------->" << range_up << endl;//30,30
    filename>>ds;
    cout << "ds ----->" << ds << endl;//1
    filename>>fall_step;
    cout << "fall_step -----> " << fall_step << endl;//100
    filename>>thick_of_box;
    cout << "thick_of_box----->" << thick_of_box << endl;//20
    filename>>z_range_x;
    filename>>z_range_y;
    cout << "z_range----->" << z_range_x <<" ,  " << z_range_y  << endl;//20,50
    filename>>plu;
    cout << "plu----->" << plu << endl;//2
    filename>>x_axisFallPoint;
    cout << "x_axisFallPoint----->" << x_axisFallPoint << endl;//50
//    filename>>window_size;
//    cout << "window_size -----> " << window_size << endl;//400
    filename>>pipe_range;
    cout << "pipe_range -----> " << pipe_range << endl;//16
    filename>>results_save;
    filename>>depth_save;
    filename>>color_save;
    filename>>cloud_save;
    cout << "results_save -----> " << results_save << endl;
    cout << "depth_save -----> " << depth_save << endl;
    cout << "color_save -----> " << color_save << endl;
    cout << "cloud_save -----> " << cloud_save << endl;
    filename.close();
    cout << endl;
    clock_t param_end=clock();
    cout<<"param_time="<<(param_end-param_start)/1000<<"ms"<<endl;
    cout<<endl;
}

void show(MainWindow &w)
{
    w.showFrame();
    w.show();
}

void saveHeight(int& height)
{
    ofstream fout;
    fout.open("../high.txt");
    fout << height << endl;

}
