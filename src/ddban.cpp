//
// Created by doudouban on 18-10-4.
//

#include "ddban.h"

extern bool key_detection;

void control::traversal_control(vector<Point3f>& fallpoint_world,int height_of_basket, const int& symbol_step, MainWindow &w, const int& fall_step) {
    ///横向
    CAN_id=0x18efff4f;
    CAN_data[0] = 0x00;
    CAN_data[2] = 0x00;
    CAN_data[3] = 0x00;
    CAN_data[4] = 0x00;
    CAN_data[5] = 0x00;
    CAN_data[6] = 0x07;
    CAN_data[7] = 0xF1;
    for (int i = 0; i < fallpoint_world.size(); i++) {
        for (int j = fallpoint_world.size() - 1; j > i; j--) {
            if (fallpoint_world[i].x < fallpoint_world[j].x) {
                Point3f point = fallpoint_world[i];
                fallpoint_world[i] = fallpoint_world[j];
                fallpoint_world[j] = point;
            }
        }
    }
    //shuchuluodian
//    for(int i=0;i<fallpoint_world.size();i++){
//     cout<<fallpoint_world[i]<<endl;
//    }
//    cout<<fallpoint_world.size()<<endl;
    if(fallpoint_world.size()==0){
        ifstream filename;
        filename.open("../high.txt");
        filename>>height_of_basket;
        filename.close();
        CAN_data[1]=0x00; //stop
        w.direction = 0;
    }
    else if(fallpoint_world[0].x<fall_step){
        CAN_data[1]=0x01; //turn left slowly
        w.direction = 2;
    }
    else {
        for (int i = 0; i <fallpoint_world.size(); ++i) {
            if (fallpoint_world[i].z > height_of_basket) {
                i++;
                if(i>fallpoint_world.size()-1)
                    break;
            }
        if ((fallpoint_world[i].z <= height_of_basket+symbol_step/2)&&(i!=0)) {
            cout<<"i="<<i<<endl;
            if ((fallpoint_world[i].x <= (fall_step/2)) && (fallpoint_world[i].x >= -(fall_step/2))) {
                CAN_data[1]=0x00; //stop?? f1 07 00 00 00 00 00 00
                w.direction = 0;
                count_l=0;
                count_r=0;
                cout<<"000000000000"<<endl;
                break;
            }
            else if((fallpoint_world[i].x) > (fall_step/2) && (++count_r >2)) {
                count_l = 0;
                if (fallpoint_world[i].x < 3*(fall_step/2)) {
                    CAN_data[1]=0x10;//turn right slowly
                    cout<<"1111111111"<<endl;
                    w.direction = 1;
                    break;

                }
/*                else if(fallpoint_world[i].x<5*(fall_step/2)){
                    CAN_data[2]=30;///转动速度为30,转动角度20,右转
                    CAN_data[3]=20;
                    CAN_data[4]=0;
                    CAN_data[6]=0x50;
                    w.direction = 1;
                    cout<<"222222222"<<endl;
                    break;

                } */else{
                    CAN_data[1]=0x50;//turn right fast
                    w.direction = 1;
                    cout<<"333333333"<<endl;
                    break;
                }

            }
            else if ((fallpoint_world[i].x) < -(fall_step/2) && (++count_l >2)) {
                count_r = 0;
                if(fallpoint_world[i].x>20){
                    CAN_data[1]=0x01;//turn left slowly
                    w.direction = 2;
                    cout<<"44444444444"<<endl;
                    break;
                }
//                else if(fallpoint_world[i].x>-3*(fall_step/2)){
//                    CAN_data[2]=30;
//                    CAN_data[3]=30;
//                    CAN_data[4]=0;
//                    CAN_data[6]=0x05;
//                    w.direction = 2;
//                    cout<<"5555555555"<<endl;
//                    break;
//                }
                else{
                    CAN_data[1]=0x05;//turn left fast
                    w.direction = 2;
                    cout<<"66666666666"<<endl;
                    break;
                }

            }
            break;
        }

    }
    }
    can_send(CAN_id,CAN_dlc,CAN_data);
}
void control::vertical_control(const vector<Point2f>& fallPoints_2D )  //喷头角度与落点的关系
{
    cout<<"zongxiangkongzhi_start--------------------"<<endl;
    CAN_id=0x18efff4f;///挡料板控制ID
    CAN_data[0] = 0x00;
    CAN_data[1] = 0x00;
    CAN_data[3] = 0x00;
    CAN_data[4] = 0x00;
    CAN_data[5] = 0x00;
    CAN_data[6] = 0x07;
    CAN_data[7] = 0xF1;
    double a,b,c,temp,mid;
    double s,h;
    double right_fallpoint_d;
    double angle_c; ///结算出挡板与水平线的夹角
    for(int i=0;i<fallPoints_2D.size();i++){
        if(fallPoints_2D[i].x==50){
            right_fallpoint_d=fallPoints_2D[i].y;
            break;

        }
    }
    right_fallpoint_d/=1000;
    cout<<right_fallpoint_d<<endl;
    s=right_fallpoint_d-arm_r*cos(angle_b*Pi/180);
    h=arm_r*sin(angle_b*Pi/180);
    a=(g1*s*s)/(2*v*v);///a>0
    b=s;///b>0
    c=(g1*s*s)/(2*v*v)-h;///   开口向上，对称轴在负半轴，
    temp=b*b-4*a*c;
    double angle_1,angle_2;
    cout<<"a="<<a<<' '<<"b="<<b<<' '<<"c="<<c<<endl;
    cout<<"temp="<<temp<<endl;
    if(temp>0)
    {
        mid=(-b+sqrt(temp))/(2*a);
        angle_1=atan(mid);
        mid=(-b-sqrt(temp))/(2*a);
        angle_2=atan(mid);
        if(angle_reslut>angle_2)
            angle_c=angle_1;
        else
            angle_c=angle_2;
    }
    if(temp<=0)
    {
        angle_c=angle_a-angle_b;
    }
    cout<<angle_1<<" "<<angle_2<<endl;
    angle_reslut=angle_c+angle_b-angle_a;
    cout<<angle_reslut<<endl;
    cout<<SensorAngle<<endl;
    if(SensorAngle>(angle_reslut-100)&&SensorAngle<(angle_reslut+100)){
        CAN_data[2] = 0x00; //??stop f1 07 00 00 00 00 00 00
        //zan ting daoliaoban
    }
    else if(SensorAngle<angle_reslut){
        CAN_data[2] = 0x10;  //down
        //  tiao jin dao liao ban
    }
    else{
        CAN_data[2] = 0x01; //up
        //tiao yuan dao liao ban
    }
     can_send(CAN_id,CAN_dlc,CAN_data);
     cout<<"zongxiangkongzhi_end--------------------"<<endl;
}

void control::vertical_control_Vision(const vector<Point2f>& fallPoints_2D,  const vector<Point3f>& box_UPworld, const int& x_axisFallPoint ){
    CAN_id=0x18efff4f;///挡料板控制ID
    CAN_data[0] = 0x00;
    CAN_data[1] = 0x00;
    CAN_data[3] = 0x00;
    CAN_data[4] = 0x00;
    CAN_data[5] = 0x00;
    CAN_data[6] = 0x07;
    CAN_data[7] = 0xF1;

    int ActualFallPointDastance;
    int IdealFallPointDastance;
    if(box_UPworld.size()){
        for(int i=0;i<box_UPworld.size();i++){
            ActualFallPointDastance += box_UPworld[i].y;
        }
        ActualFallPointDastance /= box_UPworld.size();
        for(int i=0;i<fallPoints_2D.size();i++){
            if(fallPoints_2D[i].x == x_axisFallPoint){
                IdealFallPointDastance=fallPoints_2D[i].y;
                break;
            }
        }
        if(ActualFallPointDastance>(IdealFallPointDastance-100)&&ActualFallPointDastance<(IdealFallPointDastance+100)){
            CAN_data[2] = 0x00; //??stop f1 07 00 00 00 00 00 00
            //zan ting daoliaoban
        }
        else if(ActualFallPointDastance<IdealFallPointDastance){
            CAN_data[2] = 0x10;  //down
            //  tiao jin dao liao ban
        }
        else{
            CAN_data[2] = 0x01; //up
            //tiao yuan dao liao ban
        }
    }
    can_send(CAN_id,CAN_dlc,CAN_data);
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
