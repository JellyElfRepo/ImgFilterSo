/*
 * Filter.hpp
 *
 *  Created on: 2016年7月5日
 *      Author: Administrator
 */

#ifndef OPENCV2_PHOTOS_FILTER_HPP_
#define OPENCV2_PHOTOS_FILTER_HPP_

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"

namespace cv {

class Filter {
public:
	Filter();
	virtual ~Filter();

	/**
	 * Gaussian Blur
	 *
	 * @param src [in]  InputArray
	 * @param dst [out] OutputArray
	 * @param radius [in] Gaussian radius, value range [0.1, 250]
	 *
	 * @return 0 if success, else return error code
	 */
	static int GaussianBlur(InputArray src, OutputArray dst, float radius);

	/**
	 * High Pass
	 *
	 * @param src [in]  InputArray
	 * @param dst [out] OutputArray
	 * @param radius [in] Gaussian radius, value range [0.1, 250]
	 *
	 * @return 0 if success, else return error code
	 */
	static int HighPass(InputArray src, OutputArray dst, float radius);


	static int AddNoise(InputArray src, OutputArray dst);

	/**
	 * Ripple
	 *
	 * @param src [in]  InputArray
	 * @param dst [out] OutputArray
	 * @param center [in] center point of ripple
	 * @param radius [in] radius of ripple
	 * @param phases [in]
	 * @param waveLength [in] wave length
	 * @param amplitude [in] amplitude
	 *
	 * @return 0 if success, else return error code
	 */
	static int Ripple(InputArray src, OutputArray dst, Point center, float radius,
			float phases = 0, int waveLength = 0, int amplitude = 0);

	/**
	 * Pinch
	 *
	 * @param src [in]  InputArray
	 * @param dst [out] OutputArray
	 *
	 * @return 0 if success, else return error code
	 */
	static int Pinch(InputArray src, OutputArray dst, float amount = 0.5, Point center = Point(0,0), float radius = 0.6 );


	/**
	 * Kaleidoscope
	 *
	 * @param src [in]  InputArray
	 * @param dst [out] OutputArray
	 *
	 * @return 0 if success, else return error code
	 */
	static int Kaleidoscope(InputArray src, OutputArray dst);
};

} /* namespace cv */

#endif /* OPENCV2_PHOTOS_FILTER_HPP_ */
