/*
 * BlackWhite Adjust
 *
 * Author: JoStudio
 */

#include "BlackWhite.hpp"

#define SWAP(a, b, t)  do { t = a; a = b; b = t; } while(0)
#define CLIP_RANGE(value, min, max)  ( (value) > (max) ? (max) : (((value) < (min)) ? (min) : (value)) )
#define COLOR_RANGE(value)  CLIP_RANGE(value, 0, 255)

//color index
typedef enum COLOR_INDEX {
	INDEX_RED,
	INDEX_YELLOW,
	INDEX_GREEN,
	INDEX_CYAN,
	INDEX_BLUE,
	INDEX_MAGENTA
} color_index_t;

namespace cv {

BlackWhite::BlackWhite()
{
	//set to default settings
	red = 0.4;
	yellow = 0.6;
	green = 0.4;
	cyan = 0.6;
	blue = 0.2;
	magenta = 0.8;
}

BlackWhite::~BlackWhite() {
}


int BlackWhite::adjust(InputArray src, OutputArray dst)
{
	Mat input = src.getMat();
	if( input.empty() ) {
		return -1;
	}

	dst.create(src.size(), src.type());
	Mat output = dst.getMat();

	int blackWhiteParams[6];
	blackWhiteParams[0] = CLIP_RANGE(red     * 100, -100, 100);
	blackWhiteParams[1] = CLIP_RANGE(yellow  * 100, -100, 100);
	blackWhiteParams[2] = CLIP_RANGE(green   * 100, -100, 100);
	blackWhiteParams[3] = CLIP_RANGE(cyan    * 100, -100, 100);
	blackWhiteParams[4] = CLIP_RANGE(blue    * 100, -100, 100);
	blackWhiteParams[5] = CLIP_RANGE(magenta * 100, -100, 100);

	const uchar *in;
	uchar *out;
	int channels  = input.channels();
	int rows = input.rows;
	int cols = input.cols;
	uchar gray;

	int tmp;
	int values[3];
	int indexes[3];
	int ratio_max;
	int ratio_max_mid;

	for (int y = 0; y < rows; y ++ )
	{
		in = input.ptr<uchar>(y);
		out = output.ptr<uchar>(y);
		for (int x = 0; x < cols; x ++)
		{
			//read RGB into values, set index in indexes.
			values[0] = in[0]; values[1] = in[1]; values[2] = in[2];
			indexes[0]=INDEX_BLUE; indexes[1]=INDEX_GREEN; indexes[2]=INDEX_RED;

			//sort values and indexes
			if ( values[1] > values[0] ) {
				SWAP(values[0], values[1], tmp);
				SWAP(indexes[0], indexes[1], tmp);
			}

			if ( values[2] > values[1] ) {
				SWAP(values[1], values[2], tmp);
				SWAP(indexes[1], indexes[2], tmp);
			}

			if ( values[1] > values[0] ) {
				SWAP(values[0], values[1], tmp);
				SWAP(indexes[0], indexes[1], tmp);
			}

			//get ratio_max
			ratio_max = blackWhiteParams[ indexes[0] ];

			//calculate ratio_max_mid
			if ( indexes[0] == INDEX_RED ) {
				tmp = (indexes[1] == INDEX_GREEN) ? INDEX_YELLOW : INDEX_CYAN;
			} else  if ( indexes[0] == INDEX_GREEN ) {
				tmp = (indexes[1] == INDEX_RED) ? INDEX_YELLOW : INDEX_CYAN ;
			} else {
				tmp = (indexes[1] == INDEX_RED) ? INDEX_MAGENTA : INDEX_CYAN;
			}
			ratio_max_mid = blackWhiteParams[ tmp ];

			//calculate gray =  (max - mid) * ratio_max + (mid - min) * ratio_max_mid + min
			gray = COLOR_RANGE ( (
					(values[0] - values[1]) * ratio_max +
					(values[1] - values[2]) * ratio_max_mid + values[2] * 100
					) / 100 );

			//save to ouput
			*out++ = gray;
			*out++ = gray;
			*out++ = gray;

			//move pointer forward
			in += 3;
			for (int j = 0; j < channels - 3; j++) {
				*out++ = *in++;
			}
		}
	}

	return 0;

}

} /* namespace cv */
