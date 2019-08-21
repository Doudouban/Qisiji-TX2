//
// Created by doudouban on 18-10-4.
//

#include "ddban.h"
extern bool key_detection;

void control::traversal_control(vector<Point3f>& fallpoint_world,int height_of_basket, MainWindow &w, const int& fall_step) {
    ///横向
    CAN_id=0x18efff4f;
    CAN_data[0] = 0xF1;
    CAN_data[1] = 0x07;
    CAN_data[2] = 0x00;
    CAN_data[3] = 0x00;
    CAN_data[4] = 0x00;
    CAN_data[5] = 0x00;
    CAN_data[7] = 0x00;
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
        //CAN_data[6]=0x00; //stop
        w.direction = 0;
    }
    else if(fallpoint_world[0].x<fall_step){
        CAN_data[6]=0x50; //turn left slowly
        w.direction = 2;
        can_send(CAN_id,CAN_dlc,CAN_data);
    }
    else {
//        if(fallpoint_world.size()==1){
//            if(fallpoint_world[0].x<0){
//                CAN_data[6]=0x50;
//                can_send(CAN_id,CAN_dlc,CAN_data);can_send(CAN_id,CAN_dlc,CAN_data);
//            }
//            else if(fallpoint_world[0].x>0){
//                CAN_data[6]=0x05;
//                can_send(CAN_id,CAN_dlc,CAN_data);can_send(CAN_id,CAN_dlc,CAN_data);
//            }
//        }
        for (int i = 1; i <fallpoint_world.size(); ++i) {
             if (fallpoint_world[i].z <= height_of_basket+range_up/2) {
                cout<<"i="<<i<<endl;
                if (fallpoint_world[i].x == x_axisFallPoint) {
                   //CAN_data[6]=0x00; //stop?? f1 07 00 00 00 00 00 00
                    w.direction = 0;
                    count_l=0;
                    count_r=0;
                    cout<<"000000000000"<<endl;
                    break;
                }
                else if((fallpoint_world[i].x) > (2*fall_step/3) && (++count_r >threshold)) {
                    count_l = 0;
                    if (fallpoint_world[i].x < 2*fall_step/3) {
                        CAN_data[6]=0x01;//turn right slowly
                        cout<<"1111111111"<<endl;
                        w.direction = 1;
                        can_send(CAN_id,CAN_dlc,CAN_data);
                        break;
                    }
                    else if(++count_r>threshold){
                        CAN_data[6]=0x05;//turn right fast
                        w.direction = 1;
                        cout<<"333333333"<<endl;
                        can_send(CAN_id,CAN_dlc,CAN_data);
                        break;
                    }

                }
                else if ((fallpoint_world[i].x) <0 && (++count_l >threshold)) {
                    count_r = 0;
                    if(fallpoint_world[i].x>-2*fall_step){
                        CAN_data[6]=0x10;//turn left slowly
                        w.direction = 2;
                        cout<<"44444444444"<<endl;
                        can_send(CAN_id,CAN_dlc,CAN_data);
                        break;
                    }
                    else if(++count_l>threshold){
                        CAN_data[6]=0x50;//turn left fast
                        w.direction = 2;
                        cout<<"66666666666"<<endl;
                        can_send(CAN_id,CAN_dlc,CAN_data);
                        break;
                    }

                }
                break;
            }

        }
    }
    //can_send(CAN_id,CAN_dlc,CAN_data);

}
void control::all_control(vector<Point3f>& fallpoint_world,int height_of_basket, MainWindow &w, const int& fall_step, const vector<Point2f>& fallPoints_2D, const vector<Point3f>& box_UPworld, const int& x_axisFallPoint){
    CAN_id=0x18efff4f;///挡料板控制ID
    CAN_data[0] = 0xF1;
    CAN_data[1] = 0x07;
    CAN_data[2] = 0x00;
    CAN_data[3] = 0x00;
    CAN_data[4] = 0x00;
    CAN_data[7] = 0x00;
    for (int i = 0; i < fallpoint_world.size(); i++) {
        for (int j = fallpoint_world.size() - 1; j > i; j--) {
            if (fallpoint_world[i].x < fallpoint_world[j].x) {
                Point3f point = fallpoint_world[i];
                fallpoint_world[i] = fallpoint_world[j];
                fallpoint_world[j] = point;
            }
        }
    }
    if(fallpoint_world.size()==0){
        ifstream filename;
        filename.open("../high.txt");
        filename>>height_of_basket;
        filename.close();
        CAN_data[6]=0x00; //stop
        w.direction = 0;
    }
    else if(fallpoint_world[0].x<fall_step){
        CAN_data[6]=0x50; //turn left slowly
        w.direction = 2;
    }
    else {
        for (int i = 0; i <fallpoint_world.size(); ++i) {
            if ((fallpoint_world[i].z <= height_of_basket+range_up/2)&&(i!=0)) {
                cout<<"i="<<i<<endl;
                if (fallpoint_world[i].x == x_axisFallPoint) {
                    CAN_data[6]=0x00; //stop?? f1 07 00 00 00 00 00 00
                    w.direction = 0;
                    count_l=0;
                    count_r=0;
                    cout<<"000000000000"<<endl;
                    break;
                }
                else if((fallpoint_world[i].x) > (2*fall_step/3) && (++count_r >threshold)) {
                    count_l = 0;
                    if (fallpoint_world[i].x < 2*fall_step) {
                        CAN_data[6]=0x01;//turn right slowly
                        cout<<"1111111111"<<endl;
                        w.direction = 1;                     
                        break;
                    }
                    else if(++count_r>threshold){
                        CAN_data[6]=0x05;//turn right fast
                        w.direction = 1;
                        cout<<"333333333"<<endl;
                        break;
                    }

                }
                else if ((fallpoint_world[i].x) <0 && (++count_l >threshold)) {
                    count_r = 0;
                    if(fallpoint_world[i].x>-2*fall_step){
                        CAN_data[6]=0x10;//turn left slowly
                        w.direction = 2;
                        cout<<"44444444444"<<endl;
                        break;
                    }
                    else if(++count_l>threshold){
                        CAN_data[6]=0x50;//turn left fast
                        w.direction = 2;
                        cout<<"66666666666"<<endl;
                        break;
                    }

                }
                break;
            }

        }
    }

    cout<<"zongxiangkongzhiVision_start--------------------"<<endl;
    int ActualFallPointDastance;
    int IdealFallPointDastance;
    cout<<"box_UPworld.size="<<box_UPworld.size()<<endl;
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
        if(ActualFallPointDastance>(IdealFallPointDastance-250)&&ActualFallPointDastance<(IdealFallPointDastance+250)){
            CAN_data[5] = 0x00; //??stop f1 07 00 00 00 00 00 00
            //zan ting daoliaoban
            cout<<"---------------------------------------------------------------------------------------okey"<<endl;
        }
        else if(ActualFallPointDastance<IdealFallPointDastance){
            CAN_data[5] = 0x10;  //down
            //  tiao jin dao liao ban
            cout<<"---------------------------------------------------------------------------------------down"<<endl;
        }
        else{
            CAN_data[5] = 0x01; //up
            //tiao yuan dao liao ban
            cout<<"---------------------------------------------------------------------------------------up"<<endl;
        }
    }
    cout<<"zongxiangkongzhiVision_end--------------------"<<endl;
    if(CAN_data[5]!=0&&CAN_data[6]!=0){
        can_send(CAN_id,CAN_dlc,CAN_data);
    }

}
void control::vertical_control(const vector<Point2f>& fallPoints_2D )  //喷头角度与落点的关系
{
    cout<<"zongxiangkongzhi_start--------------------"<<endl;
    CAN_id=0x18efff4f;///挡料板控制ID
    CAN_data[0] = 0xF1;
    CAN_data[1] = 0x07;
    CAN_data[2] = 0x00;
    CAN_data[3] = 0x00;
    CAN_data[4] = 0x00;
    CAN_data[6] = 0x00;
    CAN_data[7] = 0x00;
    double a,b,c,temp,mid;
    double s,h;
    double right_fallpoint_d;
    double angle_c; ///结算出挡板与水平线的夹角
    for(int i=0;i<fallPoints_2D.size();i++){
        if(fallPoints_2D[i].x==50){
            right_fallpoint_d=fallPoints_2D[i].y/1000;
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
    if(SensorAngle>(angle_reslut)&&SensorAngle<(angle_reslut)){
        CAN_data[5] = 0x00; //??stop f1 07 00 00 00 00 00 00
        //zan ting daoliaoban
    }
    else if(SensorAngle<angle_reslut){
        CAN_data[5] = 0x10;  //down
        //  tiao jin dao liao ban
    }
    else{
        CAN_data[5] = 0x01; //up
        //tiao yuan dao liao ban
    }
     can_send(CAN_id,CAN_dlc,CAN_data);
     cout<<"zongxiangkongzhi_end--------------------"<<endl;
}

void control::vertical_control_Vision(const vector<Point2f>& fallPoints_2D,  const vector<Point3f>& box_UPworld, const int& x_axisFallPoint ){
    CAN_id=0x18efff4f;///挡料板控制ID
    CAN_data[0] = 0xF1;
    CAN_data[1] = 0x07;
    CAN_data[2] = 0x00;
    CAN_data[3] = 0x00;
    CAN_data[4] = 0x00;
    CAN_data[6] = 0x00;
    CAN_data[7] = 0x00;
    cout<<"zongxiangkongzhiVision_start--------------------"<<endl;
    int ActualFallPointDastance;
    int IdealFallPointDastance;
    cout<<"box_UPworld.size="<<box_UPworld.size()<<endl;
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
        if(ActualFallPointDastance>(IdealFallPointDastance-250)&&ActualFallPointDastance<(IdealFallPointDastance+250)){
            //CAN_data[5] = 0x00; //??stop f1 07 00 00 00 00 00 00
            //zan ting daoliaoban
            cout<<"---------------------------------------------------------------------------------------okey"<<endl;
        }
        else if(ActualFallPointDastance<IdealFallPointDastance){
            CAN_data[5] = 0x10;  //down
            can_send(CAN_id,CAN_dlc,CAN_data);
            //  tiao jin dao liao ban
            cout<<"---------------------------------------------------------------------------------------down"<<endl;
        }
        else{
            CAN_data[5] = 0x01; //up
            can_send(CAN_id,CAN_dlc,CAN_data);
            //tiao yuan dao liao ban
            cout<<"---------------------------------------------------------------------------------------up"<<endl;
        }
    }
    //can_send(CAN_id,CAN_dlc,CAN_data);
     cout<<"zongxiangkongzhiVision_end--------------------"<<endl;
}

void control::receive_control(){
    can_receive(receive_id,receive_data);
   if(receive_id==0x18EFFF4F) {
        if (detect){
            if(receive_data[0]==0xF1&&receive_data[1]==0x07&&receive_data[2]==0x00&&receive_data[3]==0x00&&receive_data[4]==0x00&&receive_data[7]==0x00){
                if(receive_data[5]==0x00&&(receive_data[6]==0x10||receive_data[0]==0x01)){
                    key_detection = true;
                }else if(receive_data[5]==0x00&&(receive_data[6]==0x50||receive_data[0]==0x05)){
                    key_detection = true;
                }else if(receive_data[6]==0x00&&(receive_data[6]==0x10||receive_data[0]==0x01)){
                    key_detection = true;
                }
            }
         }
     }
   else if(receive_id==0x1CE65526){
        velocity1=receive_data[0];
        velocity2=receive_data[1];
        velocity3=receive_data[2];
        rev_velocity=true;
   }

}
void control::CAN_back_velocity(){
        CAN_id=0x1ce63326;
        int t=velocity1;
        CAN_data={0x05,0x1D,0x04,0xFF,velocity1};
        can_send(CAN_id,CAN_dlc,CAN_data);
        delay_ms(1);
        CAN_data={0x05,0xF0,0x03,0xFF,velocity2};
        can_send(CAN_id,CAN_dlc,CAN_data);
        delay_ms(1);
        CAN_data={0x05,0x23,0x04,0xFF,velocity3};
        can_send(CAN_id,CAN_dlc,CAN_data);
}
void control::can_state(){
    CAN_data = {1, 1, 1, 1, 1, 1, 1, 1};
    CAN_id = 0X1ce62655;
    can_send(CAN_id, CAN_dlc, CAN_data);
}

void control::delay_ms(int ms) {

    usleep(ms * 10000); // 微妙us

}
void control::Set_velocity(){
    CAN_id=0x1ce63326;
    int t=velocity1;
    CAN_data={0x05,0x1D,0x04,0xFF,set_velocity[0]};
    can_send(CAN_id,CAN_dlc,CAN_data);
    delay_ms(1);
    CAN_data={0x05,0xF0,0x03,0xFF,set_velocity[1]};
    can_send(CAN_id,CAN_dlc,CAN_data);
    delay_ms(1);
    CAN_data={0x05,0x23,0x04,0xFF,set_velocity[2]};
    can_send(CAN_id,CAN_dlc,CAN_data);
}

