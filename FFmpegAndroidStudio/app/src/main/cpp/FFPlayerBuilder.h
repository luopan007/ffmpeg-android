//
// Created by luopan on 2020/4/28.
//

#ifndef FFMPEG_FFPLAYERBUILDER_H
#define FFMPEG_FFPLAYERBUILDER_H

#include "IPlayerBuilder.h"

class FFPlayerBuilder : public IPlayerBuilder {
public:
    static void InitHard(void *vm);

    static FFPlayerBuilder *Get() {
        static FFPlayerBuilder ff;
        return &ff;
    }

protected:
    /**
     * 单例模式
     */
    FFPlayerBuilder() {};

    /**
     * 创建解封装器的接口----使用virtual和去掉等于0操作是为了完成具体实现
     *
     * @return 解封装器
     */
    virtual IDemux *CreateDemux();

    /**
     * 创建解码器的接口----使用virtual和去掉等于0操作是为了完成具体实现
     *
     * @return 解码器
     */
    virtual IDecode *CreateDecode();

    /**
     * 创建重采样的接口----使用virtual和去掉等于0操作是为了完成具体实现
     *
     * @return 重采样实例对象
     */
    virtual IResample *CreateResample();

    /**
     * 创建视频显示接口----使用virtual和去掉等于0操作是为了完成具体实现
     *
     * @return 视频显示实例对象
     */
    virtual IVideoView *CreateVideoView();

    /**
     * 创建音频输出接口----使用virtual和去掉等于0操作是为了完成具体实现
     *
     * @return 音频输出实例对象
     */
    virtual IAudioPlay *CreateAudioPlay();

    /**
     * 创建播放器接口----使用virtual和去掉等于0操作是为了完成具体实现
     *
     * @param index 播放器的index，多路播放
     * @return 播放器实例
     */
    virtual IPlayer *CreatePlayer(unsigned char index = 0);
};

#endif //FFMPEG_FFPLAYERBUILDER_H
