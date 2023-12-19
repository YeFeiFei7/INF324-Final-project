
#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>


#include "cartoon.h"
#include "ImageUtils.h"


using namespace std;
using namespace cv;

extern "C" {

JNIEXPORT void JNICALL Java_com_Cartoonifier_CartoonifierView_ShowPreview(JNIEnv* env, jobject,
        jint width, jint height, jbyteArray yuv, jintArray bgra)
{

    jbyte* _yuv  = env->GetByteArrayElements(yuv, 0);
    jint*  _bgra = env->GetIntArrayElements(bgra, 0);

    Mat myuv(height + height/2, width, CV_8UC1, (uchar *)_yuv);
    Mat mbgra(height, width, CV_8UC4, (uchar *)_bgra);

    cvtColor(myuv, mbgra, CV_YUV420sp2BGRA);

    env->ReleaseIntArrayElements(bgra, _bgra, 0);
    env->ReleaseByteArrayElements(yuv, _yuv, 0);
}


DECLARE_TIMING(CartoonifyImage);

JNIEXPORT void JNICALL Java_com_Cartoonifier_CartoonifierView_CartoonifyImage(JNIEnv* env, jobject,
        jint width, jint height, jbyteArray yuv, jintArray bgra,
        jboolean sketchMode, jboolean alienMode, jboolean evilMode, jboolean debugMode)
{
    START_TIMING(CartoonifyImage);

    jbyte* _yuv  = env->GetByteArrayElements(yuv, 0);
    jint*  _bgra = env->GetIntArrayElements(bgra, 0);

    Mat myuv(height + height/2, width, CV_8UC1, (unsigned char *)_yuv);
    Mat mgray(height, width, CV_8UC1, (unsigned char *)_yuv);

    Mat mbgra(height, width, CV_8UC4, (unsigned char *)_bgra);

    Mat mbgr(height, width, CV_8UC3);
    cvtColor(myuv, mbgr, CV_YUV420sp2BGR);

    Mat displayedFrame(mbgra.size(), CV_8UC3);

    int debugType = 0;
    if (debugMode)
        debugType = 1;

    cartoonifyImage(mbgr, displayedFrame, sketchMode, alienMode, evilMode, debugType);

    cvtColor(displayedFrame, mbgra, CV_BGR2BGRA);

    env->ReleaseIntArrayElements(bgra, _bgra, 0);
    env->ReleaseByteArrayElements(yuv, _yuv, 0);

    STOP_TIMING(CartoonifyImage);

    SHOW_TIMING(CartoonifyImage, "CartoonifyImage");
}



}
