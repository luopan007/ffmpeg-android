//
// Created by luopan on 2020/4/17.
//

#include "IAudioPlay.h"
#include "XLog.h"

XData IAudioPlay::GetData() {
    XData d;
    while (!isExit) {
        framesMutes.lock();
        if (!frames.empty()) {
            d = frames.front();
            frames.pop_front();
            framesMutes.unlock();
            return d;
        }
        framesMutes.unlock();
        XSleep(1);
    }
    return d;
}

void IAudioPlay::Update(XData data) {
    // 把接收的数据压入缓冲队列
    if (data.size <= 0 || !data.data) {
        XLOGW("Resample send Data here, but data is null");
        return;
    }
    while (!isExit) {
        framesMutes.lock();
        if (frames.size() > maxFrame) {
            framesMutes.unlock();
            XSleep(1);
            continue;
        }
        frames.push_back(data);
        framesMutes.unlock();
        break;
    }
}