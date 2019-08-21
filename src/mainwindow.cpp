//
// Created by elsa on 19-4-1.
//

#include "../include/mainwindow.h"
#include "../include/ui_mainwindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QTime>
#include <QMovie>

extern bool key_detection;
extern bool key_height;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
  ui->setupUi(this);

  index = false;
  img1 = new QImage;
  img1->load("../image/1.jpeg");

  img2 = new QImage;
  img2->load("../image/2.jpeg");

  img3 = new QImage;
  img3->load("../image/3.jpeg");

  img4 = new QImage;
  img4->load("../image/4.jpeg");

  img5 = new QImage;
  img5->load("../image/left.png");

  img6 = new QImage;
  img6->load("../image/right.png");\

  mov = new QMovie("../image/1.gif");

  this->showFullScreen();
}

MainWindow::~MainWindow() {
  delete ui;
}

//显示图像
void MainWindow::showFrame() {

  if (!pic.empty()) {
    QImage image = Mat2QImage(pic);
    //自适应屏幕大小
    QDesktopWidget *desktopWidget = QApplication::desktop();
    image = image.scaled(desktopWidget->size());
    ui->label->setPixmap(QPixmap::fromImage(image));

    //开关
    if (detec) {
      ui->label_2->setPixmap(QPixmap::fromImage(*img2));
    } else {
      ui->label_2->setPixmap(QPixmap::fromImage(*img1));
      height = 0;
      direction = 0;
    }

    if (direction == 1)  //右转
    {
      ui->label_6->setPixmap(QPixmap::fromImage(*img6));
      ui->label_7->setPixmap(QPixmap());
    }
    if (direction == 2)  //左转
    {
      ui->label_6->setPixmap(QPixmap());
      ui->label_7->setPixmap(QPixmap::fromImage(*img5));
    }
    if (direction == 0)  //没有箭头
    {
      ui->label_6->setPixmap(QPixmap());
      ui->label_7->setPixmap(QPixmap());
    }

    //检测
    if (index && detec == true) {

      ui->label_3->setPixmap(QPixmap::fromImage(*img4));
    } else {
      ui->label_3->setPixmap(QPixmap::fromImage(*img3));
      get_height = true;
    }
    //获取高度
    ui->label_4->setText(QString::number(height));
    if (height > range_low && height < range_high && detec) {
      ui->label_4->setStyleSheet("background-color: rgb(0,255,0);");
    } else if (height > range_low && height < range_high && !detec) {
      ui->label_4->setStyleSheet("background-color: rgb(192,192,192);");
    } else if (height <= range_low) {
      ui->label_4->setStyleSheet("background-color: rgb(192,192,192);");
    } else {
      ui->label_4->setStyleSheet("background-color: rgb(255,0,0);");
    }
    //设置字体大小、居中
    QFont ft;
    ft.setPointSize(36);
    ui->label_4->setFont(ft);
    ui->label_4->setFont(ft);
    ui->label_4->setAlignment(Qt::AlignCenter);
    //设置字体颜色
    QPalette pa;
    pa.setColor(QPalette::WindowText, Qt::white);
    ui->label_4->setPalette(pa);

    //缓冲图片
    ui->label_5->setMovie(mov);
    ui->label_5->setFixedSize(151, 101);
    ui->label_5->setScaledContents(true);
  }
}

//Mat类转为QImage类
QImage MainWindow::Mat2QImage(cv::Mat cvImg) {
  QImage qImg;
  if (cvImg.channels() == 3)                             //3 channels color image
  {

    cv::cvtColor(cvImg, cvImg, CV_BGR2RGB);
    qImg = QImage((const unsigned char *) (cvImg.data),
                  cvImg.cols, cvImg.rows,
                  cvImg.cols * cvImg.channels(),
                  QImage::Format_RGB888);
  } else if (cvImg.channels() == 1)                    //grayscale image
  {
    qImg = QImage((const unsigned char *) (cvImg.data),
                  cvImg.cols, cvImg.rows,
                  cvImg.cols * cvImg.channels(),
                  QImage::Format_Indexed8);
  } else {
    qImg = QImage((const unsigned char *) (cvImg.data),
                  cvImg.cols, cvImg.rows,
                  cvImg.cols * cvImg.channels(),
                  QImage::Format_RGB888);
  }

  return qImg;
}
void MainWindow::key_detect() {
  if (key_detection) {
    detec = !detec;
    get_height = true;
    std::cout << "1111111111111111111111111111111111111111111111111111" << std::endl;
    key_detection = false;

  }
  if (key_height && detec == 0)
    key_height = false;
  if (key_height && detec) {
    get_height = true;
    key_height = false;
    ui->label_5->show();
    mov->start();
    //QTimer::singleShot(200, this, SLOT(slotHideFinishedLabel()));
    QTime time;
    time.start();
    while (time.elapsed() < 100)             //等待时间流逝5秒钟
      QCoreApplication::processEvents();   //处理事件
    mov->stop();
    ui->label_5->hide();
  }
}
//键盘按键
void MainWindow::keyPressEvent(QKeyEvent *event) {

}
