//
// Created by luopan on 2020/4/12.
//

#include "FFDecode.h"
#include "XLog.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/jni.h>
}

const int SUCCESS = 0;

const int BUFFER_LENGTH = 1024;

const int DEFAULT_THREAD_NUM = 8;

void FFDecode::InitHard(void *vm) {
    av_jni_set_java_vm(vm, 0);
}

bool FFDecode::Open(XParameter para, bool isHard) {
    if (!para.para) {
        XLOGW("FFDecode::Open para is null");
        return false;
    }
    AVCodecParameters *parameters = para.para;
    // 1 查找解码器
    AVCodec *cd = avcodec_find_decoder(parameters->codec_id);
    if (isHard) {
        cd = avcodec_find_decoder_by_name("h264_mediacodec");
    }
    if (!cd) {
        XLOGE("avcodec_find_decoder %d failed!", parameters->codec_id);
        return false;
    }
    XLOGI("avcodec_find_decoder success %d!", isHard);

    // 2 创建解码上下文，并复制参数
    codec = avcodec_alloc_context3(cd);
    avcodec_parameters_to_context(codec, parameters);
    codec->thread_count = DEFAULT_THREAD_NUM;

    // 3 打开解码器
    int ret = avcodec_open2(codec, 0, 0);
    if (ret != SUCCESS) {
        char buf[BUFFER_LENGTH] = {0};
        av_strerror(ret, buf, sizeof(buf) - 1);
        XLOGE("%s", buf);
        return false;
    }

    if (codec->codec_type == AVMEDIA_TYPE_VIDEO) {
        this->isAudio = false;
    } else {
        this->isAudio = true;
    }
    XLOGI("avcodec_open2 success!");
    return true;
}

bool FFDecode::SendPacket(XData pkt) {
    if (pkt.size <= 0 || !pkt.data) {
        return false;
    }
    if (!codec) {
        return false;
    }
    int ret = avcodec_send_packet(codec, (AVPacket *) pkt.data);
    if (ret != SUCCESS) {
        return false;
    }
    return true;
}

XData FFDecode::RecvFrame() {
    if (!codec) {
        return XData();
    }
    if (!frame) {
        frame = av_frame_alloc();
    }
    int ret = avcodec_receive_frame(codec, frame);
    if (ret != SUCCESS) {
        return XData();
    }
    XData data;
    data.data = reinterpret_cast<unsigned char *>(frame);
    if (codec->codec_type == AVMEDIA_TYPE_VIDEO) {
        data.size = (frame->linesize[0] + frame->linesize[1] + frame->linesize[2]) * frame->height;
        data.width = frame->width;
        data.height = frame->height;
    } else if (codec->codec_type == AVMEDIA_TYPE_AUDIO) {
        // 计算方法：样本大小 * 单通道样本数 * 通道数
        data.size = av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->format)) *
                    frame->nb_samples * 2;
    } else {
        XLOGW("invalid type");
    }
    data.format = frame->format;
    if (!isAudio) {
        XLOGI("format %d", frame->format);
    }
    memcpy(data.datas, frame->data, sizeof(data.datas));
    return data;
}