//
// Created by elsa on 19-4-1.
//

#ifndef BOX_RUNTIME_VERTION_MAINWINDOW_H
#define BOX_RUNTIME_VERTION_MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QKeyEvent>
#include <opencv2/opencv.hpp>
#include <QMovie>


using namespace cv;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{

//    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    cv::Mat pic;
    bool index;
    int height = 0;
    int range_low;
    int range_high;
    bool detec = false;
    bool get_height = true;
    int direction = 0;
    void showFrame();
    void key_detect();

private:
    Ui::MainWindow *ui;

    QImage *img1;
    QImage *img2;
    QImage *img3;
    QImage *img4;
    QImage *img5;
    QImage *img6;
    QMovie *mov;

    QImage Mat2QImage(cv::Mat cvImg);

protected:
    void keyPressEvent(QKeyEvent *event);


};


#endif //BOX_RUNTIME_VERTION_MAINWINDOW_H
