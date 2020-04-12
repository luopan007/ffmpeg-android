//
// Created by luopan on 2020/4/12.
//

#ifndef FFMPEG_FFDEMUX_H
#define FFMPEG_FFDEMUX_H


#include "IDemux.h"

class FFDemux : public IDemux {
public:
    /**
     * Open the video or audio file or stream url, such as: rmtp http rtsp
     *
     * @param url the target url
     * @return success or fail
     */
    virtual bool Open(const char *url);

    /**
     * Read one frame data
     * <p>
     * NOTE: The return data is cleared or not by the user decision.
     *
     * @return The frame data
     */
    virtual XData Read();
};


#endif //FFMPEG_FFDEMUX_H