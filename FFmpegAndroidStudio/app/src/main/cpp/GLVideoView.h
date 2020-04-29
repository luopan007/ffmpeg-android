//
// Created by luopan on 2020/4/14.
//

#ifndef FFMPEG_GLVIDEOVIEW_H
#define FFMPEG_GLVIDEOVIEW_H


#include "XData.h"
#include "IVideoView.h"

class XTexture; // 写这句话的目的是不想去引用XTexture的头文件

class GLVideoView : public IVideoView {
public:
    /**
     * 设置渲染器---此时播放器可能还没有创建
     *
     * @param win 窗口
     */
    virtual void SetRender(void *win);

    /**
     * 实际渲染数据的接口---作为IVideoView的子类
     *
     * @param data 需要渲染的数据
     */
    virtual void Render(XData data);

    virtual void Close();

protected:
    /**
     * 窗体显示
     */
    void *view = 0;

    /**
     * 渲染的纹理
     */
    XTexture *txt = 0;
    std::mutex mux;
};


#endif //FFMPEG_GLVIDEOVIEW_H
