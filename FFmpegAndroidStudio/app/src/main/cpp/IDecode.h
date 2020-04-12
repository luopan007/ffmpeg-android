//
// Created by luopan on 2020/4/12.
//

#ifndef FFMPEG_IDECODE_H
#define FFMPEG_IDECODE_H


#include "XParameter.h"
#include "IObserver.h"

/**
 * 解码接口，支持硬解码
 */
class IDecode : public IObserver {
public:
    /**
     * 打开解码器参数
     *
     * @return 是否打开成功
     */
    virtual bool Open(XParameter param) = 0;
};


#endif //FFMPEG_IDECODE_H