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

//顶点着色器glsl
#define GET_STR(x) #x
static const char *vertexShader = GET_STR(
        attribute vec4 aPosition; //顶点坐标
        attribute vec2 aTexCoord; //材质顶点坐标
        varying vec2 vTexCoord;   //输出的材质坐标
        void main(){
            vTexCoord = vec2(aTexCoord.x,1.0-aTexCoord.y);
            gl_Position = aPosition;
        }
);

//片元着色器,软解码和部分x86硬解码
static const char *fragYUV420P = GET_STR(
        precision mediump float;    //精度
        varying vec2 vTexCoord;     //顶点着色器传递的坐标
        uniform sampler2D yTexture; //输入的材质（不透明灰度，单像素）
        uniform sampler2D uTexture;
        uniform sampler2D vTexture;
        void main(){
            vec3 yuv;
            vec3 rgb;
            yuv.r = texture2D(yTexture,vTexCoord).r;
            yuv.g = texture2D(uTexture,vTexCoord).r - 0.5;
            yuv.b = texture2D(vTexture,vTexCoord).r - 0.5;
            rgb = mat3(1.0,     1.0,    1.0,
                       0.0,-0.39465,2.03211,
                       1.13983,-0.58060,0.0)*yuv;
            //输出像素颜色
            gl_FragColor = vec4(rgb,1.0);
        }
);

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

/**
 * 初始化FFMpeg运行所需的换进
 */
void initFfmpeg() {
    av_register_all();
    avformat_network_init();
    avcodec_register_all();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_XPlay_open(JNIEnv *env, jobject thiz, jstring path, jobject surface) {
    initFfmpeg();
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


GLint InitShader(const char *code, GLint type) {
    //创建shader
    GLint sh = glCreateShader(type);
    if (sh == 0) {
        LOGD("glCreateShader %d failed!", type);
        return 0;
    }
    //加载shader
    glShaderSource(sh,
                   1,    //shader数量
                   &code, //shader代码
                   0);   //代码长度
    //编译shader
    glCompileShader(sh);

    //获取编译情况
    GLint status;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        LOGD("glCompileShader failed!");
        return 0;
    }
    LOGD("glCompileShader success!");
    return sh;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_XPlay_playYuv(JNIEnv *env, jobject thiz, jstring path,
                                     jobject android_surface) {
    const char *fPath = env->GetStringUTFChars(path, 0);
    LOGI("path = %s", fPath);
    FILE *fp = fopen(fPath, "rb");
    if (!fp) {
        LOGD("open file %s failed!", fPath);
        return;
    }

    // 1、获取原始窗口
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, android_surface);

    // 2、创建display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        LOGW("eglGetDisplay 获取失败");
        return;
    }
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        LOGW("eglInitialize 初始化失败");
        return;
    }

    // 3、创建surface
    EGLConfig config; // 输出配置项
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE
    }; // 输入配置项
    if (EGL_TRUE != eglChooseConfig(display, configSpec, &config, 1, &configNum)) {
        LOGW("eglChooseConfig 配置surface失败");
        return;
    };
    EGLSurface winSurface = eglCreateWindowSurface(display, config, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        LOGW("eglCreateWindowSurface 创建失败");
        return;
    }

    // 4、创建关联的上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    }; // 使用数组传递参数，EGL_NONE表示数组结束
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        LOGW("eglCreateContext 创建失败");
        return;
    }

    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) { // 真正的关联
        LOGW("eglMakeCurrent 关联失败");
        return;
    }

    //顶点和片元shader初始化
    //顶点shader初始化
    GLint vsh = InitShader(vertexShader, GL_VERTEX_SHADER);
    //片元yuv420 shader初始化
    GLint fsh = InitShader(fragYUV420P, GL_FRAGMENT_SHADER);

    //创建渲染程序
    GLint program = glCreateProgram();
    if (program == 0) {
        LOGD("glCreateProgram failed!");
        return;
    }
    //渲染程序中加入着色器代码
    glAttachShader(program, vsh);
    glAttachShader(program, fsh);

    //链接程序
    glLinkProgram(program);
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        LOGD("glLinkProgram failed!");
        return;
    }
    glUseProgram(program);
    LOGD("glLinkProgram success!");

    //加入三维顶点数据 两个三角形组成正方形
    static float vers[] = {
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
    };
    GLuint apos = (GLuint) glGetAttribLocation(program, "aPosition");
    glEnableVertexAttribArray(apos);
    //传递顶点
    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 12, vers);

    //加入材质坐标数据
    static float txts[] = {
            1.0f, 0.0f, //右下
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0, 1.0
    };
    GLuint atex = (GLuint) glGetAttribLocation(program, "aTexCoord");
    glEnableVertexAttribArray(atex);
    glVertexAttribPointer(atex, 2, GL_FLOAT, GL_FALSE, 8, txts);

    int width = 424;
    int height = 240;

    //材质纹理初始化
    //设置纹理层
    glUniform1i(glGetUniformLocation(program, "yTexture"), 0); //对于纹理第1层
    glUniform1i(glGetUniformLocation(program, "uTexture"), 1); //对于纹理第2层
    glUniform1i(glGetUniformLocation(program, "vTexture"), 2); //对于纹理第3层

    //创建opengl纹理
    GLuint texts[3] = {0};
    //创建三个纹理
    glGenTextures(3, texts);

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D, texts[0]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,           //细节基本 0默认
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图
                 width, height, //拉升到全屏
                 0,             //边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
    );

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D, texts[1]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,           //细节基本 0默认
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图
                 width / 2, height / 2, //拉升到全屏
                 0,             //边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
    );

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D, texts[2]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,           //细节基本 0默认
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图
                 width / 2, height / 2, //拉升到全屏
                 0,             //边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
    );

    ////纹理的修改和显示
    unsigned char *buf[3] = {0};
    buf[0] = new unsigned char[width * height];
    buf[1] = new unsigned char[width * height / 4];
    buf[2] = new unsigned char[width * height / 4];


    for (int i = 0; i < 10000; i++) {
        //420p   yyyyyyyy uu vv
        if (feof(fp) == 0) {
            //yyyyyyyy
            fread(buf[0], 1, width * height, fp);
            fread(buf[1], 1, width * height / 4, fp);
            fread(buf[2], 1, width * height / 4, fp);
        }

        //激活第1层纹理,绑定到创建的opengl纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texts[0]);
        //替换纹理内容
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                        buf[0]);

        //激活第2层纹理,绑定到创建的opengl纹理
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, texts[1]);
        //替换纹理内容
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                        GL_UNSIGNED_BYTE, buf[1]);

        //激活第2层纹理,绑定到创建的opengl纹理
        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, texts[2]);
        //替换纹理内容
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                        GL_UNSIGNED_BYTE, buf[2]);

        //三维绘制
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //窗口显示EGLSurface
        eglSwapBuffers(display, winSurface);

        // 释放字符串空间
        env->ReleaseStringUTFChars(path, fPath);
    }
}