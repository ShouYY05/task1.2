#include <iostream>
#include<opencv2/opencv.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>   //ɫ�ʿռ�ת��
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>

using namespace cv;
using namespace std;

int main()
{
    //��ȡ��Ƶ��ÿһ֡ͼ��
    VideoCapture video("�������� (1).avi");      // ����Ƶ�ļ� debug�޷����룬release�ſ��ԣ�����Opencv���صĿ�����Release �汾
    if (!video.isOpened())               // �ж��Ƿ�򿪳ɹ�
    {
        cout << "open video file failed. " << endl;
        return -1;
    }

    Mat hsvimg;                      //��������ͼ��Ĵ���

    while (true)
    {
        Mat src;
        video >> src;                    // ��ȡͼ��֡��frame
        //if (!src.empty())	imshow("source images", src);   // frame��Ϊ��ʱ����ʾԭʼͼ��

//1.��ȡ��ɫ��ͼ�񣬲���ֵ������
        cvtColor(src, hsvimg, CV_BGR2HSV);                         //�Ѷ�ȡ��srcͼ��תΪHSVͼ
        vector<Mat> channels;                                     //�ֳ�����Ƶ��
        split(src, channels);
        Mat hue = channels.at(0);
        Mat value = channels.at(2);
        Mat dst(hue.rows, hue.cols, CV_8UC1, Scalar(255, 255, 255));   //��������ͼ�񻭰�

        for (int i = 0; i < hue.rows; i++)
        {
            for (int j = 0; j < hue.cols; j++)
            {
                int h = hue.at<uchar>(i, j);                      //�õ�H��V��ֵ
                int v = value.at<uchar>(i, j);
                if (h > 78 && v < 50)                             //ɸѡ��ɫ����Ϊ��ɫֵ
                    dst.at<uchar>(i, j) = 0;
            }
        }
        imshow("��ֵ��Եͼ", dst);


        /*
        //���Ͳ���
        int structElementSize = 1;
        Mat element = getStructuringElement(MORPH_RECT, Size(2 * structElementSize + 1,
            2 * structElementSize + 1), Point(structElementSize, structElementSize));
        dilate(dst, dst, element);
        
        //�����㣬������Ҷ�Ͽ��ܴ��ڵ�С��
        int structElementSize = 1;
        Mat element = getStructuringElement(MORPH_RECT, Size(2 * structElementSize + 1,
            2 * structElementSize + 1), Point(structElementSize, structElementSize));
        morphologyEx(dst, dst, MORPH_CLOSE, element);
        imshow("��ֵͼ", dst);
       */ 
  
// 2.��������+��ˮ��䣬�õ����ֲ�ͬ�ľ���
        Mat element1 = getStructuringElement(MORPH_RECT, Size(5, 5));//�����ں�1
        Mat element2 = getStructuringElement(MORPH_RECT, Size(23, 23));//�����ں�2
        morphologyEx(dst, dst, MORPH_OPEN, element1);//������(ʹͼ������)
        floodFill(dst, Point(0, 0), Scalar(0));//��ˮ��
        morphologyEx(dst, dst, MORPH_CLOSE, element2);//������(����ͼ������)
        imshow("��ˮ���", dst);

//3. ���ýǵ㣬�ҵ�������Σ�����ͼ
        vector<vector<Point>> contours;
        vector<Vec4i> hireachy;
        Point2f vertex[4];                                         //��Ӿ��ε��ĸ�����
        findContours(dst, contours, hireachy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        
        for (int i = 0; i < contours.size(); ++i)
        {
            RotatedRect minRect = minAreaRect(Mat(contours[i]));  //��С��Ӿ���
            
            minRect.points(vertex);                               //��Ӿ��ε��ĸ�����
            if (minRect.size.width * minRect.size.height >600 && minRect.size.width * minRect.size.height < 1000 )
            {
                for (int j = 0;j <= 3;j++)
                {
                    line(src, vertex[j], vertex[(j + 1) % 4], Scalar(255, 255, 255), 2); //����ͼ����㣬�յ㣬��ɫ���߿�
                }
                circle(src, minRect.center, 2, Scalar(0, 0, 255), 3);
            }
        }

        imshow("source images2", src);


        /*
      
        //2.���ʶ���ķ�Χ
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(dst, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point());
        //ֻ�������Χ����,�����������Ĺյ���Ϣ,���е�������Ϣ�����ԭʼͼ���Ӧ���ƫ����Ϊ0

        RotatedRect rect_tmp2;
        bool findTarget = 0;

        if (hierarchy.size())
        {
            for (int i = 0;i >= 0;i = hierarchy[i][0])
            {
                //�ҳ���������С��Ӿ���
                rect_tmp2 = minAreaRect(contours[i]);
                Point2f P[4];
                //�����ε��ĸ��㱣����P��
                rect_tmp2.points(P);
                //���ͼ��
                for (int j = 0;j <= 3;j++)
                {
                    line(src, P[j], P[(j + 1) % 4], Scalar(255, 255, 255), 2); //����ͼ����㣬�յ㣬��ɫ���߿�
                }

                
                //Ϊ͸�ӱ任��׼��
                Point2f srcRect[4];
                Point2f dstRect[4];

                double width;
                double height;

                //��ÿ�������Ŀ�߽��д���ʹ����ڸ�
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

                //ͨ�����ɸѡ
                double area = height * width;
                if (area < 5000)
                {
                    dstRect[0] = Point2f(0, 0);
                    dstRect[1] = Point2f(width, 0);
                    dstRect[2] = Point2f(width, height);
                    
                    //͸�ӱ任�������ɹ������
                    Mat warp_mat = getAffineTransform(srcRect, dstRect);
                    imshow("warp_mat", warp_mat);
                    Mat warp_dst_map;
                    warpAffine(dst, warp_dst_map, warp_mat, warp_dst_map.size());
                    // ��ȡ��ҶͼƬ
                    Mat testim;
                    testim = warp_dst_map(Rect(0, 0, width, height));
                    imshow("testim", testim);
                   
                }



            }
        }
        imshow("source images2", src);

        if (waitKey(300) > 0) break;	                     // delay 30 ms �ȴ��Ƿ񰴼�
        */
        waitKey(50);
    }


    
    cout << "finish";
    return 0;
}


double getDistance(Point2f a, Point2f b)     //����a, b����ľ���
{
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}
