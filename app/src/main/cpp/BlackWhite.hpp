/*
 * BlackWhite Adjust
 *
 * Author: JoStudio
 */

#ifndef OPENCV2_PS_BLACKWHITE_HPP_
#define OPENCV2_PS_BLACKWHITE_HPP_

#include "opencv2/core.hpp"

namespace cv {

class BlackWhite {
public:
	float 	red;     //红色的灰度系数值，取值范围: [-1.0, 1.0]
	float 	yellow;  //黄色的灰度系数值，取值范围: [-1.0, 1.0]
	float 	green;   //绿色的灰度系数值，取值范围: [-1.0, 1.0]
	float 	cyan;    //青色的灰度系数值，取值范围: [-1.0, 1.0]
	float 	blue;    //蓝色的灰度系数值，取值范围: [-1.0, 1.0]
	float 	magenta; //洋红色的灰度系数值，取值范围: [-1.0, 1.0]

	BlackWhite();
	virtual ~BlackWhite();

	int adjust(InputArray src, OutputArray dst);//实施黑白调整
};

} /* namespace cv */

#endif /* OPENCV2_PS_BLACKWHITE_HPP_ */
