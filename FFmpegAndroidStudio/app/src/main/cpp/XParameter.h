//
// Created by luopan on 2020/4/12.
//

#ifndef FFMPEG_XPARAMETER_H
#define FFMPEG_XPARAMETER_H

// 声明指针可以避免引入头文件
struct AVCodecParameters;

class XParameter {
public:
    AVCodecParameters *para = 0;
};


#endif //FFMPEG_XPARAMETER_H
