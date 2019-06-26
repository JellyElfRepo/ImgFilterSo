//
// Created by wentu on 2019/5/6.
//

#ifndef IMAGEPROCESSAPP_NATIVE_LIB_HPP
#define IMAGEPROCESSAPP_NATIVE_LIB_HPP
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "error", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "debug", __VA_ARGS__))
#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <android/bitmap.h>
#include <android/log.h>

using namespace cv;

class nativeUtil{
public:
    static void BitmapToMat(JNIEnv *env, jobject& bitmap, Mat& mat, jboolean needUnPremultiplyAlpha) {
        AndroidBitmapInfo info;
        void *pixels = 0;
        Mat &dst = mat;

        try {
            LOGD("nBitmapToMat");
            CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
            CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                      info.format == ANDROID_BITMAP_FORMAT_RGB_565);
            CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
            CV_Assert(pixels);
            dst.create(info.height, info.width, CV_8UC4);
            if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
                LOGD("nBitmapToMat: RGBA_8888 -> CV_8UC4");
                Mat tmp(info.height, info.width, CV_8UC4, pixels);
                if (needUnPremultiplyAlpha) cvtColor(tmp, dst, COLOR_mRGBA2RGBA);
                else tmp.copyTo(dst);
            } else {
                // info.format == ANDROID_BITMAP_FORMAT_RGB_565
                LOGD("nBitmapToMat: RGB_565 -> CV_8UC4");
                Mat tmp(info.height, info.width, CV_8UC2, pixels);
                cvtColor(tmp, dst, COLOR_BGR5652RGBA);
            }
            AndroidBitmap_unlockPixels(env, bitmap);
            return;
        } catch (const cv::Exception &e) {
            AndroidBitmap_unlockPixels(env, bitmap);
            LOGE("nBitmapToMat catched cv::Exception: %s", e.what());
            jclass je = env->FindClass("org/opencv/core/CvException");
            if (!je) je = env->FindClass("java/lang/Exception");
            env->ThrowNew(je, e.what());
            return;
        } catch (...) {
            AndroidBitmap_unlockPixels(env, bitmap);
            LOGE("nBitmapToMat catched unknown exception (...)");
            jclass je = env->FindClass("java/lang/Exception");
            env->ThrowNew(je, "Unknown exception in JNI code {nBitmapToMat}");
            return;
        }
    }//bitmap to mat
    static jobject generateBitmap(JNIEnv *env, uint32_t width, uint32_t height) {

        jclass bitmapCls = env->FindClass("android/graphics/Bitmap");
        jmethodID createBitmapFunction = env->GetStaticMethodID(bitmapCls,
                                                                "createBitmap",
                                                                "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
        jstring configName = env->NewStringUTF("ARGB_8888");
        jclass bitmapConfigClass = env->FindClass("android/graphics/Bitmap$Config");
        jmethodID valueOfBitmapConfigFunction = env->GetStaticMethodID(
                bitmapConfigClass, "valueOf",
                "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");

        jobject bitmapConfig = env->CallStaticObjectMethod(bitmapConfigClass,
                                                           valueOfBitmapConfigFunction, configName);

        jobject newBitmap = env->CallStaticObjectMethod(bitmapCls,
                                                        createBitmapFunction,
                                                        width,
                                                        height, bitmapConfig);
        return newBitmap;
    }
    static jobject mat_to_bitmap(JNIEnv * env, Mat & src,jobject srcBitmap, bool needPremultiplyAlpha){

        //找到android/graphics/Bitmap 类
        jclass java_bitmap_class = (jclass)env->FindClass("android/graphics/Bitmap");
        jmethodID _mid = env->GetMethodID(java_bitmap_class, "getConfig", "()Landroid/graphics/Bitmap$Config;");
        jobject bitmap_config = env->CallObjectMethod(srcBitmap, _mid);


        //找到Bitmap类的中的静态方法createBitmap
        jmethodID mid = env->GetStaticMethodID(java_bitmap_class,
                                               "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");

        //调用
        jobject bitmap = env->CallStaticObjectMethod(java_bitmap_class,
                                                     mid, src.size().width, src.size().height, bitmap_config);
        AndroidBitmapInfo  info;
        void* pixels = 0;

        try {
            //validate
            CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
            CV_Assert(src.type() == CV_8UC1 || src.type() == CV_8UC3 || src.type() == CV_8UC4);
            CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
            CV_Assert(pixels);

            //type mat
            if(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888){
                Mat tmp(info.height, info.width, CV_8UC4, pixels);
                if(src.type() == CV_8UC1){
                    cvtColor(src, tmp, CV_GRAY2RGBA);
                } else if(src.type() == CV_8UC3){
                    cvtColor(src, tmp, CV_RGB2RGBA);
                } else if(src.type() == CV_8UC4){
                    if(needPremultiplyAlpha){
                        cvtColor(src, tmp, COLOR_RGBA2mRGBA);
                    }else{
                        src.copyTo(tmp);
                    }
                }

            } else{
                Mat tmp(info.height, info.width, CV_8UC2, pixels);
                if(src.type() == CV_8UC1){
                    cvtColor(src, tmp, CV_GRAY2BGR565);
                } else if(src.type() == CV_8UC3){
                    cvtColor(src, tmp, CV_RGB2BGR565);
                } else if(src.type() == CV_8UC4){
                    cvtColor(src, tmp, CV_RGBA2BGR565);
                }
            }
            AndroidBitmap_unlockPixels(env, bitmap);
            return bitmap;
        } catch(cv::Exception e){
            AndroidBitmap_unlockPixels(env, bitmap);
            jclass je = env->FindClass("org/opencv/core/CvException");
            if(!je) je = env->FindClass("java/lang/Exception");
            env->ThrowNew(je, e.what());
            return bitmap;
        } catch (...){
            AndroidBitmap_unlockPixels(env, bitmap);
            jclass je = env->FindClass("java/lang/Exception");
            env->ThrowNew(je, "Unknown exception in JNI code {nMatToBitmap}");
            return bitmap;
        }
    }
};



#endif //IMAGEPROCESSAPP_NATIVE_LIB_HPP
