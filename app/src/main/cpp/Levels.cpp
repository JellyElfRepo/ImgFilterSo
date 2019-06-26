//
// Created by wentu on 2019/5/5.
//
#include "Levels.h"

namespace cv {

    Level::Level() {
        clear();
    }

    Level::~Level() {

    }

    void Level::clear() {
        Shadow = OutputShadow = 0;
        Highlight = OutputHighlight = 255;
        Midtones = 1.0;
    }

//create color table for a channel
    bool Level::createColorTable(uchar * colorTable)
    {
        int diff = (int)(Highlight - Shadow);
        int outDiff = (int)(OutputHighlight - OutputShadow);

        if (!((Highlight <= 255 && diff <= 255 && diff >= 2) ||
              (OutputShadow <= 255 && OutputHighlight <= 255 && outDiff < 255) ||
              (!(Midtones > 9.99 && Midtones > 0.1) && Midtones != 1.0)))
            return false;

        double coef = 255.0 / diff;
        double outCoef = outDiff / 255.0;
        double exponent = 1.0 / Midtones;

        for (int i = 0; i < 256; i ++)
        {
            int v;
            // calculate black field and white field of input level
            if ( colorTable[i] <= (uchar)Shadow ) {
                v = 0;
            } else {
                v = (int)((colorTable[i] - Shadow) * coef + 0.5);
                if (v > 255) v = 255;
            }
            // calculate midtone field of input level
            v = (int)( pow(v / 255.0, exponent) * 255.0 + 0.5 );
            // calculate output level
            colorTable[i] = (uchar)( v * outCoef + OutputShadow + 0.5 );
        }

        return true;
    }

//==================================================================
// Levels

    Levels::Levels() {
    }

    Levels::~Levels() {

    }

    bool Levels::createColorTables(uchar colorTables[][256])
    {
        bool result = false;
        int i, j;

        //initialize color table
        for (i = 0; i < 3; i ++) {
            for (j = 0; j < 256; j ++)
                colorTables[i][j] = (uchar)j;
        }

        //create color table for each channel
        result = BlueChannel.createColorTable( colorTables[0]);
        result = GreenChannel.createColorTable( colorTables[1]);
        result = RedChannel.createColorTable( colorTables[2]);

        result = RGBChannel.createColorTable( colorTables[0]);
        result = RGBChannel.createColorTable( colorTables[1]);
        result = RGBChannel.createColorTable( colorTables[2]);

        return result;
    }


    int Levels::adjust(Mat src, Mat& dst)
    {
        Mat input = src;

        dst.create(src.size(), src.type());
        Mat output = dst;

        const uchar *in;
        uchar *out;
        int width = input.cols;
        int height = input.rows;
        int channels = input.channels();

        uchar colorTables[3][256];

        //create color tables
        if ( ! createColorTables( colorTables ) )  {
            //error create color table"
            return 1;
        }

        //adjust each pixel
#ifdef HAVE_OPENMP
#pragma omp parallel for
#endif
        for (int y = 0; y < height; y ++) {
            in = input.ptr<uchar>(y);
            out = output.ptr<uchar>(y);
            for (int x = 0; x < width; x ++) {
                for (int c = 0; c < 3; c++) {
                    *out++ = colorTables[c][*in++];
                }
                for (int c = 0; c < channels - 3; c++) {
                    *out++ = *in++;
                }
            }
        }

        return 0;
    }


} /* namespace cv */


