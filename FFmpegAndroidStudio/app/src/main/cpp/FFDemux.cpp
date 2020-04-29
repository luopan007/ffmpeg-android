//
// Created by luopan on 2020/4/12.
//

#include "FFDemux.h"
#include "XLog.h"

extern "C" {
#include <libavformat/avformat.h> // Demux library head file
}

//分数转为浮点数
static double r2d(AVRational r) {
    return r.num == 0 || r.den == 0 ? 0. : (double) r.num / (double) r.den;
}

void FFDemux::Close() {
    mux.lock();
    if (ic) {
        avformat_close_input(&ic);
    }
    mux.unlock();
}

bool FFDemux::Seek(double pos) {
    if (pos < 0 || pos > 1) {
        XLOGW("Seek value must 0.0~1.0");
        return false;
    }
    bool re = false;
    mux.lock();
    if (!ic) {
        mux.unlock();
        return false;
    }
    //清理读取的缓冲
    avformat_flush(ic);
    long long seekPts = 0;
    seekPts = ic->streams[videoStream]->duration * pos;

    //往后跳转到关键帧
    re = av_seek_frame(ic, videoStream, seekPts, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
    mux.unlock();
    return re;
}

static const int SUCCESS = 0;

static const int BUFFER_LENGTH = 1024;

static const int ONE_THOUSAND = 1000;

bool FFDemux::Open(const char *url) {
    XLOGI("Open file %s begin", url);
    Close();
    mux.lock();
    int ret = avformat_open_input(&ic, url, 0, 0);
    if (ret != SUCCESS) {
        mux.unlock();
        char buf[BUFFER_LENGTH] = {0};
        av_strerror(ret, buf, sizeof(buf));
        XLOGI("FFDemux Open file %s failed", url);
        return false;
    }
    XLOGI("FFDemux Open file %s success", url);

    ret = avformat_find_stream_info(ic, 0);
    if (ret != SUCCESS) {
        mux.unlock();
        char buf[BUFFER_LENGTH] = {0};
        av_strerror(ret, buf, sizeof(buf));
        XLOGI("FFDemux avformat_find_stream_info [%s] failed", url);
        return false;
    }
    this->total = static_cast<int>(ic->duration / (AV_TIME_BASE / ONE_THOUSAND));
    XLOGI("total = %d ms", this->total);
    mux.unlock();
    // Initial audio and video stream index.

    GetVideoPara();
    GetAudioPara();
    return true;
}

XParameter FFDemux::GetVideoPara() {
    mux.lock();
    if (!ic) {
        mux.unlock();
        XLOGW("GetVideoPara ic is NULL");
        return XParameter();
    }
    int ret = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, 0, 0);
    if (ret < SUCCESS) {
        mux.unlock();
        XLOGW("av_find_best_stream of video failed");
        return XParameter();
    }
    XLOGW("GetVideoPara ret = %d", ret);
    videoStream = ret;
    XParameter para;
    para.para = ic->streams[ret]->codecpar;
    mux.unlock();
    return para;
}

XParameter FFDemux::GetAudioPara() {
    mux.lock();
    if (!ic) {
        mux.unlock();
        XLOGW("GetAudioPara ic is NULL");
        return XParameter();
    }
    int ret = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, 0, 0);
    if (ret < SUCCESS) {
        mux.unlock();
        XLOGW("av_find_best_stream of audio failed");
        return XParameter();
    }
    XLOGW("GetAudioPara ret = %d", ret);
    audioStream = ret;
    XParameter para;
    para.para = ic->streams[ret]->codecpar;
    para.channels = ic->streams[ret]->codecpar->channels;
    para.sampleRate = ic->streams[ret]->codecpar->sample_rate;
    mux.unlock();
    return para;
}

XData FFDemux::Read() {
    mux.lock();
    if (!ic) {
        XLOGW("Read ic is NULL");
        mux.unlock();
        return XData();
    }
    XData data;
    AVPacket *pkt = av_packet_alloc();
    int ret = av_read_frame(ic, pkt);
    if (ret != SUCCESS) {
        mux.unlock();
        av_packet_free(&pkt);
        return XData();
    }
    data.data = reinterpret_cast<unsigned char *>(pkt);
    data.size = pkt->size;

    if (pkt->stream_index == audioStream) {
        data.isAudio = true;
    } else if (pkt->stream_index == videoStream) {
        data.isAudio = false;
    } else {
        XLOGW("Invalid type");
        mux.unlock();
        av_packet_free(&pkt);
        return XData();
    }
    //转换pts
    pkt->pts = static_cast<int64_t>(pkt->pts *
                                    (1000 * r2d(ic->streams[pkt->stream_index]->time_base)));
    pkt->dts = static_cast<int64_t>(pkt->dts *
                                    (1000 * r2d(ic->streams[pkt->stream_index]->time_base)));
    data.pts = (int) pkt->pts;
    mux.unlock();
    return data;
}

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