//
// Created by luopan on 2020/4/27.
//

#include "IPlayer.h"
#include "IDemux.h"
#include "IDecode.h"
#include "IAudioPlay.h"
#include "IVideoView.h"
#include "IResample.h"
#include "XLog.h"

const int MAX_PLAYER_NUM = 256;

IPlayer *IPlayer::Get(unsigned char index) {
    static IPlayer player[MAX_PLAYER_NUM];
    return &player[index];
}

void IPlayer::Main() {
    while (!isExit) {
        mux.lock();
        if (!audioPlay || !vDecode) {
            mux.unlock();
            XSleep(2);
            continue;
        }

        //同步
        //获取音频的pts 告诉视频
        int apts = audioPlay->pts;
        vDecode->synPts = apts;

        mux.unlock();
        XSleep(2);
    }
}

void IPlayer::Close() {
    mux.lock();
    //2 先关闭主体线程，再清理观察者
    //同步线程
    XThread::Stop();
    //解封装
    if (demux) {
        demux->Stop();
    }
    //解码
    if (vDecode) {
        vDecode->Stop();
    }
    if (aDecode) {
        aDecode->Stop();
    }
    if (audioPlay) {
        audioPlay->Stop();
    }

    //2 清理缓冲队列
    if (vDecode) {
        vDecode->Clear();
    }
    if (aDecode) {
        aDecode->Clear();
    }
    if (audioPlay) {
        audioPlay->Clear();
    }

    //3 清理资源
    if (audioPlay) {
        audioPlay->Close();
    }
    if (videoView) {
        videoView->Close();
    }
    if (vDecode) {
        vDecode->Close();
    }
    if (aDecode) {
        aDecode->Close();
    }
    if (demux) {
        demux->Close();
    }
    mux.unlock();
}

double IPlayer::PlayPos() {
    double pos = 0.0;
    mux.lock();
    int total = 0;
    if (demux) {
        total = demux->total;
    }
    if (total > 0) {
        if (vDecode) {
            pos = (double) vDecode->pts / (double) total;
        }
    }
    mux.unlock();
    return pos;
}

void IPlayer::SetPause(bool isP) {
    mux.lock();
    XThread::SetPause(isP);
    if (demux) {
        demux->SetPause(isP);
    }
    if (vDecode) {
        vDecode->SetPause(isP);
    }
    if (aDecode) {
        aDecode->SetPause(isP);
    }
    if (audioPlay) {
        audioPlay->SetPause(isP);
    }
    mux.unlock();
}

bool IPlayer::Seek(double pos) {
    bool re = false;
    if (!demux) return false;

    //暂停所有线程
    SetPause(true);
    mux.lock();
    //清理缓冲
    //2 清理缓冲队列
    if (vDecode) {
        vDecode->Clear(); //清理缓冲队列，清理ffmpeg的缓冲
    }
    if (aDecode) {
        aDecode->Clear();
    }
    if (audioPlay) {
        audioPlay->Clear();
    }

    re = demux->Seek(pos); //seek跳转到关键帧
    if (!vDecode) {
        mux.unlock();
        SetPause(false);
        return re;
    }
    //解码到实际需要显示的帧
    int seekPts = pos * demux->total;
    while (!isExit) {
        XData pkt = demux->Read();
        if (pkt.size <= 0)break;
        if (pkt.isAudio) {
            if (pkt.pts < seekPts) {
                pkt.Drop();
                continue;
            }
            //写入缓冲队列
            demux->Notify(pkt);
            continue;
        }

        //解码需要显示的帧之前的数据
        vDecode->SendPacket(pkt);
        pkt.Drop();
        XData data = vDecode->RecvFrame();
        if (data.size <= 0) {
            continue;
        }
        if (data.pts >= seekPts) {
            break;
        }
    }
    mux.unlock();

    SetPause(false);
    return re;
}

bool IPlayer::Open(const char *path) {
    Close();
    mux.lock();
    if (!demux || !demux->Open(path)) {
        mux.unlock();
        XLOGW("demux open failed %s", path);
        return false;
    }
    if (!vDecode || !vDecode->Open(demux->GetVideoPara(), isHardDecode)) {
        XLOGW("vDecode open failed %s", path);
    }
    if (!aDecode || !aDecode->Open(demux->GetAudioPara())) {
        XLOGW("aDecode open failed %s", path);
    }
    outPara = demux->GetAudioPara();
    if (!resample || !resample->Open(demux->GetAudioPara(), outPara)) {
        XLOGW("resample open failed %s", path);
    }
    XLOGI("IPlayer Open success");
    mux.unlock();
    return true;
}

bool IPlayer::Start() {

    mux.lock();
    if (vDecode) {
        vDecode->Start();
    }

    if (!demux || !demux->Start()) {
        mux.unlock();
        XLOGW("demux start failed");
        return false;
    }
    if (aDecode) {
        aDecode->Start();
    }
    if (audioPlay) {
        audioPlay->StartPlay(outPara);
    }
    XThread::Start();
    mux.unlock();

    XLOGI("IPlayer Start success");
    return true;
}


void IPlayer::InitView(void *win) {
    if (videoView) {
        videoView->Close();
        videoView->SetRender(win);
    }
}

IPlayer::IPlayer() {

}
