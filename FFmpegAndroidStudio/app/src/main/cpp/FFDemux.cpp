//
// Created by luopan on 2020/4/12.
//

#include "FFDemux.h"
#include "XLog.h"

extern "C" {
#include <libavformat/avformat.h> // Demux library head file
}

const int SUCCESS = 0;

const int BUFFER_LENGTH = 1024;

const int ONE_THOUSAND = 1000;

/**
 * Constructor
 */
FFDemux::FFDemux() {
    static bool isFirst = true;
    if (isFirst) {
        isFirst = false;
        // 1. register all demux library
        av_register_all();
        // 2. register all decode library
        avcodec_register_all();
        // 3. register network library
        avformat_network_init();
        XLOGI("register ffmpeg library success.")
    }
}

bool FFDemux::Open(const char *url) {
    int ret = avformat_open_input(&ic, url, 0, 0);
    if (ret != SUCCESS) {
        char buf[BUFFER_LENGTH] = {0};
        av_strerror(ret, buf, sizeof(buf));
        XLOGI("FFDemux Open file %s failed", url);
        return false;
    }
    XLOGI("FFDemux Open file %s success", url);

    ret = avformat_find_stream_info(ic, 0);
    if (ret != SUCCESS) {
        char buf[BUFFER_LENGTH] = {0};
        av_strerror(ret, buf, sizeof(buf));
        XLOGI("FFDemux avformat_find_stream_info [%s] failed", url);
        return false;
    }
    this->total = static_cast<int>(ic->duration / (AV_TIME_BASE / ONE_THOUSAND));
    XLOGI("total = %d ms", this->total);
    return true;
}

XData FFDemux::Read() {
    if (!ic) {
        XLOGW("Read ic is NULL");
        return XData();
    }
    XData data;
    AVPacket *pkt = av_packet_alloc();
    int ret = av_read_frame(ic, pkt);
    if (ret != SUCCESS) {
        av_packet_free(&pkt);
        return XData();
    }
    data.data = reinterpret_cast<unsigned char *>(pkt);
    data.size = pkt->size;
    return data;
}

XParameter FFDemux::GetVideoPara() {
    if (!ic) {
        XLOGW("GetVideoPara ic is NULL");
        return XParameter();
    }
    int ret = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, 0, 0);
    if (ret < SUCCESS) {
        XLOGW("av_find_best_stream of video failed");
        return XParameter();
    }
    XLOGW("GetVideoPara ret = %d", ret);
    XParameter para;
    para.para = ic->streams[ret]->codecpar;
    return para;
}
