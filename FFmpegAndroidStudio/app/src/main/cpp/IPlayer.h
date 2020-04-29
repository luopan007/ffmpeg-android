//
// Created by luopan on 2020/4/27.
//

#ifndef FFMPEG_IPLAYER_H
#define FFMPEG_IPLAYER_H


#include <mutex>
#include "XThread.h"
#include "XParameter.h"

class IDemux;

class IDecode;

class IResample;

class IVideoView;

class IAudioPlay;

class IPlayer : public XThread {
public:
    /**
     * 获取IPlayer对象
     *
     * @param index IPlayer的对象索引
     * @return IPlayer的实例对象
     */
    static IPlayer *Get(unsigned char index = 0);

    /**
     * 打开播放器
     *
     * @param path 文件路径
     * @return 是否打开成功
     */
    virtual bool Open(const char *path);

    virtual void Close();

    /**
     * 开始播放
     *
     * @return 是否开始成功
     */
    virtual bool Start();

    /**
     * 初始化显示窗口
     *
     * @param win 窗口实例
     * @return 是否初始化成功
     */
    virtual void InitView(void *win);

    //获取当前的播放进度 0.0 ~ 1.0
    virtual double PlayPos();

    virtual bool Seek(double pos);

    virtual void SetPause(bool isP);

    /**
     * 音频输出参数配置
     */
    XParameter outPara;

    /**
     * 是否采用硬解码
     */
    bool isHardDecode = false;

    /**
 * 解封装器---使用public保护，暴漏外部使用
 */
    IDemux *demux = 0;

    /**
     * 视频解码器---使用public保护，暴漏外部使用
     */
    IDecode *vDecode = 0;

    /**
     * 音频解码器---使用public保护，暴漏外部使用
     */
    IDecode *aDecode = 0;

    /**
     * 音频重采样---使用public保护，暴漏外部使用
     */
    IResample *resample = 0;

    /**
     * 视频显示---使用public保护，暴漏外部使用
     */
    IVideoView *videoView = 0;

    /**
     * 音频输出---使用public保护，暴漏外部使用
     */
    IAudioPlay *audioPlay = 0;

protected:
    //用作音视频同步
    void Main();

    std::mutex mux;

    /**
     * 构造函数被保护起来
     */
    IPlayer();
};


#endif //FFMPEG_IPLAYER_H
