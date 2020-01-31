#include <jni.h>
#include <string>
#include <android/log.h>

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
}

/**
 * 把分数转换为小数，防止除零
 *
 * @param rational 待转换的分数
 * @return 转换后的小数
 */
static double r2d(AVRational rational){
    return rational.num == 0 || rational.den == 0 ? 0.0 : (double) rational.num / (double)rational.den;
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_luopan_ffmpeg_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    hello += avcodec_configuration();
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_MainActivity_avRegisterAll(JNIEnv *env, jobject thiz) {
    av_register_all();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_MainActivity_avFormatNetworkInit(JNIEnv *env, jobject thiz) {
    avformat_network_init();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_MainActivity_avFormatOpenInput(JNIEnv *env, jobject thiz, jstring path) {
    const char *fPath = env->GetStringUTFChars(path, 0);

    AVFormatContext *ic = NULL;
    int ret = avformat_open_input(&ic, fPath, 0, 0);
    if (ret != 0) {
        LOGW("avformat_open_input %s failed!", av_err2str(ret));
        return;
    }
    LOGI("avformat_open_input %s success!", fPath);
    LOGI("duration = %lld, nb_streams = %d.", ic->duration, ic->nb_streams);

    // 对于部门没有头部信息的码流，需要使用探测函数
    ret = avformat_find_stream_info(ic, 0);
    if (ret != 0) {
        LOGW("avformat_find_stream_info %s failed!", av_err2str(ret));
        return;
    }
    LOGI("avformat_find_stream_info %s success!", fPath);
    LOGI("duration = %lld, nb_streams = %d.", ic->duration, ic->nb_streams);

    // 1、遍历寻找音视频流
    int fps = 0;
    int videoStream = 0;
    int audioStream = 0;
    for (int i = 0; i < ic->nb_streams; ++i) {
        AVStream *as = ic->streams[i];
        if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            LOGI("视频数据");
            videoStream = i;
            fps = r2d(as->avg_frame_rate);
            LOGI("fps = %d, width=%d, height=%d, code id = %d, pixformat = %d.",fps,
                    as->codecpar->width,
                    as->codecpar->height,
                    as->codecpar->codec_id,
                    as->codecpar->format);
        } else if (as->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            LOGI("音频数据");
            audioStream = i;
            LOGI("sample_rate=%d, channel=%d, samplae_format=%d",
                    as->codecpar->sample_rate,
                    as->codecpar->channels,
                    as->codecpar->format);
        } else {
            LOGI("其他数据");

        }
    }

    // 2、使用函数寻找音视频流
    audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    AVStream *audio = ic->streams[audioStream];
    LOGI("sample_rate=%d, channel=%d, samplae_format=%d",
         audio->codecpar->sample_rate,
         audio->codecpar->channels,
         audio->codecpar->format);

    videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    AVStream *video = ic->streams[videoStream];
    fps = r2d(video->avg_frame_rate);
    LOGI("fps = %d, width=%d, height=%d, code id = %d, pixformat = %d.",fps,
         video->codecpar->width,
         video->codecpar->height,
         video->codecpar->codec_id,
         video->codecpar->format);

    // 3、读取帧数据



    avformat_close_input(&ic);
    env->ReleaseStringUTFChars(path, fPath);
}