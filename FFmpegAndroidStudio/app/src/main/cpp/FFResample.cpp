//
// Created by luopan on 2020/4/15.
//

extern "C"
{
#include <libswresample/swresample.h>
}

#include "XLog.h"
#include <libavcodec/avcodec.h>
#include "FFResample.h"

bool FFResample::Open(XParameter in, XParameter out) {
    //音频重采样上下文初始化
    actx = swr_alloc();
    actx = swr_alloc_set_opts(actx,
                              av_get_default_channel_layout(out.channels),
                              AV_SAMPLE_FMT_S16, out.sampleRate,
                              av_get_default_channel_layout(in.para->channels),
                              (AVSampleFormat) in.para->format, in.para->sample_rate,
                              0, 0);

    int re = swr_init(actx);
    if (re != 0) {
        XLOGE("swr_init failed!");
        return false;
    } else {
        XLOGI("swr_init success!");
    }
    outChannels = in.para->channels;
    outFormat = AV_SAMPLE_FMT_S16;
    return true;

}

XData FFResample::Resample(XData indata) {
    if (indata.size <= 0 || !indata.data) {
        return XData();
    }
    if (!actx) {
        return XData();
    }
    AVFrame *frame = (AVFrame *) indata.data;

    //输出空间的分配
    XData out;
    int outsize =
            outChannels * frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat) outFormat);
    if (outsize <= 0) {
        return XData();
    }
    out.Alloc(outsize);
    uint8_t *outArr[2] = {0};
    outArr[0] = out.data;
    int len = swr_convert(actx, outArr, frame->nb_samples, (const uint8_t **) frame->data,
                          frame->nb_samples);
    if (len <= 0) {
        out.Drop();
        return XData();
    }
    // XLOGI("swr_convert success = %d", len);
    return out;
}