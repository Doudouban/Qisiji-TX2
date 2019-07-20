//
// Created by doudouban on 18-10-4.
//

#include "ddban.h"

extern bool key_detection;

void control::traversal_control(vector<Point3f>& fallpoint_world,const int height_of_basket, const int symbol_step, MainWindow &w) {
    CAN_id=0x18efff4f;
    CAN_data[0]=0;//保留
    CAN_data[1]=0;//保留
    CAN_data[5]=0;//保留
    CAN_data[7]=0x10;
    for (int i = 0; i < fallpoint_world.size(); i++) {
        for (int j = fallpoint_world.size() - 1; j > i; j--) {
            if (fallpoint_world[i].x < fallpoint_world[j].x) {
                Point3f point = fallpoint_world[i];
                fallpoint_world[i] = fallpoint_world[j];
                fallpoint_world[j] = point;
            }
        }
    }
    for(int i=0;i<fallpoint_world.size();i++){
     cout<<fallpoint_world[i]<<endl;
}

    bool left_= false;
    bool right_= false;
    for (int i = 0; i <fallpoint_world.size(); ++i) {
        if (fallpoint_world[i].z > height_of_basket+symbol_step) {
            i++;
            if(i>fallpoint_world.size())
                break;
        }
        if (fallpoint_world[i].z < height_of_basket+symbol_step) {
            cout<<"i="<<i<<endl;
            if ((fallpoint_world[i].x <= 50) && (fallpoint_world[i].x >= -50)) {
                right_ = false;
                CAN_data[2]=0;//转动速度
                CAN_data[3]=0;//a[5],[6]表示转动角度
                CAN_data[4]=0;
                CAN_data[6]=0;//转动方向：右：0x50,左：0x05
                left_ = false;
                w.direction = 0;
                cout<<"000000000000"<<endl;
                break;
            }
            else if(fallpoint_world[i].x > 50) {



                if (fallpoint_world[i].x < 80) {
                    left_ = true;
                    right_ = false;///转动速度为20,转动角度10,右转
                    CAN_data[2]=20;
                    CAN_data[3]=10;
                    CAN_data[4]=0;
                    CAN_data[6]=0x50;
                    cout<<"1111111111"<<endl;
                    w.direction = 1;
                    break;

                }
                else if(fallpoint_world[i].x<120){
                    left_ = true;
                    right_ = false;///转动速度为30,转动角度20,右转
                    CAN_data[2]=30;
                    CAN_data[3]=20;
                    CAN_data[4]=0;
                    CAN_data[6]=0x50;
                    w.direction = 1;
                    cout<<"222222222"<<endl;
                    break;

                } else{
                    left_ = true;
                    right_ = false;///转动速度为40,转动角度40,右转
                    CAN_data[2]=40;
                    CAN_data[3]=40;
                    CAN_data[4]=0;
                    CAN_data[5]=0x50;
                    w.direction = 1;
                    cout<<"333333333"<<endl;
                    break;

                }

            }
            else if (fallpoint_world[i].x < -50) {



                if(fallpoint_world[i].x>-80){
                    left_ = false;
                    right_ =true;
                    CAN_data[2]=20;
                    CAN_data[3]=10;
                    CAN_data[4]=0;
                    CAN_data[6]=0x05;
                    w.direction = 2;
                    cout<<"44444444444"<<endl;
                    break;
                }
                else if(fallpoint_world[i].x>-240){
                    left_ = false;
                    right_ =true;
                    CAN_data[2]=30;
                    CAN_data[3]=30;
                    CAN_data[4]=0;
                    CAN_data[6]=0x05;
                    w.direction = 2;
                    cout<<"5555555555"<<endl;
                    break;
                }
                else{
                    left_ = false;
                    right_ =true;
                    CAN_data[2]=40;
                    CAN_data[3]-=40;
                    CAN_data[4]=0;
                    CAN_data[6]=0x05;
                    w.direction = 2;
                    cout<<"66666666666"<<endl;
                    break;
                }

            }
            cout << "right=" << right_ << endl;
            cout << "left=" << left_ << endl;
            break;
        }

    }
    can_send(CAN_id,CAN_dlc,CAN_data);
}
void control::vertical_control(vector<Point2f>& fallPoints_2D )  //喷头角度与落点的关系
{

/*    CAN_id=0x18eeff4f;///挡料板控制ID
    CAN_data[0]=0;//保留
    CAN_data[1]=0;//保留
    CAN_data[4]=0;//保留
    CAN_data[7]=0x10;*/
    double a,b,c,temp,mid;
    double s,h;
    //double trailer_h;
    double right_fallpoint_d;
    double angle_c; ///结算出挡板与水平线的夹角
    //trailer_h=height_of_basket; //车筐的高度
    right_fallpoint_d=(fallPoints_2D[1].y+fallPoints_2D[0].y);

    s=right_fallpoint_d-arm_r*cos(angle_b*Pi/180);
    h=arm_r*sin(angle_b*Pi/180);
    a=(g1*s*s)/(2*v*v);///a>0
    b=s;///b>0
    c=(g1*s*s)/(2*v*v)-h;///c>0   开口向上，对称轴在负半轴，
    temp=b*b-4*a*c;
    double angle_1,angle_2;
    if(temp>0)
    {
        mid=(-b+sqrt(temp))/(2*a);
        angle_1=atan(mid);
        mid=(-b-sqrt(temp))/(2*a);
        angle_2=atan(mid);
        if(angle_1>angle_2)
            angle_c=angle_1;
        else
            angle_c=angle_2;
    }
    if(temp<=0)
    {
        angle_c=0;
    }
    angle_1=angle_c+angle_b-angle_a;
/*    CAN_data[2]=50;
    CAN_data[3]=int(angle_1);*/

}

void control::receive_control(){
    can_receive(receive_id,receive_data);
    if(receive_id==0x98ec1727){
        car_up=(receive_data[0]+receive_data[1]*16*16);
    } else if(receive_id==0x98eFff27) {
        if (receive_data[0] == 0xB0 || receive_data[0] == 0xB1)
            key_detection = true;
    }

}
void control::can_state(){
    CAN_data = {0, 0, 0, 0, 0, 0, 0, 0xF0};
    CAN_id = 0X18ec274f;
    can_send(CAN_id, CAN_dlc, CAN_data);
}
