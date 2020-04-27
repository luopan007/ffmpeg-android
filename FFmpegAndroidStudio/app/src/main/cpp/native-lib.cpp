#include <jni.h>
#include <string>
#include <android/native_window_jni.h> // android native 窗口

#include "IDemux.h"
#include "XLog.h"
#include "IDecode.h"
#include "FFDemux.h"
#include "FFDecode.h"
#include "XEGL.h"
#include "XShader.h"
#include "IVideoView.h"
#include "GLVideoView.h"
#include "FFResample.h"
#include "IAudioPlay.h"
#include "SLAudioPlay.h"
#include "IPlayer.h"


extern "C"
jint JNI_OnLoad(JavaVM *vm, void *res) {
    FFDecode::InitHard(vm);
    return JNI_VERSION_1_6;
}

IVideoView *view = NULL;

extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_XPlay_open(JNIEnv *env, jobject thiz, jstring path, jobject surface) {
    IDemux *demux = new FFDemux();
    IDecode *vdecode = new FFDecode();
    demux->AddObserver(vdecode); // 把视频解码器当成观察者加入到解封装器中

    IDecode *adecode = new FFDecode();
    demux->AddObserver(adecode); // 把音频解码器当成观察者加入到解封装器中

    view = new GLVideoView();
    vdecode->AddObserver(view);

    IResample *resample = new FFResample();
    adecode->AddObserver(resample);

    IAudioPlay *audioPlay = new SLAudioPlay();
    resample->AddObserver(audioPlay);

    IPlayer::Get()->demux = demux;
    IPlayer::Get()->vDecode = vdecode;
    IPlayer::Get()->aDecode = adecode;
    IPlayer::Get()->videoView = view;
    IPlayer::Get()->resample = resample;
    IPlayer::Get()->audioPlay = audioPlay;
    IPlayer::Get()->isHardDecode = false;
    IPlayer::Get()->Open("/sdcard/1080.mp4");
    IPlayer::Get()->Start();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_XPlay_InitView(JNIEnv *env, jobject instance, jobject surface) {
    // 从Java获取窗口
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    IPlayer::Get()->InitView(nativeWindow);
}