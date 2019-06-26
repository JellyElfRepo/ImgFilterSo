#include <jni.h>
#include <string>
#include "native-lib.hpp"
#include "HSL.hpp"
#include "Levels.h"
#include "Filter.hpp"
#include "BlackWhite.hpp"
#include "SelectiveColor.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <android/bitmap.h>
#include <android/log.h>

#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "error", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "debug", __VA_ARGS__))

#define SWAP(a, b, t)  do { t = a; a = b; b = t; } while(0)
#define CLIP_RANGE(value, min, max)  ( (value) > (max) ? (max) : (((value) < (min)) ? (min) : (value)) )
#define COLOR_RANGE(value)  CLIP_RANGE(value, 0, 255)

using namespace std;
using namespace cv;

SelectiveColor  selectiveColor;

extern "C" JNIEXPORT jstring JNICALL
Java_com_cv_effect_NativeFilter_stringFromJNI(
        JNIEnv* env,
        jobject /* this */,jstring jstring1) {

    char* stringFromJava =(char*) env->GetStringUTFChars(jstring1, nullptr);


    string hello = string(stringFromJava);

    env->ReleaseStringUTFChars(jstring1,stringFromJava);

    return env->NewStringUTF(hello.c_str());
}


//可选颜色
//９个颜色通道（红、黄、绿、青、蓝、洋红、白、中性色、黑）。
int color = 8;
int   cyan=100;
int   magenta=150;
int   yellow=200;
int   black=100;
int   is_absolute=0;



//色阶参数
int   channel = 1;
int   Shadow;//输入色阶黑点值
int   Midtones=100;//输入色阶灰点值（注意是浮点数）
int   Highlight;//输入色阶白点值
int   OutputShadow;//输出色阶黑点值
int   OutputHighlight; //输出色阶白点值
static  Levels  levels;

/*
//黑白反差
#define BASE 200
static int red     = 40 + BASE;
static int yellow  = 60 + BASE;
static int green   = 40 + BASE;
static int magenta = 60 + BASE;
static int blue    = 20 + BASE;
static int cyan    = 80 + BASE;
*/


//卡通化处理
void  cartoonize(Mat & src, Mat & dst)
{
    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);//转成灰度图
    const int MEDIAN_BLUR_FILTER_SIZE = 7;
    medianBlur(gray, gray, MEDIAN_BLUR_FILTER_SIZE);//中值滤波

    Mat edges;
    const int LAPLACIAN_FILTER_SIZE = 5;
    Laplacian(gray, edges, CV_8U, LAPLACIAN_FILTER_SIZE);//边缘检测，也可用canny检测或是其他算法

    Mat mask;
    const int EDGES_THRESHOLD = 65;
    threshold(edges, mask, EDGES_THRESHOLD, 255, cv::THRESH_BINARY_INV);//二值化，生成边缘掩码，注意是cv::THRESH_BINARY_INV
    mask.copyTo(dst);

    //为了保证处理速度，将图像大小压缩一倍
    Size srcSize = src.size();
    Size newSize;
    newSize.width = srcSize.width / 2;
    newSize.height = srcSize.height / 2;
    Mat newImg = Mat(newSize, CV_8UC3);
    resize(src, newImg, newSize, 0, 0, cv::INTER_LINEAR);

    //做两次双边滤波
    Mat tmp = Mat(newSize, CV_8UC3);
    int repetitions = 3;
    for(int i=0;i<repetitions;i++)
    {
        int ksize = 9;
        double sigmaColor = 11;
        double sigmaSpace = 5;
        bilateralFilter(newImg, tmp, ksize, sigmaColor, sigmaSpace);
        bilateralFilter(tmp, newImg, ksize, sigmaColor, sigmaSpace);
    }

    Mat resImg;
    resize(newImg, resImg, srcSize, 0, 0, cv::INTER_LINEAR);//调整回原来的大小
    dst.setTo(0);

    resImg.copyTo(dst, mask);
}


//src为原图像， dst为新图像, angle为旋转角度(正值为顺时针旋转,负值为逆时针旋转)
//图像旋转: src为原图像， dst为新图像, angle为旋转角度
void imageRotate(Mat& src, Mat& dst, double angle)
{
    Mat input = src;
    //得到图像大小
    int width = input.cols;
    int height = input.rows;

    //计算图像中心点
    Point2f center;
    center.x = width / 2.0;
    center.y = height / 2.0;

    //获得旋转变换矩阵
    double scale = 1.0;
    Mat trans_mat = getRotationMatrix2D( center, -angle, scale );

    //计算新图像大小
    double angle1 = angle  * CV_PI / 180. ;
    double a = sin(angle1) * scale;
    double b = cos(angle1) * scale;
    double out_width = height * fabs(a) + width * fabs(b);
    double out_height = width * fabs(a) + height * fabs(b);

    //在旋转变换矩阵中加入平移量
    trans_mat.at<double>(0, 2) += cvRound( (out_width - width) / 2 );
    trans_mat.at<double>(1, 2) += cvRound( (out_height - height) / 2);

    //仿射变换
    warpAffine( input, dst, trans_mat, Size(out_width, out_height));
}


//图像剪切
//参数：src为源图像， dst为结果图像, rect为剪切区域
//返回值：返回0表示成功，否则返回错误代码
int imageCrop(Mat src, Mat& dst, Rect rect)
{
    Mat input = src;

    //计算剪切区域：  剪切Rect与源图像所在Rect的交集
    Rect srcRect(0, 0, input.cols, input.rows);
    rect = rect & srcRect;
    if ( rect.width <= 0  || rect.height <= 0 ) return -2;

    //创建结果图像
    dst.create(Size(rect.width, rect.height), src.type());
    Mat output = dst;
    if ( output.empty() ) return -1;

    try {
        //复制源图像的剪切区域 到结果图像
        input(rect).copyTo( output );
        return 0;
    } catch (...) {
        return -3;
    }
}


//调整亮度和对比度
int adjustBrightnessContrast(Mat src, Mat& dst, int brightness, int contrast)
{
    Mat input = src;
    if( input.empty() ) {
        return -1;
    }
    dst.create(src.size(), src.type());
    Mat output = dst;

    brightness = CLIP_RANGE(brightness, -255, 255);
    contrast = CLIP_RANGE(contrast, -255, 255);

    /**
    Algorithm of Brightness Contrast transformation
    The formula is:
        y = [x - 127.5 * (1 - B)] * k + 127.5 * (1 + B);
        x is the input pixel value
        y is the output pixel value
        B is brightness, value range is [-1,1]
        k is used to adjust contrast
            k = tan( (45 + 44 * c) / 180 * PI );
            c is contrast, value range is [-1,1]
    */

    double B = brightness / 255.;
    double c = contrast / 255. ;
    double k = tan( (45 + 44 * c) / 180 * M_PI );

    Mat lookupTable(1, 256, CV_8U);
    uchar *p = lookupTable.data;
    for (int i = 0; i < 256; i++)
        p[i] = COLOR_RANGE( (i - 127.5 * (1 - B)) * k + 127.5 * (1 + B) );

    LUT(input, lookupTable, output);

    return 0;
}


//色阶调整
void channelRead(int which_channel)
{
    channel = which_channel;
    Level * CurrentChannel = NULL;
    switch (channel) {
        case 0: CurrentChannel = &levels.RGBChannel; break;
        case 1: CurrentChannel = &levels.RedChannel; break;
        case 2: CurrentChannel = &levels.GreenChannel; break;
        case 3: CurrentChannel = &levels.BlueChannel; break;
    }
    if ( CurrentChannel == NULL ) return;

    Shadow = CurrentChannel->Shadow;
    Midtones = int (CurrentChannel->Midtones * 100);
    Highlight = CurrentChannel->Highlight;
    OutputShadow = CurrentChannel->OutputShadow;
    OutputHighlight = CurrentChannel->OutputHighlight;

}

void channelWrite()
{
    Level * CurrentChannel = NULL;
    switch (channel) {
        case 0: CurrentChannel = &levels.RGBChannel; break;
        case 1: CurrentChannel = &levels.RedChannel; break;
        case 2: CurrentChannel = &levels.GreenChannel; break;
        case 3: CurrentChannel = &levels.BlueChannel; break;
    }
    if ( CurrentChannel == NULL )
        return ;
    CurrentChannel->Shadow = Shadow;
    CurrentChannel->Midtones = Midtones / 100.0;
    CurrentChannel->Highlight = Highlight;
    CurrentChannel->OutputShadow = OutputShadow;
    CurrentChannel->OutputHighlight = OutputHighlight;
}
//色阶调整

//颜色选择
static void selectColorChannelRead(int which)
{
    color = which;

    SelectiveColorAdjust * current = NULL;
    if ( color >=0 && color <= 9)
        current = &(selectiveColor.colors[color]);
    if ( current == NULL ) return;

    cyan = (current->cyan < 0) ? (current->cyan + 1) * 100 : current->cyan * 100;
    magenta = (current->magenta < 0) ? (current->magenta + 1) * 100 : current->magenta * 100;
    yellow = (current->yellow < 0) ? (current->yellow + 1) * 100 : current->yellow * 100;
    black = (current->black < 0) ? (current->black + 1) * 100 : current->black * 100;

    if ( selectiveColor.isAbsolute )
        is_absolute = 1;
    else
        is_absolute = 0;

}

static void selectColorChannelWrite()
{
    SelectiveColorAdjust * current = NULL;
    if ( color >=0 && color <= 9)
        current = &(selectiveColor.colors[color]);
    if ( current == NULL ) return;

    current->cyan = (cyan - 100 ) / 100.0;
    current->magenta =  (magenta - 100 ) / 100.0;
    current->yellow =  (yellow - 100 ) / 100.0;
    current->black =  (black - 100 ) / 100.0;

    selectiveColor.isAbsolute = ( is_absolute == 1 );

}


//色阶调整
extern "C" JNIEXPORT jobject JNICALL
Java_com_cv_effect_NativeFilter_colorLevelsTweak(JNIEnv* env,
 jobject,jobject bitmap,jint colorChannel
 ,jint shadowValue,jint midtones,jint highlight
 ,jint outputShadow,jint outputHighlight){

//色阶
    /*Level RGBChannel;  //RGB整体调整  0
       Level RedChannel;  //红色通道   1
       Level GreenChannel; //绿色通道  2
       Level BlueChannel; //蓝色通道   3
      */
    Mat tmp;
    //色阶
    //RGB 0,Red 1,Green 2,Blue 3
    //以下六个值是调整的
    channel = colorChannel;
    Shadow= shadowValue;//输入色阶黑点值
    Midtones = midtones;//输入色阶灰点值（注意是浮点数）
    Highlight= highlight;//输入色阶白点值
    OutputShadow=outputShadow;//输出色阶黑点值
    OutputHighlight=outputHighlight; //输出色阶白点值

    nativeUtil::BitmapToMat(env,bitmap,tmp, false);
    //BGRA
    Mat dst= tmp;
    Mat src = tmp;
    cvtColor(tmp,tmp,COLOR_BGRA2BGR);
    channelWrite();

    channelRead(channel);

    levels.adjust(src, dst);
    jobject _bitmap = nativeUtil::mat_to_bitmap(env, dst,bitmap,false);
    return _bitmap;

}//色阶调整


extern "C" JNIEXPORT jobject JNICALL
Java_com_cv_effect_NativeFilter_imgGray(JNIEnv* env,
jobject,jobject bitmap,jint angle){

    Mat tmp;
    int color = 0;
    int hue = 180;
    int saturation = 120;
    int brightness = 100;

    nativeUtil::BitmapToMat(env,bitmap,tmp, false);
    //BGRA
    Mat dst= tmp;
    Mat src = tmp;
    Rect rect(0,0,500,1000);
    cvtColor(tmp,tmp,COLOR_BGRA2BGR);

    //imageRotate(tmp,dst,angle);
    //imageCrop(tmp,dst,rect);
    //cvtColor(dst,dst,COLOR_BGRA2BGR);
    //face_cascade.load("lbpcascade_frontalface.xml");
   // detectAndDisplay(dst);
    //adjustBrightnessContrast(tmp,dst,brightness - 255,contrast - 255);
    //色相（hue）,  饱和度(satuation), 明度(Lightness)
//    hsl.channels[color].hue = hue - 180;
//    hsl.channels[color].saturation = saturation - 100;
//    hsl.channels[color].brightness = brightness - 100;
//
//    hsl.adjust(tmp, dst);

//色阶
 /*Level RGBChannel;  //RGB整体调整  0
    Level RedChannel;  //红色通道   1
    Level GreenChannel; //绿色通道  2
    Level BlueChannel; //蓝色通道   3
    */


  //  channelWrite();
   // channelRead(channel);

   // levels.adjust(src, dst);

   /*
   //黑白反差
    //set params
    BlackWhite b;
    red = 276;
    yellow =237;
    green = 170;
    magenta = 157;
    blue = 275;
    cyan = 241;

    b.red = (red - BASE) / 100.0;
    b.yellow = (yellow - BASE) / 100.0;
    b.green = (green - BASE) / 100.0;
    b.magenta = (magenta - BASE) / 100.0;
    b.blue = (blue - BASE) / 100.0;
    b.cyan = (cyan - BASE) / 100.0;
    //adjust Black White
    b.adjust(src, dst);
*/
    //可选颜色
    //int color = 2;
   // selectColorChannelWrite();
   // selectColorChannelRead(color);
   // selectiveColor.adjust(src, dst);
   // int radius = 3;
    //Filter::HighPass(src,dst,radius);
    cvtColor(dst,dst,COLOR_BGR2BGRA);


    /*
     * dst = (image * (100 - Opacity) + (image + 2 * GaussianBlur (bilateralFilter (image) - image + 128) - 256) * Opacity) /100 ;
     */
//    int value1 = 5, value2 = 2;     //磨皮程度与细节程度的确定
//
//    int dx = value1 * 5;    //双边滤波参数之一
//    double fc = value1*12.5; //双边滤波参数之一
//    int p = 50; //透明度
//    Mat temp1, temp2, temp3, temp4;



//    cvtColor(tmp,tmp,COLOR_BGRA2BGR);
//    cartoonize(tmp,dst);
//    cvtColor(dst,dst,COLOR_BGR2BGRA);

////双边滤波
//    bilateralFilter(srcMat, temp1, dx, fc, fc);
//
//    temp2 = (temp1 - srcMat + 128);
//
////高斯模糊
//    GaussianBlur(temp2, temp3, Size(2 * value2 - 1, 2 * value2 - 1), 0, 0);
//
//    temp4 = srcMat + 2 * temp3 - 255;
//
//    dst = (srcMat*(100 - p) + temp4*p) / 100;
//    dst.copyTo(srcMat);
//
//    cvtColor(dst,dst,COLOR_BGR2BGRA);
//    cvtColor(dst,dst,COLOR_BGRA2GRAY);
//    blur(dst,dst,Size(3,3));
//    Canny( dst, dst, 3, 9,3 );
//    cvtColor(dst,dst,COLOR_GRAY2BGRA);
//    for(int i=0;i<dst.rows;i++){
//        for(int j=0;j<dst.cols;j++){
//
//            dst.at<Vec4b>(i,j)[0] = (uchar)255 -dst.at<Vec4b>(i,j)[0];
//            dst.at<Vec4b>(i,j)[1] = (uchar)255 -dst.at<Vec4b>(i,j)[1];
//            dst.at<Vec4b>(i,j)[2] = (uchar)255 -dst.at<Vec4b>(i,j)[2];
//
//        }
//    }



    jobject _bitmap = nativeUtil::mat_to_bitmap(env, dst,bitmap,false);
    return _bitmap;
}


