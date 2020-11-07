#include <iostream>
#include<opencv2/opencv.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>   //色彩空间转换
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>

using namespace cv;
using namespace std;

int main()
{
    //截取视频的每一帧图像
    VideoCapture video("能量机关 (1).avi");
    // 判断是否打开成功
    if (!video.isOpened())              
    {
        cout << "open video file failed. " << endl;
        return -1;
    }
    //创建接受图像的窗口
    Mat hsvimg;                      

    while (true)
    {
        Mat src;
        video >> src;                  
        //if (!src.empty())	imshow("source images", src);   // frame不为空时，显示原始图像

//1.提取蓝色的图像，并二值化处理
        cvtColor(src, hsvimg, CV_BGR2HSV);
        vector<Mat> channels;                  
        split(src, channels);
        Mat hue = channels.at(0);
        Mat value = channels.at(2);
        Mat dst(hue.rows, hue.cols, CV_8UC1, Scalar(255, 255, 255));   //建立纯白图像画板

        for (int i = 0; i < hue.rows; i++)
        {
            for (int j = 0; j < hue.cols; j++)
            {
                int h = hue.at<uchar>(i, j);                      //该点H、V的值
                int v = value.at<uchar>(i, j);
                if (h > 78 && v < 50)                             //筛选蓝色区域为黑色值
                    dst.at<uchar>(i, j) = 0;
            }
        }
        imshow("二值边缘图", dst);

  
// 2.开闭运算+漫水填充，得到两种不同的矩形
        Mat element1 = getStructuringElement(MORPH_RECT, Size(5, 5));   //设置内核1
        Mat element2 = getStructuringElement(MORPH_RECT, Size(23, 23)); //设置内核2
        morphologyEx(dst, dst, MORPH_OPEN, element1);                   //开运算(使图形明显)
        floodFill(dst, Point(0, 0), Scalar(0));                         //漫水法
        morphologyEx(dst, dst, MORPH_CLOSE, element2);                  //闭运算(减少图形数量)
        imshow("漫水填充", dst);

//3. 利用角点，找到所需矩形，并画图
        vector<vector<Point>> contours;
        vector<Vec4i> hireachy;
        Point2f vertex[4];                                              //外接矩形的四个顶点
        findContours(dst, contours, hireachy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        
        for (int i = 0; i < contours.size(); ++i)
        {
            RotatedRect minRect = minAreaRect(Mat(contours[i]));        //最小外接矩形
            
            minRect.points(vertex);                                     //外接矩形的四个顶点
            if (minRect.size.width * minRect.size.height >600 && minRect.size.width * minRect.size.height < 1000 )
            {
                for (int j = 0;j <= 3;j++)
                {
                    line(src, vertex[j], vertex[(j + 1) % 4], Scalar(255, 255, 255), 2); //所画图像，起点，终点，颜色，线宽
                }
                circle(src, minRect.center, 2, Scalar(0, 0, 255), 3);
            }
        }

        imshow("source images2", src);


        waitKey(50);
    }

    cout << "finish";
    return 0;
}


double getDistance(Point2f a, Point2f b)     //计算a, b两点的距离
{
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}
