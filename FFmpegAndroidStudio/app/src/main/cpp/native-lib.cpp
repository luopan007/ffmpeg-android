#include <jni.h>
#include <string>

#include "IDemux.h"
#include "XLog.h"
#include "IDecode.h"
#include "FFDemux.h"
#include "FFDecode.h"

/**
 * 测试观察者模式----测试代码
 */
class TestObs : public IObserver {
public:
    void Update(XData data) {
//        XLOGI("TestObs-->Update-->data size[%d]", data.size);
    }
};

extern "C"
JNIEXPORT void JNICALL
Java_com_luopan_ffmpeg_XPlay_open(JNIEnv *env, jobject thiz, jstring path, jobject surface) {
    TestObs *obs = new TestObs();
    IDemux *demux = new FFDemux();
//    demux->AddObserver(obs);
    demux->Open("/sdcard/1080.mp4");

    IDecode *vdecode = new FFDecode();
    vdecode->Open(demux->GetVideoPara());
    demux->AddObserver(vdecode); // 把视频解码器当成观察者加入到解封装器中

    IDecode *adecode = new FFDecode();
    adecode->Open(demux->GetAudioPara());
    demux->AddObserver(adecode); // 把音频解码器当成观察者加入到解封装器中

    demux->Start();
    vdecode->Start();
    adecode->Start();
}