//
// Created by luopan on 2020/4/17.
//

#ifndef FFMPEG_IAUDIOPLAY_H
#define FFMPEG_IAUDIOPLAY_H


#include "IObserver.h"
#include "XParameter.h"

class IAudioPlay : public IObserver {
public:
    /**
     * 更新数据
     *
     * @param data
     */
    virtual void Update(XData data);

    /**
     * 开始播放
     *
     * @param out
     * @return
     */
    virtual bool StartPlay(XParameter out) = 0;
};


#endif //FFMPEG_IAUDIOPLAY_H
