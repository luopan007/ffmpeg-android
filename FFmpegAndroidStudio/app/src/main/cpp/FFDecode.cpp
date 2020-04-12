//
// Created by luopan on 2020/4/12.
//

#include "FFDecode.h"
#include "XLog.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

const int SUCCESS = 0;

const int BUFFER_LENGTH = 1024;

bool FFDecode::Open(XParameter para) {
    if (!para.para) return false;
    AVCodecParameters *p = para.para;
    //1 查找解码器
    AVCodec *cd = avcodec_find_decoder(p->codec_id);
    if (!cd) {
        XLOGE("avcodec_find_decoder %d failed!", p->codec_id);
        return false;
    }
    XLOGI("avcodec_find_decoder success!");
    //2 创建解码上下文，并复制参数
    codec = avcodec_alloc_context3(cd);
    avcodec_parameters_to_context(codec, p);

    codec->thread_count = 8;
    //3 打开解码器
    int re = avcodec_open2(codec, 0, 0);
    if (re != 0) {
        char buf[1024] = {0};
        av_strerror(re, buf, sizeof(buf) - 1);
        XLOGE("%s", buf);
        return false;
    }
    XLOGI("avcodec_open2 success!");
    return true;
}