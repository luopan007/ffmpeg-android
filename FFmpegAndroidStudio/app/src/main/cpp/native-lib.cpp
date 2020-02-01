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
Java_com_luopan_ffmpeg_MainActivity_avCodecRegisterAll(JNIEnv *env, jobject thiz) {
    avcodec_register_all();
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
    //    audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    //    videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

    // 2、打开音视频解码器
    AVCodec *vCodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id); // 寻找视频解码器
    AVCodec *aCodec = avcodec_find_decoder(ic->streams[audioStream]->codecpar->codec_id); // 寻找音频解码器
    if (!vCodec || !aCodec) {
        LOGW("视频或音频解码器创建失败。");
        return;
    }
    AVCodecContext *vc = avcodec_alloc_context3(vCodec); // 创建视频解码器内存空间
    AVCodecContext *ac = avcodec_alloc_context3(aCodec); // 创建音频解码器内存空间
    avcodec_parameters_to_context(vc, ic->streams[videoStream]->codecpar); // 把视频参数赋值给视频解码器
    avcodec_parameters_to_context(ac, ic->streams[audioStream]->codecpar); // 把音频参数赋值给音频解码器
    vc->thread_count = 1; // 视频解码采用单线程
    ac->thread_count = 1; // 音频解码采用单线程
    ret = avcodec_open2(vc, 0, 0); // 打开视频解码器
    if (ret != 0) {
        LOGW("视频解码器打开失败。");
        return;
    }
    ret = avcodec_open2(ac, 0, 0); // 打开音频解码器
    if (ret != 0) {
        LOGW("音频解码器打开失败。");
        return;
    }

    // 3、读取帧数据
    AVPacket *pkt = av_packet_alloc(); // 分配用来保存解码前的帧数据的内存空间
    AVFrame *frame = av_frame_alloc(); // 分配用来保存解码后的帧数据的内存空间
    for (; ;) {
        ret = av_read_frame(ic, pkt);
        if (ret != 0) {
            LOGW("读取文件打破结尾处,从头开始播放。");
            int seekPosition = 0;
            av_seek_frame(ic, videoStream, seekPosition, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            continue;
        }
        LOGW("stream = %d size =%d pts=%lld flag=%d",
             pkt->stream_index,
             pkt->size,
             pkt->pts,
             pkt->flags);

        AVCodecContext *cc = vc;
        if (pkt->stream_index == audioStream) {
            cc = ac;
        }
        ret = avcodec_send_packet(cc, pkt); // 把视频数据送到解码缓冲线程
        av_packet_unref(pkt); // 数据复制送完以后，可以将解码前的pkt数据进行清理
        if (ret != 0) {
            LOGW("送数据失败，直接进行下一次送数据: %s", av_err2str(ret
            ));
            continue;
        }
        ret = avcodec_receive_frame(cc, frame); // 读取解码后的帧数据
        if (ret != 0) {
            LOGW("接收数据失败，直接进行下一次接收数据。");
            continue;
        }
        LOGI("接收成功，解码后的数据的PTS = %lld.", frame->pts);
    }

    // 关闭上下文
    avformat_close_input(&ic);

    // 释放字符串空间
    env->ReleaseStringUTFChars(path, fPath);
}