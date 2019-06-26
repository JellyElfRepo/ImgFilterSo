package com.cv.effect;

import android.graphics.Bitmap;

public class NativeFilter {

    static {
        System.loadLibrary("native-lib");
    }
    public native String stringFromJNI(String string);
    public native Bitmap imgGray(Bitmap bitmap,int angle);

    //色阶
    //bitmap
    //colorChannel 颜色通道
    //shadowValue 色阶黑点值
    //midtones色阶灰点值
    //highlight 输出色阶白点值
    //output shadow 输出色阶黑点值
    //output highlight 输出色阶白点值
    public native Bitmap colorLevelsTweak(Bitmap bitmap,int colorChannel,int shadowValue,int midtones,int highlight,int outputShadow,int outputHighlight);

}
