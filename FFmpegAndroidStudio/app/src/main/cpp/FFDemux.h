//
// Created by luopan on 2020/4/12.
//

#ifndef FFMPEG_FFDEMUX_H
#define FFMPEG_FFDEMUX_H


#include "IDemux.h"
#include <mutex>

struct AVFormatContext;

class FFDemux : public IDemux {
public:
    /**
     * Open the video or audio file or stream url, such as: rmtp http rtsp
     *
     * @param url the target url
     * @return success or fail
     */
    virtual bool Open(const char *url);

    virtual bool Seek(double pos);

    virtual void Close();

    /**
     * Get the video parameter for video decode
     *
     * @return video parameter
     */
    virtual XParameter GetVideoPara();

    /**
     * Get the audio parameter for audio decode
     *
     * @return audio parameter
     */
    virtual XParameter GetAudioPara();

    /**
     * Read one frame data
     * <p>
     * NOTE: The return data is cleared or not by the user decision.
     *
     * @return The frame data
     */
    virtual XData Read();

    FFDemux();

private:
    AVFormatContext *ic = 0;
    std::mutex mux;

    int audioStream = 1;

    int videoStream = 0;
};

#endif //FFMPEG_FFDEMUX_H