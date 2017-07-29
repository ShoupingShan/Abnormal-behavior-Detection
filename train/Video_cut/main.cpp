#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<iostream>
using namespace std;
using namespace cv;
char s[50]="/home/shp/codeBlocks/HoughCircle/samples/400.avi";
int main(int argc, char *argv[])
{
   // 读取视频流
    VideoCapture cap("DJI_0011.avi");
    int nums=0;
   // 检测视频是否读取成功
    if (!cap.isOpened())
    {
       cout<<"视频流打开失败";
       return 1;
    }
    else
    {
        int numframes=(int)cap.get(CV_CAP_PROP_FRAME_COUNT);
        int width=(int)cap.get(CV_CAP_PROP_FRAME_WIDTH);
        int height=(int)cap.get(CV_CAP_PROP_FRAME_HEIGHT);
        double fps=(double)cap.get(CV_CAP_PROP_FPS);
        int cnt=0;
        Mat frame;
         VideoWriter wr(s,CV_FOURCC('M','P','4','2'),fps,Size(width,height));
        while(cap.read(frame))
        {
            wr<<frame;
            cnt++;
            if(cnt>=40)
            {
                nums++;
                cnt=0;
                int a=nums/100;
                int b=(nums%100)/10;
                int c=nums%10;
                s[41]='4'+a;
                s[42]='0'+b;
                s[43]='0'+c;
                wr=VideoWriter(s,CV_FOURCC('M','P','4','2'),fps,Size(width,height));
            }
        }
    }
    return 0;
}
