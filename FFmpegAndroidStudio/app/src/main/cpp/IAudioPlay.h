//
// Created by luopan on 2020/4/17.
//

#ifndef FFMPEG_IAUDIOPLAY_H
#define FFMPEG_IAUDIOPLAY_H


#include <list>
#include "IObserver.h"
#include "XParameter.h"

class IAudioPlay : public IObserver {
public:
    /**
     * 更新数据---缓冲满了以后，阻塞等待
     *
     * @param data
     */
    virtual void Update(XData data);

    /**
     * 获取缓冲数据，如果没有数据则阻塞；
     *
     * @return 缓冲数据
     */
    virtual XData GetData();

    /**
     * 开始播放
     *
     * @param out
     * @return
     */
    virtual bool StartPlay(XParameter out) = 0;

    virtual void Close() = 0;

    virtual void Clear();

    /**
     * 默认队列的最大缓冲
     */
    int maxFrame = 100;
    int pts = 0;
protected:
    std::list<XData> frames;
    std::mutex framesMutex;
};


#endif //FFMPEG_IAUDIOPLAY_H
