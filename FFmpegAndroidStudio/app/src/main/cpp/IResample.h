//
// Created by luopan on 2020/4/15.
//

#ifndef FFMPEG_IRESAMPLE_H
#define FFMPEG_IRESAMPLE_H

#include "XParameter.h"
#include "IObserver.h"

class IResample : public IObserver {
public:
    /**
     * 重采样--需要知道输入和输出的格式
     *
     * @param in 音频的输入格式---原始编码格式
     * @param out 音频的输出格式---与声卡保持一致
     * @return 是否打开成功
     */
    virtual bool Open(XParameter in, XParameter out = XParameter()) = 0;

    /**
     * 重采样
     *
     * @param indata 输入数据
     * @return 重采样之后的数据
     */
    virtual XData Resample(XData indata) = 0;

    virtual void Close() = 0;

    /**
     * 从解封装其获取数据，然后重采样，然后发送给声卡输出
     *
     * @param data
     */
    virtual void Update(XData data);

    int outChannels = 2;

    int outFormat = 1;
};


#endif //FFMPEG_IRESAMPLE_H
