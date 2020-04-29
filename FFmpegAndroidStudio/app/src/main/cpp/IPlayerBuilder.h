//
// Created by luopan on 2020/4/28.
//

#ifndef FFMPEG_IPLAYERBUILDER_H
#define FFMPEG_IPLAYERBUILDER_H

#include "IPlayer.h"

class IPlayerBuilder {
public:
    /**
     * 构建播放器对象
     *
     * @param index 播放器的index，多路播放
     * @return 播放器实例
     */
    virtual IPlayer *BuilderPlayer(unsigned char index = 0);

protected:
    /**
     * 创建解封装器的接口----使用virtual和等于0操作是为了标记为接口
     *
     * @return 解封装器
     */
    virtual IDemux *CreateDemux() = 0;

    /**
     * 创建解码器的接口----使用virtual和等于0操作是为了标记为接口
     *
     * @return 解码器
     */
    virtual IDecode *CreateDecode() = 0;

    /**
     * 创建重采样的接口----使用virtual和等于0操作是为了标记为接口
     *
     * @return 重采样实例对象
     */
    virtual IResample *CreateResample() = 0;

    /**
     * 创建视频显示接口----使用virtual和等于0操作是为了标记为接口
     *
     * @return 视频显示实例对象
     */
    virtual IVideoView *CreateVideoView() = 0;

    /**
     * 创建音频输出接口----使用virtual和等于0操作是为了标记为接口
     *
     * @return 音频输出实例对象
     */
    virtual IAudioPlay *CreateAudioPlay() = 0;

    /**
     * 创建播放器接口----使用virtual和等于0操作是为了标记为接口
     *
     * @param index 播放器的index，多路播放
     * @return 播放器实例
     */
    virtual IPlayer *CreatePlayer(unsigned char index = 0) = 0;
};


#endif //FFMPEG_IPLAYERBUILDER_H
