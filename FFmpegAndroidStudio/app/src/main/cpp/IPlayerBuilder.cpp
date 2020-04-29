//
// Created by luopan on 2020/4/28.
//

#include "IPlayerBuilder.h"
#include "IVideoView.h"
#include "IResample.h"
#include "IDecode.h"
#include "IAudioPlay.h"
#include "IDemux.h"

IPlayer *IPlayerBuilder::BuilderPlayer(unsigned char index) {
    IPlayer *play = CreatePlayer(index);

    IDemux *demux = CreateDemux();

    IDecode *vdecode = CreateDecode();
    demux->AddObserver(vdecode); // 视频解码器观察解封装器

    IDecode *adecode = CreateDecode();
    demux->AddObserver(adecode); // 音频解码器观察解封装器

    IVideoView *view = CreateVideoView();
    vdecode->AddObserver(view);

    IResample *resample = CreateResample();
    adecode->AddObserver(resample);

    IAudioPlay *audioPlay = CreateAudioPlay();
    resample->AddObserver(audioPlay);

    play->demux = demux;
    play->vDecode = vdecode;
    play->aDecode = adecode;
    play->videoView = view;
    play->resample = resample;
    play->audioPlay = audioPlay;
    play->isHardDecode = false;
    return play;
}