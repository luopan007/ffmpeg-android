//
// Created by luopan on 2020/4/15.
//

#ifndef FFMPEG_IRESAMPLE_H
#define FFMPEG_IRESAMPLE_H

#include "XParameter.h"
#include "IObserver.h"

class IResample : public IObserver {
public:
    virtual bool Open(XParameter in, XParameter out = XParameter()) = 0;

    virtual XData Resample(XData indata) = 0;

    virtual void Update(XData data);

    int outChannels = 2;
    int outFormat = 1;
};


#endif //FFMPEG_IRESAMPLE_H
