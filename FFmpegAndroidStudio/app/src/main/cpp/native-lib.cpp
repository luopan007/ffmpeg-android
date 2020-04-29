#include <jni.h>
#include <string>
#include <android/native_window_jni.h>

#include "XLog.h"
#include "IPlayerPorxy.h"

extern "C"
JNIEXPORT
jint JNI_OnLoad(JavaVM *vm, void *res) {
    IPlayerPorxy::Get()->Init(vm);
    return JNI_VERSION_1_6;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_XPlay_InitView(JNIEnv *env, jobject instance, jobject surface) {
    // 从Java获取窗口
    ANativeWindow *win = ANativeWindow_fromSurface(env, surface);
    IPlayerPorxy::Get()->InitView(win);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_OpenUrl_Open(JNIEnv *env, jobject thiz, jstring path) {
    const char *url = env->GetStringUTFChars(path, 0);
    IPlayerPorxy::Get()->Open(url);
    IPlayerPorxy::Get()->Start();
    env->ReleaseStringUTFChars(path, url);
}

extern "C"
JNIEXPORT jdouble JNICALL
Java_com_luopan_ffmpeg_MainActivity_GetPlayPosition(JNIEnv *env, jobject thiz) {
    return IPlayerPorxy::Get()->PlayPos();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_MainActivity_SeekTo(JNIEnv *env, jobject thiz, jdouble position) {
    IPlayerPorxy::Get()->Seek(position);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_XPlay_PlayOrPause(JNIEnv *env, jobject thiz) {
    IPlayerPorxy::Get()->SetPause(!IPlayerPorxy::Get()->IsPause());
}