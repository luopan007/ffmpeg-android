//
// Created by luopan on 2020/4/12.
//

#ifndef FFMPEG_XTHREAD_H
#define FFMPEG_XTHREAD_H


/**
 * c++11 Thread Library
 */
class XThread {
public:
    /**
     * Start thread
     */
    virtual void Start();

    /**
     * Stop thread(safe mode)
     */
    virtual void Stop();

    /**
     * Main function
     */
    virtual void Main();;
};

#endif //FFMPEG_XTHREAD_H