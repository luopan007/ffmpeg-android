//
// Created by luopan on 2020/4/12.
//

#ifndef FFMPEG_IDEMUX_H
#define FFMPEG_IDEMUX_H


#include "XData.h"
#include "XThread.h"
#include "IObserver.h"

class IDemux : public IObserver {
public:
    /**
     * Open the video or audio file or stream url, such as: rmtp http rtsp
     *
     * @param url the target url
     * @return success or fail
     */
    virtual bool Open(const char *url) = 0;

    /**
     * Read one frame data
     * <p>
     * NOTE: The return data is cleared or not by the user decision.
     *
     * @return The frame data
     */
    virtual XData Read() = 0;

    /**
     * File duration in millisecond
     */
    int total;

protected:
    /**
     * Extends from XThread
     */
    virtual void Main();
};


#endif //FFMPEG_IDEMUX_H