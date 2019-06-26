/*
 * Filter.cpp
 *
 *  Created on: 2016年7月5日
 *      Author: Administrator
 */

#include "Filter.hpp"
#include <math.h>
#include <iostream>

#define CLIP_RANGE(value, min, max)  ( (value) > (max) ? (max) : (((value) < (min)) ? (min) : (value)) )
#define COLOR_RANGE(value)  CLIP_RANGE((value), 0, 255)

namespace cv {

Filter::Filter() {

}

Filter::~Filter() {
}

/**
 * Gaussian Blur
 *
 * @param src [in]  InputArray
 * @param dst [out] OutputArray
 * @param radius [in] Gaussian radius, value range [0.1, 250]
 *
 * @return 0 if success, else return error code
 */
int Filter::GaussianBlur(InputArray src, OutputArray dst, float radius)
{
	if ( radius < 0.1 ) return -1;
	if ( radius > 250 ) return -1;
	cv::GaussianBlur(src, dst, Size(0,0), radius, radius);
	return 0;
}

/**
 * High Pass
 *
 * @param src [in]  InputArray
 * @param dst [out] OutputArray
 * @param radius [in] Gaussian radius, value range [0.1, 250]
 *
 * @return 0 if success, else return error code
 */
int Filter::HighPass(InputArray src, OutputArray dst, float radius)
{
	Mat input = src.getMat();
	if( input.empty() ) {
		return -1;
	}

	dst.create(src.size(), src.type());
	Mat output = dst.getMat();

	if ( radius < 0.1 ) return -1;
	if ( radius > 250 ) return -1;
	cv::GaussianBlur(input, output, Size(0, 0), radius, radius);

	const uchar *in;
	uchar *out;
	int width = input.cols;
	int height = input.rows;
	int channels = input.channels();

	for (int y = 0; y < height; ++y) {
		in = input.ptr<uchar>(y);
		out = output.ptr<uchar>(y);
		for (int x = 0; x < width; ++x) {
			for (int i = 0; i < 3; ++i ) {
				*out = COLOR_RANGE((*in - *out) + 127);
				++out; ++in;
			}

			for (int i = 0; i < channels-3; ++i) {
				*out++ = *in++;
			}
		}
	}
	return 0;
}



int Filter::AddNoise(InputArray src, OutputArray dst)
{
	Mat input = src.getMat();
	if( input.empty() ) {
		return -1;
	}

	dst.create(src.size(), src.type());
	Mat output = dst.getMat();

	//output = input.clone();
	cv::randn(output, 0.5, 0.25);
	return 1;
}

#define PRINT_VAR(a) std::cout << #a << "= " << a << std::endl

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
int Filter::Ripple(InputArray src, OutputArray dst, Point center, float radius, float phases,
		int wave_length, int amplitude_in)
{
	Mat input = src.getMat();
	if( input.empty() ) {
		return -1;
	}

	dst.create(src.size(), src.type());
	Mat output = dst.getMat();

	int width = input.cols;
	int height = input.rows;

	float centreX = center.x * 1.0 / width; //0.25;
	float centreY = center.y * 1.0  / height; //0.5;
	//float radius = 90;

	if (radius==0)
		radius = std::min(width, height) / 2;

	int wavelength = (wave_length == 0) ?  radius / 2 : wave_length; //50;
	int amplitude = (amplitude_in == 0) ? wavelength / 4 : amplitude_in; //20

	if ( phases <= 2) phases = 4;
	float phase = M_PI / phases; // default = M_PI / 4;

	float icentreX=width*centreX;
	float icentreY=height*centreY;
	float radius2=radius*radius;

	float dx,dy,new_x,new_y;
	float p,q,x1,y1; //,x0,y0;
	float distance, distance2;
	float amount;

	for (int y=0; y<height; y++)
	{
		for (int x=0; x<width; x++)
		{

			 dx = x-icentreX;
			 dy = y-icentreY;

			 distance2=dx*dx+dy*dy;

			 if (distance2>radius2)
			 {
				 new_x=x;
				 new_y=y;
			 }
			 else
			 {
				 distance=sqrt(distance2);
				 amount=amplitude * sin(distance / wavelength * 2*M_PI - phase);
				 amount =amount* (radius-distance)/radius;
				 amount=amount*wavelength/(distance+0.0001);

				 new_x =x + dx*amount;
				 new_y =y + dy*amount;
			 }

			if(new_x<0)         new_x=0;
			if(new_x>=width-1)  new_x=width-2;
			if(new_y<0)         new_y=0;
			if(new_y>=height-1) new_y=height-2;

			x1=(int)new_x;
			y1=(int)new_y;

			p=new_x-x1;
			q=new_y-y1;

			for (int k=0; k<3; k++)
			{
				output.at<Vec3b>(y, x)[k]=(1-p)*(1-q)*input.at<Vec3b>(y1, x1)[k]+
										(p)*(1-q)*input.at<Vec3b>(y1,x1+1)[k]+
										(1-p)*(q)*input.at<Vec3b>(y1+1,x1)[k]+
										(p)*(q)*input.at<Vec3b>(y1+1,x1+1)[k];
			}

		}
	}
	return 0;
}

int  Filter::Pinch(InputArray src, OutputArray dst, float amount, Point center, float radiusPercent)
{
	Mat input = src.getMat();
	if( input.empty() ) {
		return -1;
	}

	dst.create(src.size(), src.type());
	Mat output = dst.getMat();


	int width = input.cols;
	int height = input.rows;

	float angle = M_PI / 2;
	float centreX = 0.5;
	float centreY = 0.5;

	float radius = std::min(width, height) * radiusPercent ;
	//float amount = 0.5;
	std::cout << amount << std::endl;

	if (radius==0)
		radius = std::min(width, height) / 2;

	float icentreX = width * centreX;
	float icentreY = height * centreY;

	if (center.x != 0 || center.y != 0) {
		icentreX = center.x;
		icentreY = center.y;
	}

	float radius2 = radius * radius;

	float dx, dy, new_x, new_y;
	float p, q, x1 ,y1;
	float distance;
	float a, d, t, s, c, e;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			 dx = x - icentreX;
			 dy = y - icentreY;

			 distance = dx*dx + dy*dy;

			 if (distance > radius2 || distance == 0)
			 {
				 new_x = x;
				 new_y = y;
			 }
			 else
			 {
				d  =  sqrt( distance / radius2 );
				t  =  pow( sin( M_PI * 0.5 * d ), -amount );

				dx  = dx* t;
				dy  = dy* t;

				e = 1 - d;
				a = angle * e * e;

				s = sin( a );
				c = cos( a );

				new_x = icentreX + c*dx - s*dy;
				new_y = icentreY + s*dx + c*dy;
			 }

			if(new_x<0)         new_x = 0;
			if(new_x>=width-1)  new_x = width - 2;
			if(new_y<0)         new_y = 0;
			if(new_y>=height-1) new_y = height - 2;

			x1=(int)new_x;
			y1=(int)new_y;

			p = new_x - x1;
			q = new_y - y1;

			for (int k = 0; k < 3; k++)
			{
				output.at<Vec3b>(y, x)[k]=(1-p)*(1-q)*input.at<Vec3b>(y1, x1)[k]+
										(p)*(1-q)*input.at<Vec3b>(y1,x1+1)[k]+
										(1-p)*(q)*input.at<Vec3b>(y1+1,x1)[k]+
										(p)*(q)*input.at<Vec3b>(y1+1,x1+1)[k];
			}

		}
	}
	return 0;
}


static float Triangle(float x)
{
    float temp_r = fmod(x, 1.0);

    if (temp_r < 0.5) {
       return 2 * temp_r;
    } else {
       return 2 * (1 - temp_r);
    }
}

int Filter::Kaleidoscope(InputArray src, OutputArray dst)
{
	Mat input = src.getMat();
	if( input.empty() ) {
		return -1;
	}

	dst.create(src.size(), src.type());
	Mat output = dst.getMat();

	//const uchar *in;
	//uchar *out;
	int width = input.cols;
	int height = input.rows;
	//int channels = input.channels();

	float angle = M_PI / 4;
	float angle2 = M_PI / 4;
	float centreX = 0.5;
	float centreY = 0.5;
	float sides = 4;

	float icentreX = width * centreX;
	float icentreY = height * centreY;
	float radius = 150;

	float dx, dy, new_x, new_y;
	float p, q, x1, y1;
	float c, r, theta, temp_theta, radius_c;

	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			 dx = j - icentreX;
			 dy = i - icentreY;

			 theta = atan2(dy, dx) - angle - angle2;
			 r = sqrt( dy * dy + dx * dx );

			 temp_theta = theta / M_PI * sides * 0.5;
			 theta = Triangle (temp_theta);

			 if (radius)
			 {
				c = cos(theta);
				radius_c = radius/c;
				r=radius_c * Triangle( r / radius_c );
			 }


			theta = theta + angle;

			new_x = r * cos(theta) + icentreX;
			new_y = r * sin(theta) + icentreY;

			if(new_x<0)         new_x = 0;
			if(new_x>=width-1)  new_x = width-2;
			if(new_y<0)         new_y = 0;
			if(new_y>=height-1) new_y = height-2;

//		    if (new_x < 0)     continue;
//		    if (new_x >= width-1)   continue;
//		    if (new_y >= height-1)  continue;
//		    if (new_y < 0)  continue;

			x1 = (int) new_x;
			y1 = (int) new_y;

			p = new_x - x1;
			q = new_y - y1;

			//in = input.ptr<uchar>(y1) + x1 * channels;;
			//out = output.ptr<uchar>(y) + x * channels;
			for (int k = 0; k < 3; ++k)
			{
				output.at<Vec3b>(i, j)[k]=
										COLOR_RANGE(
						                (1-p) * (1-q) * input.at<Vec3b>(y1, x1)[k] +
										(p) * (1-q) * input.at<Vec3b>(y1,x1+1)[k]+
										(1-p) * (q) * input.at<Vec3b>(y1+1,x1)[k]+
										(p) * (q)* input.at<Vec3b>(y1+1,x1+1)[k]
						                 );
			}

		}
	}
	return 0;
}



} /* namespace cv */
