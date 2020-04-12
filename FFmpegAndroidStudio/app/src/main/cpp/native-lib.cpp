#include <jni.h>
#include <string>

#include "FFDemux.h"
#include "XLog.h"

class TestObs : public IObserver {
public:
    void Update(XData data) {
        XLOGI("TestObs-->Update-->data size[%d]", data.size);
    }
};

extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_XPlay_open(JNIEnv *env, jobject thiz, jstring path, jobject surface) {
    TestObs *obs = new TestObs();
    IDemux *demux = new FFDemux();
    demux->AddObserver(obs);
    demux->Open("/sdcard/1080.mp4");
    demux->Start();
    XSleep(3000);
    demux->Stop();
}