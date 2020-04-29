//
// Created by luopan on 2020/4/12.
//

#ifndef FFMPEG_IOBSERVER_H
#define FFMPEG_IOBSERVER_H

#include "XData.h"
#include "XThread.h"
#include <vector>
#include <mutex>

/**
 * 观察者和主体---线程安全
 */
class IObserver : public XThread {
public:
    /**
     * 观察者接收数据
     *
     * @param 音视频帧数据
     */
    virtual void Update(XData data) {};

    /**
     * 主体函数，添加观察者
     *
     * @param observer
     */
    void AddObserver(IObserver *observer);

    /**
     * 主体通知所有观察者
     *
     * @param data 音视频帧数据
     */
    void Notify(XData data);

protected:
    /**
     * 观察者集合
     */
    std::vector<IObserver *> obss;

    /**
     * 线程安全的互斥锁
     */
    std::mutex mux;
};


#endif //FFMPEG_IOBSERVER_H