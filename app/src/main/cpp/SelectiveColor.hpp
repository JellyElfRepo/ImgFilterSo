/*
 * Adjust Selective Colors
 *
 * Author: JoStudio
 */

#ifndef OPENCV2_PS_SELECTIVECOLOR_HPP_
#define OPENCV2_PS_SELECTIVECOLOR_HPP_

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
using namespace cv;

namespace cv {

typedef enum select_color {
	SELECT_RED,
	SELECT_YELLOW,
	SELECT_GREEN,
	SELECT_CYAN,
	SELECT_BLUE,
	SELECT_MAGENTA,
	SELECT_WHITE,
	SELECT_MIDDLE,
	SELECT_BLACK
}  select_color_t;

/**
 * Class of Adjustment for One Select Color
 */
class SelectiveColorAdjust{
public:
	float cyan;       //青色调整值，取值范围： [-1, 1]
	float magenta;    //取值范围： [-1, 1]
	float yellow;     //取值范围： [-1, 1]
	float black;      //取值范围： [-1, 1]


	bool  defined;

	SelectiveColorAdjust();
	virtual ~SelectiveColorAdjust();
	void   calcDefined();
};

/**
 * Class of Selective Color
 */
class SelectiveColor {
public:
	bool isAbsolute;  //是否采用绝对方法
	SelectiveColorAdjust colors[ SELECT_BLACK + 1 ]; //9种可选颜色通道

	SelectiveColor();
	virtual ~SelectiveColor();

	int adjust(InputArray src, OutputArray dst); //实施可选颜色调整
};

} /* namespace cv */

#endif /* OPENCV2_PS_SELECTIVECOLOR_HPP_ */
