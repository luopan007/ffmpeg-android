//
// Created by luopan on 2020/4/15.
//

#ifndef FFMPEG_FFRESAMPLE_H
#define FFMPEG_FFRESAMPLE_H


#include "IResample.h"

struct SwrContext;

class FFResample : public IResample {
public:
    virtual bool Open(XParameter in, XParameter out = XParameter());

    virtual XData Resample(XData indata);

protected:
    SwrContext *actx = 0;

};


#endif //FFMPEG__FFRESAMPLE_H
