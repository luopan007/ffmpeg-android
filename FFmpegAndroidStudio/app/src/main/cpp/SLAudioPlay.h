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

    /**
     * 构造函数
     */
    SLAudioPlay();

    /**
     * 析构函数
     */
    virtual ~SLAudioPlay();

protected:
    /**
     * 缓存音频数据
     */
    unsigned char *buf = 0;
};


#endif //FFMPEG_SLAUDIOPLAY_H
