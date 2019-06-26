/*
我用OpenCV 编写了一个 SelectiveColor　类，实现可选颜色调整。在源文件 SelectiveColor.hpp,  SelectiveColor.cpp中

使用方法： SelectiveColor类有一个属性colors[9]， 定义了９个颜色通道（红、黄、绿、青、蓝、洋红、白、中性色、黑）。

每个颜色通道有cyan, magenta, yellow, black　四个值。设置好所需通道和值，再调用SelectiveColor类的adjust()方法即可对图像进行  可选颜色调整。

 */

#include "SelectiveColor.hpp"


#define DEBUG

#ifndef DEBUG

#define DEBUG_PRINT(a)
#define PRINT_VAR(var)
#define PRINT_VAR1(var)
#define PRINT_VARF(var)

#else

#include <iostream>
using namespace std;
#define DEBUG_PRINT(a)  cout << (a) << endl
#define PRINT_VAR(var)  cout << #var << " = " << (var) <<  endl
#define PRINT_VAR1(var) if ( nn == 0 ) cout << #var << " = " << int(var) <<  endl
#define PRINT_VARF(var) if ( nn == 0 ) cout << #var << " = " << double(var) <<  endl

#endif


#define SWAP(a, b, t)  do { t = a; a = b; b = t; } while(0)
#define CLIP_RANGE(value, min, max)  ( (value) > (max) ? (max) : (((value) < (min)) ? (min) : (value)) )
#define COLOR_RANGE(value)  CLIP_RANGE(value, 0, 255)

#define MAX2(a, b) ( (a) > (b) ) ?  (a) : (b)
#define MAX3(a,b,c) ( ( a > b ) ?  MAX2(a,c) : MAX2(b,c) )
#define BLUE  0
#define GREEN 1
#define RED   2


namespace cv {


SelectiveColorAdjust:: SelectiveColorAdjust()
{
	cyan = 1.0;
	magenta = 1.0;
	yellow = 1.0;
	black = 1.0;
	defined = false;
}

SelectiveColorAdjust::~SelectiveColorAdjust()
{

}

void SelectiveColorAdjust::calcDefined()
{
	if (cyan != 1.0 || magenta != 1.0 || yellow != 1.0 || black != 1.0 ) {
		defined =  true;
		return;
	}
	defined = false;
}

//=========================================================
// SelectiveColor

SelectiveColor::SelectiveColor() {
	isAbsolute = false;
}

SelectiveColor::~SelectiveColor() {
}

int SelectiveColor::adjust(InputArray src, OutputArray dst)
{

	Mat input = src.getMat();
	if( input.empty() ) {
		return -1;
	}

	dst.create(src.size(), src.type());
	Mat output = dst.getMat();

	//calculate color adjustment is defined
	for(int i = 0; i < 9; i++ ) {
		colors[i].calcDefined();
	}

	const uchar *in;
	uchar *out;
	int width = input.cols;
	int height = input.rows;
	int channels = input.channels();
	int nn = 0;
	uchar t;  //temp
	uchar sorted[3];      //RGB value sorted
	uchar c[SELECT_BLACK + 1];
	float delta[3];  //delta of RGB value
	float ratio[3],  ratio_positive[3], ratio_negative[3];

	for (int h = 0; h < height; h ++, nn++) {
		in = input.ptr<uchar>(h);  //pointer to input image data
		out = output.ptr<uchar>(h); //pointer to output image data

		for (int w = 0; w < width; w ++) {
			//Sort RGB values:  sorted[0] is biggest, sorted[1] is middle, sorted[2] is smallest
			memcpy(sorted, in, 3);
			if (sorted[1] > sorted[0]) SWAP(sorted[0], sorted[1], t);
			if (sorted[2] > sorted[1]) SWAP(sorted[1], sorted[2], t);
			if (sorted[1] > sorted[0]) SWAP(sorted[0], sorted[1], t);

			//calculation c[] array
			memset(c, sizeof(c), 0);
			c[SELECT_BLUE] = in[BLUE];
			c[SELECT_GREEN] = in[GREEN];
			c[SELECT_RED] = in[RED];

			//subtract the smallest value from the RGB
			c[SELECT_BLUE]  -= sorted[2];
			c[SELECT_GREEN] -= sorted[2];
			c[SELECT_RED]   -= sorted[2];

			//calculate WHIT, MIDDLE, BLACK
			if ( sorted[2] <= 127 ) {
				c[SELECT_WHITE] = 0;
				c[SELECT_MIDDLE] = sorted[2] * 2;
				c[SELECT_BLACK]  = 255 - sorted[2] * 2;
			} else  {
				c[SELECT_WHITE]  = sorted[2] * 2 - 255;
				c[SELECT_MIDDLE] = 255 - (  sorted[2] - 127.5 ) * 2;
				c[SELECT_BLACK] = 0;
			}

			//calculate YELLOW
			if ( c[SELECT_RED] > 0 && c[SELECT_GREEN] > 0 ) {
				c[SELECT_YELLOW] = ( c[SELECT_RED] > c[SELECT_GREEN] ) ?  c[SELECT_GREEN] : c[SELECT_RED];
				c[SELECT_GREEN] -= c[SELECT_YELLOW];
				c[SELECT_RED]   -= c[SELECT_YELLOW];
			};

			//calculate CYAN
			if ( c[SELECT_BLUE] > 0 && c[SELECT_GREEN] > 0 ) {
				c[SELECT_CYAN] = ( c[SELECT_BLUE] > c[SELECT_GREEN] ) ?  c[SELECT_GREEN] : c[SELECT_BLUE];
				c[SELECT_GREEN] -= c[SELECT_CYAN];
				c[SELECT_BLUE]  -= c[SELECT_CYAN];
			};

			//calculate MAGENTA
			if ( c[SELECT_BLUE] > 0 && c[SELECT_RED] > 0 ) {
				c[SELECT_MAGENTA] = ( c[SELECT_BLUE] > c[SELECT_RED] ) ?  c[SELECT_RED] : c[SELECT_BLUE];
				c[SELECT_RED]   -= c[SELECT_MAGENTA];
				c[SELECT_BLUE]  -= c[SELECT_MAGENTA];
			};

			//initialize delta[]
			delta[BLUE] = delta[GREEN] = delta[RED] = 0;

			//initialize ratios
			for(int i = 0; i < 3 ; i++ ) {
				ratio_positive[i] = in[i] / 255.0;
				ratio_negative[i] = ratio_positive[i] - 1 ;
			}

			//calculate each selective color
			for (int j = 0; j <= SELECT_BLACK; j++ ) {
				if ( colors[j].defined && (c[j] > 0) ) {
					if ( isAbsolute ) {
						ratio[RED] = colors[j].cyan + colors[j].black;
						ratio[RED] = CLIP_RANGE(ratio[RED], ratio_negative[RED], ratio_positive[RED]);
						ratio[GREEN] = colors[j].magenta + colors[j].black;
						ratio[GREEN] = CLIP_RANGE(ratio[GREEN], ratio_negative[GREEN], ratio_positive[GREEN]);
						ratio[BLUE] = colors[j].yellow + colors[j].black;
						ratio[BLUE] = CLIP_RANGE(ratio[BLUE], ratio_negative[BLUE], ratio_positive[BLUE]);
					} else {
						ratio[RED] = colors[j].cyan + ::abs(colors[j].cyan + 1) * colors[j].black;
						ratio[RED] = CLIP_RANGE(ratio[RED], -1, 1);
						ratio[RED] = ( ratio[RED] > 0 ) ? ratio[RED] * ratio_positive[RED] : - ratio[RED] * ratio_negative[RED];

						ratio[GREEN] = colors[j].magenta + ::abs(colors[j].magenta + 1) * colors[j].black;
						ratio[GREEN] = CLIP_RANGE(ratio[GREEN], -1, 1);
						ratio[GREEN] = ( ratio[GREEN] > 0 ) ? ratio[GREEN] * ratio_positive[GREEN] : - ratio[GREEN] * ratio_negative[GREEN];

						ratio[BLUE] = colors[j].yellow + ::abs(colors[j].yellow + 1) * colors[j].black;
						ratio[BLUE] = CLIP_RANGE(ratio[BLUE], -1, 1);
						ratio[BLUE] = ( ratio[BLUE] > 0 ) ? ratio[BLUE] * ratio_positive[BLUE] : - ratio[BLUE] * ratio_negative[BLUE];
					}
					delta[RED] -=  c[j] * ratio[RED];
					delta[GREEN] -=  c[j] * ratio[GREEN];
					delta[BLUE] -=  c[j] * ratio[BLUE];
				}
			}

			//save to output
			out[BLUE]  = COLOR_RANGE( in[BLUE] + delta[BLUE] );
			out[GREEN] = COLOR_RANGE( in[GREEN] + delta[GREEN] );
			out[RED]   = COLOR_RANGE( in[RED] + delta[RED] );

			//move pointer forward
			in += 3;
			out += 3;
			for (int j = 0; j < channels - 3; j++) {
				*out++ = *in++;
			}
		}
	}

	return 0;
}


} /* namespace cv */
