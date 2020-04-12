//
// Created by luopan on 2020/4/12.
//

#ifndef FFMPEG_FFDECODE_H
#define FFMPEG_FFDECODE_H

#include "XParameter.h"
#include "IDecode.h"

struct AVCodecContext;

/**
 * 解码接口，支持硬解码
 */
class FFDecode : public IDecode {
    /**
     * 打开解码器参数
     *
     * @return 是否打开成功
     */
    virtual bool Open(XParameter param);

protected:
    /**
     * 解码器上下文，复用
     */
    AVCodecContext *codec = 0;
};


#endif //FFMPEG_FFDECODE_H
