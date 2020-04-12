//
// Created by luopan on 2020/4/12.
//

extern "C" {
#include <libavformat/avformat.h> // Demux library head file
}

#include "XData.h"

void XData::Drop() {
    if (!data) {
        return;
    }
    av_packet_free(reinterpret_cast<AVPacket **>(&data));
    data = 0;
    size = 0;
}