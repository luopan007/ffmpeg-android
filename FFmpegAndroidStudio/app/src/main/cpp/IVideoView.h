//
// Created by luopan on 2020/4/14.
//

#ifndef FFMPEG_IVIDEOVIEW_H
#define FFMPEG_IVIDEOVIEW_H


#include "XData.h"
#include "IObserver.h"

class IVideoView : public IObserver {
public:

/**
 * 设置渲染器---此时播放器可能还没有创建
 *
 * @param win 窗口
 */
    virtual void SetRender(void *win) = 0;

/**
 * 定义给子类去实际渲染数据的接口
 *
 * @param data 需要渲染的数据
 */
    virtual void Render(XData data) = 0;

/**
 * 由自己出发渲染，其实是直接调用自己的Render函数
 *
 * @param data 需要渲染的数据
 */
    virtual void Update(XData data);

    virtual void Close() = 0;
};


#endif //FFMPEG_IVIDEOVIEW_H