#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window.h> // Android NDK绘制GLSurfaceView
#include <android/native_window_jni.h>
#include <SLES/OpenSLES.h> // 音频数据播放头文件
#include <SLES/OpenSLES_Android.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

// 定义打印的宏
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"testffmepg", __VA_ARGS__);
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,"testffmepg", __VA_ARGS__);
#define LOGE(...) __android_log_print(ANDROID_LOG_FATAL,"testffmepg", __VA_ARGS__);
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,"testffmepg", __VA_ARGS__);

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
#include <libavcodec/jni.h> // 硬解码的必须的头文件
#include <libswscale/swscale.h> // 像素格式转换的头文件
#include <libswresample/swresample.h> // 音频数据重采样的头文件
}

