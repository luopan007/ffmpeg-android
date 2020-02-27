#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window.h> // Android NDK绘制GLSurfaceView
#include <android/native_window_jni.h>
#include <SLES/OpenSLES.h> // 音频数据播放头文件
#include <SLES/OpenSLES_Android.h>

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

extern "C"
JNIEXPORT
jint JNI_OnLoad(JavaVM *vm, void *res) {
    av_jni_set_java_vm(vm, 0); // 硬解码需要设置Java虚拟机给FFmpeg
    return JNI_VERSION_1_4;
}

/**
 * 把分数转换为小数，防止除零
 *
 * @param rational 待转换的分数
 * @return 转换后的小数
 */
static double r2d(AVRational rational) {
    return rational.num == 0 || rational.den == 0 ? 0.0 : (double) rational.num /
                                                          (double) rational.den;
}

/**
 * 获取当前时间
 *
 * @return 当前时间，毫秒为单位
 */
long long getNowTimeMs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int sec = tv.tv_sec % 36000;
    long long millisecond = sec * 1000 + tv.tv_usec / 1000;
    return millisecond;
}

static SLObjectItf engineSL = NULL;

/**
 * 创建引擎
 *
 * @return 音频引擎
 */
SLEngineItf CreateSL() {
    SLresult re;
    SLEngineItf en;
    re = slCreateEngine(&engineSL, 0, 0, 0, 0, 0);
    if (re != SL_RESULT_SUCCESS) return NULL;
    re = (*engineSL)->Realize(engineSL, SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS) return NULL;
    re = (*engineSL)->GetInterface(engineSL, SL_IID_ENGINE, &en);
    if (re != SL_RESULT_SUCCESS) return NULL;
    return en;
};

/**
 * 回调函数
 *
 * @param bf
 * @param context
 */
void PcmCall(SLAndroidSimpleBufferQueueItf bf, void *context) {
    LOGI("PcmCall IN");
    static FILE *fp = NULL;
    static char *buf = NULL;
    if (!buf) {
        buf = new char[1024 * 1024];
    }
    if (!fp) {
        fp = fopen("/sdcard/test.pcm", "rb");
    }
    if (!fp) {
        LOGW("打开音频文件失败");
        return;
    }
    if (feof(fp) == 0) { // 等于0，表示不再文件结尾
        int len = fread(buf, 1, 1024, fp);
        if (len > 0) {
            (*bf)->Enqueue(bf, buf, len); // 发送音频数据
        }
    }
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
}

extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_XPlay_open(JNIEnv *env, jobject thiz, jstring path, jobject surface) {
    const char *fPath = env->GetStringUTFChars(path, 0);
    AVFormatContext *ic = NULL;
    int ret = avformat_open_input(&ic, fPath, 0, 0);
    if (ret != 0) {
        LOGW("avformat_open_input %s failed!", av_err2str(ret));
        return;
    }
    // 对于部门没有头部信息的码流，需要使用探测函数
    ret = avformat_find_stream_info(ic, 0);
    if (ret != 0) {
        LOGW("avformat_find_stream_info %s failed!", av_err2str(ret));
        return;
    }

    // 1、使用函数寻找音视频流
    int audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    int videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

    // 2、打开音视频解码器
    AVCodec *vCodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id); // 寻找视频解码器
    AVCodec *aCodec = avcodec_find_decoder(ic->streams[audioStream]->codecpar->codec_id); // 寻找音频解码器
    if (!vCodec || !aCodec) {
        LOGW("视频或音频解码器创建失败。");
        return;
    }

    // 创建音频解码引擎
    SLEngineItf eng = CreateSL();
    if (eng) {
        LOGI("创建引擎成功");
    } else {
        LOGW("创建引擎失败");
    }

    // 创建混音器
    SLresult re = 0;
    SLObjectItf mix = NULL;
    re = (*eng)->CreateOutputMix(eng, &mix, 0, 0, 0);
    if (re != SL_RESULT_SUCCESS) {
        LOGW("CreateOutputMix fail")
    }
    re = (*mix)->Realize(mix, SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS) {
        LOGW("Realize fail");
    }

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, mix}; // 直接给结构体赋值
    SLDataSink audioSink = {&outputMix, 0};

    // 配置音频信息
    SLDataLocator_AndroidSimpleBufferQueue que = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                  10}; //缓冲队列
    SLDataFormat_PCM pcmData = {SL_DATAFORMAT_PCM, // 数据格式为PCM
                                2, // 声道数
                                SL_SAMPLINGRATE_44_1, // 采样率
                                SL_PCMSAMPLEFORMAT_FIXED_16, // 位宽
                                SL_PCMSAMPLEFORMAT_FIXED_16,
                                SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, // 前左和前右输出
                                SL_BYTEORDER_LITTLEENDIAN // 字节序，小端
    };
    SLDataSource ds = {&que, &pcmData}; // 参数福给结构体，供播放器使用

    // 创建播放器
    SLObjectItf player = NULL;
    const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE};
    re = (*eng)->CreateAudioPlayer(eng, &player, &ds, &audioSink,
                                   sizeof(ids) / sizeof(SLInterfaceID), ids, req);
    if (re != SL_RESULT_SUCCESS) {
        LOGW("创建播放器失败");
    } else {
        LOGI("创建播放器成功");
    }

    // 获取player接口
    SLPlayItf iplayer;
    (*player)->Realize(player, SL_BOOLEAN_FALSE);
    re = (*player)->GetInterface(player, SL_IID_PLAY, &iplayer);
    if (re != SL_RESULT_SUCCESS) {
        LOGW("获取接口失败");
    }

    // 获取队列接口
    SLAndroidSimpleBufferQueueItf pcmQue = NULL;
    re = (*player)->GetInterface(player, SL_IID_BUFFERQUEUE, &pcmQue);
    if (re != SL_RESULT_SUCCESS) {
        LOGW("获取队列失败");
    }

    // 设置回调函数，播放队列函数空调用
    (*pcmQue)->RegisterCallback(pcmQue, PcmCall, 0);
    (*iplayer)->SetPlayState(iplayer, SL_PLAYSTATE_PLAYING); // 设置播放状态

    // 启动队列
    (*pcmQue)->Enqueue(pcmQue, "", 1);


    AVCodecContext *vc = avcodec_alloc_context3(vCodec); // 创建视频解码器内存空间
    AVCodecContext *ac = avcodec_alloc_context3(aCodec); // 创建音频解码器内存空间
    avcodec_parameters_to_context(vc, ic->streams[videoStream]->codecpar); // 把视频参数赋值给视频解码器
    avcodec_parameters_to_context(ac, ic->streams[audioStream]->codecpar); // 把音频参数赋值给音频解码器
    vc->thread_count = 4; // 视频解码采用单线程，改成4线程测试解码性能
    ac->thread_count = 4; // 音频解码采用单线程，改成4线程测试解码性能
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

    // 初始化像素转换的上下文对象
    SwsContext *vctx = NULL;
    int outWidth = 1920;
    int outHeight = 1080;
    char *rgb = new char[outWidth * outHeight * 4];

    // 初始化音频数据重采样
    SwrContext *actx = swr_alloc();
    actx = swr_alloc_set_opts(actx, av_get_default_channel_layout(2), AV_SAMPLE_FMT_S16,
                              ac->sample_rate, av_get_default_channel_layout(ac->channels),
                              ac->sample_fmt, ac->sample_rate, 0, 0);
    char *pcm = new char[48000 * 4 * 2];
    ret = swr_init(actx);
    if (ret != 0) {
        LOGW("swr_init failed.");
    } else {
        LOGI("swr_init success.");
    }

    // 显示窗口初始化
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface); // 通过surface获取得到窗口
    ANativeWindow_setBuffersGeometry(nwin, outWidth, outHeight,
                                     WINDOW_FORMAT_RGBA_8888); // 设置窗口的尺寸和格式
    ANativeWindow_Buffer wBuf; // 创建窗口缓存

    for (;;) {
        ret = av_read_frame(ic, pkt);
        if (ret != 0) {
            int seekPosition = 0; // 从头开始
            av_seek_frame(ic, videoStream, seekPosition, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            continue;
        }
        AVCodecContext *cc = vc;
        if (pkt->stream_index == audioStream) {
            cc = ac;
        }
        ret = avcodec_send_packet(cc, pkt); // 把视频数据送到解码缓冲线程
        av_packet_unref(pkt); // 数据复制送完以后，可以将解码前的pkt数据进行清理
        if (ret != 0) {
            continue;
        }
        ret = avcodec_receive_frame(cc, frame); // 读取解码后的帧数据
        if (ret != 0) {
            continue;
        }
        if (cc == vc) {
            vctx = sws_getCachedContext(vctx, frame->width, frame->height,
                                        (AVPixelFormat) (frame->format),
                                        outWidth, outHeight, AV_PIX_FMT_RGBA, SWS_FAST_BILINEAR, 0,
                                        0, 0); // 解码成功以后获取这个格式
            if (!vctx) {
                LOGD("sws_getCachedContext is failed.");
            } else {
                uint8_t *data[AV_NUM_DATA_POINTERS] = {0};
                data[0] = (uint8_t *) (rgb);
                int lines[AV_NUM_DATA_POINTERS] = {0};
                lines[0] = outWidth * 4;
                int h = sws_scale(vctx,
                                  (const uint8_t **) frame->data,
                                  frame->linesize, 0,
                                  frame->height,
                                  data, lines);
                if (h > 0) {
                    ANativeWindow_lock(nwin, &wBuf, 0); // 锁住窗口及其缓存
                    uint8_t *dst = (uint8_t *) (wBuf.bits); // 取出窗口的缓存的数据地址
                    memcpy(dst, rgb, outWidth * outHeight * 4); // 拷贝数据从rgb到dst
                    ANativeWindow_unlockAndPost(nwin); // 解锁窗口，并通知
                }
            }
        } else {
            uint8_t *out[2] = {0};
            out[0] = (uint8_t *) (pcm);
            // 音频重采样的上下文
            int len = swr_convert(actx, out, frame->nb_samples,
                                  (const uint8_t **) (frame->data),
                                  frame->nb_samples);
        }
    }

    delete rgb;
    delete pcm;

    // 关闭上下文
    avformat_close_input(&ic);

    // 释放字符串空间
    env->ReleaseStringUTFChars(path, fPath);
}