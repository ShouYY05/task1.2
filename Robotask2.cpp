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
    VideoCapture video("能量机关 (1).avi");      // 打开视频文件 debug无法编译，release才可以！！！Opencv下载的可能是Release 版本
    if (!video.isOpened())               // 判断是否打开成功
    {
        cout << "open video file failed. " << endl;
        return -1;
    }

    Mat hsvimg;                      //创建接受图像的窗口

    while (true)
    {
        Mat src;
        video >> src;                    // 读取图像帧至frame
        //if (!src.empty())	imshow("source images", src);   // frame不为空时，显示原始图像

//1.提取蓝色的图像，并二值化处理
        cvtColor(src, hsvimg, CV_BGR2HSV);                         //把读取的src图像转为HSV图
        vector<Mat> channels;                                     //分成三个频道
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


        /*
        //膨胀操作
        int structElementSize = 1;
        Mat element = getStructuringElement(MORPH_RECT, Size(2 * structElementSize + 1,
            2 * structElementSize + 1), Point(structElementSize, structElementSize));
        dilate(dst, dst, element);
        
        //闭运算，消除扇叶上可能存在的小洞
        int structElementSize = 1;
        Mat element = getStructuringElement(MORPH_RECT, Size(2 * structElementSize + 1,
            2 * structElementSize + 1), Point(structElementSize, structElementSize));
        morphologyEx(dst, dst, MORPH_CLOSE, element);
        imshow("二值图", dst);
       */ 
  
// 2.开闭运算+漫水填充，得到两种不同的矩形
        Mat element1 = getStructuringElement(MORPH_RECT, Size(5, 5));//设置内核1
        Mat element2 = getStructuringElement(MORPH_RECT, Size(23, 23));//设置内核2
        morphologyEx(dst, dst, MORPH_OPEN, element1);//开运算(使图形明显)
        floodFill(dst, Point(0, 0), Scalar(0));//漫水法
        morphologyEx(dst, dst, MORPH_CLOSE, element2);//闭运算(减少图形数量)
        imshow("漫水填充", dst);

//3. 利用角点，找到所需矩形，并画图
        vector<vector<Point>> contours;
        vector<Vec4i> hireachy;
        Point2f vertex[4];                                         //外接矩形的四个顶点
        findContours(dst, contours, hireachy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        
        for (int i = 0; i < contours.size(); ++i)
        {
            RotatedRect minRect = minAreaRect(Mat(contours[i]));  //最小外接矩形
            
            minRect.points(vertex);                               //外接矩形的四个顶点
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


        /*
      
        //2.框出识别板的范围
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(dst, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point());
        //只检测最外围轮廓,仅保存轮廓的拐点信息,所有的轮廓信息相对于原始图像对应点的偏移量为0

        RotatedRect rect_tmp2;
        bool findTarget = 0;

        if (hierarchy.size())
        {
            for (int i = 0;i >= 0;i = hierarchy[i][0])
            {
                //找出轮廓的最小外接矩形
                rect_tmp2 = minAreaRect(contours[i]);
                Point2f P[4];
                //将矩形的四个点保存在P中
                rect_tmp2.points(P);
                //框出图像
                for (int j = 0;j <= 3;j++)
                {
                    line(src, P[j], P[(j + 1) % 4], Scalar(255, 255, 255), 2); //所画图像，起点，终点，颜色，线宽
                }

                
                //为透视变换做准备
                Point2f srcRect[4];
                Point2f dstRect[4];

                double width;
                double height;

                //对每个轮廓的宽高进行处理使宽大于高
                width = getDistance(P[0], P[1]);
                height = getDistance(P[1], P[2]);

                if (width > height)
                {
                    srcRect[0] = P[0];
                    srcRect[1] = P[1];
                    srcRect[2] = P[2];
                    
                }
                else
                {
                    swap(width, height);
                    srcRect[0] = P[1];
                    srcRect[1] = P[2];
                    srcRect[2] = P[3];
                    
                }

                //通过面积筛选
                double area = height * width;
                if (area < 5000)
                {
                    dstRect[0] = Point2f(0, 0);
                    dstRect[1] = Point2f(width, 0);
                    dstRect[2] = Point2f(width, height);
                    
                    //透视变换，矫正成规则矩形
                    Mat warp_mat = getAffineTransform(srcRect, dstRect);
                    imshow("warp_mat", warp_mat);
                    Mat warp_dst_map;
                    warpAffine(dst, warp_dst_map, warp_mat, warp_dst_map.size());
                    // 提取扇叶图片
                    Mat testim;
                    testim = warp_dst_map(Rect(0, 0, width, height));
                    imshow("testim", testim);
                   
                }



            }
        }
        imshow("source images2", src);

        if (waitKey(300) > 0) break;	                     // delay 30 ms 等待是否按键
        */
        waitKey(50);
    }


    
    cout << "finish";
    return 0;
}


double getDistance(Point2f a, Point2f b)     //计算a, b两点的距离
{
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}
