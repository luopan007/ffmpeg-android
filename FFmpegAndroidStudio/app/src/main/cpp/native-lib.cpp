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

IVideoView *view = NULL;

extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_XPlay_open(JNIEnv *env, jobject thiz, jstring path, jobject surface) {
    IDemux *demux = new FFDemux();
    demux->Open("/sdcard/1080.mp4");

    IDecode *vdecode = new FFDecode();
    vdecode->Open(demux->GetVideoPara());
    demux->AddObserver(vdecode); // 把视频解码器当成观察者加入到解封装器中

    IDecode *adecode = new FFDecode();
    adecode->Open(demux->GetAudioPara());
    demux->AddObserver(adecode); // 把音频解码器当成观察者加入到解封装器中

    view = new GLVideoView();
    vdecode->AddObserver(view);

    IResample *resample = new FFResample();
    resample->Open(demux->GetAudioPara());
    adecode->AddObserver(resample);

    demux->Start();
    vdecode->Start();
    adecode->Start();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_XPlay_InitView(JNIEnv *env, jobject instance, jobject surface) {
    // 从Java获取窗口
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    view->SetRender(nativeWindow);
//    XEGL::Get()->Init(nativeWindow);
//    XShader shader;
//    shader.Init();
}