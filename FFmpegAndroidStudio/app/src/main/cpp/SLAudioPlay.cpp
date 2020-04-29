//
// Created by luopan on 2020/4/17.
//

#include "SLAudioPlay.h"
#include <SLES/OpenSLES.h> // 音频数据播放头文件
#include <SLES/OpenSLES_Android.h>
#include "XLog.h"

// static成员变量只在本cpp文件生效
static SLObjectItf engineSL = NULL;

static SLEngineItf eng = NULL;

static SLObjectItf mix = NULL;

static SLObjectItf player = NULL;

static SLPlayItf iplayer = NULL;

static SLAndroidSimpleBufferQueueItf pcmQue = NULL;

// 1MB的缓冲空间
static const int BUF_SIZE = 1024 * 1024;

SLAudioPlay::SLAudioPlay() {
    buf = new unsigned char[BUF_SIZE];
}

SLAudioPlay::~SLAudioPlay() {
    delete buf;
    buf = 0;
}

/**
 * 创建引擎
 *
 * @return 音频引擎
 */
static SLEngineItf CreateSL() {
    SLresult re;
    SLEngineItf en;
    re = slCreateEngine(&engineSL, 0, 0, 0, 0, 0);
    if (re != SL_RESULT_SUCCESS) {
        return NULL;
    }
    re = (*engineSL)->Realize(engineSL, SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS) {
        return NULL;
    }
    re = (*engineSL)->GetInterface(engineSL, SL_IID_ENGINE, &en);
    if (re != SL_RESULT_SUCCESS) {
        return NULL;
    }
    return en;
}

void SLAudioPlay::PlayCall(void *bufQueue) {
    if (!bufQueue) {
        XLOGW("PlayCall failed bufQueue is null.");
        return;
    }
    SLAndroidSimpleBufferQueueItf bf = static_cast<SLAndroidSimpleBufferQueueItf>(bufQueue);
    // 阻塞函数
    XData d = GetData();
    if (d.size <= 0) {
        XLOGW("GetData size is 0");
        return;
    }
    if (!buf) {
        return;
    }
    memcpy(buf, d.data, d.size);
    mux.lock();
    if (bf && (*bf)) {
        (*bf)->Enqueue(bf, buf, d.size);
    }
    mux.unlock();
    d.Drop(); // 销毁数据，否则内存泄漏
}

/**
 * 回调函数
 *
 * @param bf
 * @param context
 */
static void PcmCall(SLAndroidSimpleBufferQueueItf bf, void *context) {
    SLAudioPlay *ap = static_cast<SLAudioPlay *>(context);
    if (!ap) {
        XLOGW("PcmCall failed context is null");
        return;
    }

    // 如果bf是对象，是不能强转为void*的；
    ap->PlayCall((void *) bf);
}

void SLAudioPlay::Close() {
    IAudioPlay::Clear();
    mux.lock();

    //停止播放
    if (iplayer && (*iplayer)) {
        (*iplayer)->SetPlayState(iplayer, SL_PLAYSTATE_STOPPED);
    }

    //清理播放队列
    if (pcmQue && (*pcmQue)) {
        (*pcmQue)->Clear(pcmQue);
    }

    //销毁player对象
    if (player && (*player)) {
        (*player)->Destroy(player);
    }

    //销毁混音器
    if (mix && (*mix)) {
        (*mix)->Destroy(mix);
    }

    //销毁播放引擎
    if (engineSL && (*engineSL)) {
        (*engineSL)->Destroy(engineSL);
    }

    engineSL = NULL;
    eng = NULL;
    mix = NULL;
    player = NULL;
    iplayer = NULL;
    pcmQue = NULL;
    mux.unlock();
}

bool SLAudioPlay::StartPlay(XParameter out) {
    Close();
    mux.lock();

    // 1.创建音频解码引擎
    eng = CreateSL();
    if (!eng) {
        XLOGW("创建引擎失败");
        mux.unlock();
        return false;
    }

    // 2.创建混音器
    SLresult re = 0;
    re = (*eng)->CreateOutputMix(eng, &mix, 0, 0, 0);
    if (re != SL_RESULT_SUCCESS) {
        XLOGW("CreateOutputMix fail")
        mux.unlock();
        return false;
    }
    re = (*mix)->Realize(mix, SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS) {
        mux.unlock();
        XLOGW("Realize fail");
        return false;
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, mix};
    SLDataSink audioSink = {&outputMix, 0};

    // 3.配置音频信息
    SLDataLocator_AndroidSimpleBufferQueue que = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            10};                                                //缓冲队列
    SLDataFormat_PCM pcmData = {SL_DATAFORMAT_PCM,              // 数据格式为PCM
                                (SLuint32) out.channels,                    // 声道数
                                (SLuint32) out.sampleRate * 1000,           // 采样率
                                SL_PCMSAMPLEFORMAT_FIXED_16,                // 位宽
                                SL_PCMSAMPLEFORMAT_FIXED_16,
                                SL_SPEAKER_FRONT_LEFT |
                                SL_SPEAKER_FRONT_RIGHT,                     // 前左和前右输出
                                SL_BYTEORDER_LITTLEENDIAN                   // 字节序，小端
    };
    SLDataSource ds = {&que, &pcmData}; // 参数福给结构体，供播放器使用

    // 4.创建播放器
    const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE};
    re = (*eng)->CreateAudioPlayer(eng, &player, &ds, &audioSink,
                                   sizeof(ids) / sizeof(SLInterfaceID), ids,
                                   req);
    if (re != SL_RESULT_SUCCESS) {
        XLOGW("创建播放器失败");
        mux.unlock();
        return false;
    }

    // 5.获取player接口
    (*player)->Realize(player, SL_BOOLEAN_FALSE);
    re = (*player)->GetInterface(player, SL_IID_PLAY, &iplayer);
    if (re != SL_RESULT_SUCCESS) {
        XLOGW("获取接口失败");
        mux.unlock();
        return false;
    }

    // 6.获取队列接口
    re = (*player)->GetInterface(player, SL_IID_BUFFERQUEUE, &pcmQue);
    if (re != SL_RESULT_SUCCESS) {
        XLOGW("获取队列失败");
        mux.unlock();
        return false;
    }

    // 7.设置回调函数，播放队列函数空调用
    (*pcmQue)->RegisterCallback(pcmQue, PcmCall, this);
    (*iplayer)->SetPlayState(iplayer, SL_PLAYSTATE_PLAYING); // 设置播放状态

    // 启动队列
    (*pcmQue)->Enqueue(pcmQue, "", 1);
    isExit = false;
    mux.unlock();
    XLOGI("SLAudioPlay::StartPlay Success.");
    return true;
}