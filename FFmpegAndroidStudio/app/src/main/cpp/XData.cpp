//
// Created by luopan on 2020/4/12.
//

#include "XData.h"

extern "C" {
#include <libavformat/avformat.h> // Demux library head file
}


bool XData::Alloc(int size, const char *d) {
    Drop();
    type = UCHAR_TYPE;
    if (size <= 0) {
        return false;
    }
    this->data = new unsigned char[size];
    if (!this->data) {
        return false;
    }
    if (d) {
        memcpy(this->data, d, size);
    }
    return true;
}

void XData::Drop() {
    if (!data) return;
    if (type == AVPACKET_TYPE) {
        av_packet_free((AVPacket **) &data);
    } else {
        delete data;
    }
    data = 0;
    size = 0;
}