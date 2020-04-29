//
// Created by luopan on 2020/4/28.
//

#include "FFPlayerBuilder.h"
#include "FFDemux.h"
#include "FFDecode.h"
#include "FFResample.h"
#include "GLVideoView.h"
#include "SLAudioPlay.h"

IDemux *FFPlayerBuilder::CreateDemux() {
    IDemux *ffdemux = new FFDemux();
    return ffdemux;
}

IDecode *FFPlayerBuilder::CreateDecode() {
    IDecode *ffdecode = new FFDecode();
    return ffdecode;
}

IResample *FFPlayerBuilder::CreateResample() {
    IResample *ffresample = new FFResample();
    return ffresample;
}

IVideoView *FFPlayerBuilder::CreateVideoView() {
    IVideoView *glvideoview = new GLVideoView();
    return glvideoview;
}

IAudioPlay *FFPlayerBuilder::CreateAudioPlay() {
    IAudioPlay *slaudioplay = new SLAudioPlay();
    return slaudioplay;
}

IPlayer *FFPlayerBuilder::CreatePlayer(unsigned char index) {
    return IPlayer::Get(index);
}

void FFPlayerBuilder::InitHard(void *vm) {
    FFDecode::InitHard(vm);
}
