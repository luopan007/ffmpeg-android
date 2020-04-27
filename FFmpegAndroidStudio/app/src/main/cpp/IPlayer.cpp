//
// Created by luopan on 2020/4/27.
//

#include "IPlayer.h"
#include "IDemux.h"
#include "IDecode.h"
#include "IResample.h"
#include "IVideoView.h"
#include "IAudioPlay.h"
#include "XLog.h"

const int MAX_PLAYER_NUM = 256;

IPlayer *IPlayer::Get(unsigned char index) {
    static IPlayer player[MAX_PLAYER_NUM];
    return &player[index];
}

bool IPlayer::Open(const char *path) {
    if (!demux || !demux->Open(path)) {
        XLOGW("demux open failed %s", path);
        return false;
    }
    if (!vDecode || !vDecode->Open(demux->GetVideoPara(), isHardDecode)) {
        XLOGW("vDecode open failed %s", path);
    }
    if (!aDecode || !aDecode->Open(demux->GetAudioPara())) {
        XLOGW("aDecode open failed %s", path);
    }
    if (outPara.sampleRate <= 0) {
        outPara = demux->GetAudioPara();
    }
    if (!resample || !resample->Open(demux->GetAudioPara(), outPara)) {
        XLOGW("resample open failed %s", path);
    }
    XLOGI("IPlayer Open success");
    return true;
}

bool IPlayer::Start() {
    if (!demux || !demux->Start()) {
        XLOGW("demux start failed");
        return false;
    }
    if (aDecode) {
        aDecode->Start();
    }
    if (audioPlay) {
        audioPlay->StartPlay(outPara);
    }
    if (vDecode) {
        vDecode->Start();
    }
    XLOGI("IPlayer Start success");
    return true;
}

IPlayer::IPlayer() {
}

bool IPlayer::InitView(void *win) {
    if (videoView) {
        videoView->SetRender(win);
    }
    return true;
}
