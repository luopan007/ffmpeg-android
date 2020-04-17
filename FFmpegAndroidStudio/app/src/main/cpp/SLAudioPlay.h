//
// Created by luopan on 2020/4/17.
//

#ifndef FFMPEG_SLAUDIOPLAY_H
#define FFMPEG_SLAUDIOPLAY_H


#include "IAudioPlay.h"

class SLAudioPlay : public IAudioPlay {
public:
    /**
     * 开始播放
     *
     * @param out
     * @return
     */
    virtual bool StartPlay(XParameter out);

    void PlayCall(void *bufQueue);
};


#endif //FFMPEG_SLAUDIOPLAY_H
