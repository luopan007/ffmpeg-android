//
// Created by luopan on 2020/4/14.
//

#ifndef FFMPEG_XEGL_H
#define FFMPEG_XEGL_H


class XEGL {
public:
    /**
     * 初始化EGL
     *
     * @param win 从Java层传递下来的窗口
     * @return 是否初始化成功
     */
    virtual bool Init(void *win) = 0;

    /**
     * 绘制
     */
    virtual void Draw() = 0;

    /**
     * 单例模式获取EGL对象
     *
     * @return
     */
    static XEGL *Get();

protected:
    XEGL() {}
};


#endif //FFMPEG_XEGL_H