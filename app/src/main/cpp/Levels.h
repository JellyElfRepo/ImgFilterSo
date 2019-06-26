//
// Created by wentu on 2019/5/5.
//

#ifndef IMAGEPROCESSAPP_LEVELS_H
#define IMAGEPROCESSAPP_LEVELS_H

#endif //IMAGEPROCESSAPP_LEVELS_H
#ifndef OPENCV2_PS_LEVELS_HPP_
#define OPENCV2_PS_LEVELS_HPP_

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
using namespace cv;

namespace cv {

/**
 * Class of Level for one channel
 */
    class Level {
    public:
        int   Shadow;  //输入色阶黑点值
        float Midtones; //输入色阶灰点值（注意是浮点数）
        int   Highlight; //输入色阶白点值

        int   OutputShadow; //输出色阶黑点值
        int   OutputHighlight; //输出色阶白点值

        Level();
        virtual ~Level();

        bool createColorTable(uchar * colorTable);
        void clear();
    };//Level

/**
 * Class of Levels for all channels
 */
    class Levels {
    protected:
        bool createColorTables(uchar colorTables[][256]);

    public:
        Level RGBChannel;  //RGB整体调整
        Level RedChannel;  //红色通道
        Level GreenChannel; //绿色通道
        Level BlueChannel; //蓝色通道

        Levels();
        virtual ~Levels();

        int adjust(Mat src, Mat& dst); //实施色阶调整
    };


} /* namespace cv */

#endif /* OPENCV2_PS_LEVELS_HPP_ */
