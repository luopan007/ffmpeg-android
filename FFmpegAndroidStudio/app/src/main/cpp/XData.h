//
// Created by luopan on 2020/4/12.
//

#ifndef FFMPEG_XDATA_H
#define FFMPEG_XDATA_H


struct XData {
    unsigned char *data = 0;
    int size = 0;

    /**
     * Clear XData
     */
    void Drop();
};


#endif //FFMPEG_XDATA_H