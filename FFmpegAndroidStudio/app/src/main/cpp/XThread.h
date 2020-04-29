//
// Created by luopan on 2020/4/12.
//

#ifndef FFMPEG_XTHREAD_H
#define FFMPEG_XTHREAD_H

void XSleep(int mis);

/**
 * c++11 Thread Library
 */
class XThread {
public:
    /**
     * 线程开始
     */
    virtual bool Start();

    /**
     * 线程停止--安全模式
     */
    virtual void Stop();

    /**
     * 线程暂停
     *
     * @param isP 是否暂停
     */
    virtual void SetPause(bool isP);

    /**
     * 返回线程是否处于暂停状态
     */
    virtual bool IsPause() {
        isPausing = isPause;
        return isPause;
    }

    /**
     * Main function
     */
    virtual void Main() {}

protected:
    bool isExit = false;
    bool isRunning = false;
    bool isPause = false;
    bool isPausing = false;
private:
    void ThreadMain();
};

#endif //FFMPEG_XTHREAD_H