//
// Created by luopan on 2020/4/12.
//

#ifndef FFMPEG_IDECODE_H
#define FFMPEG_IDECODE_H

#include "XParameter.h"
#include "IObserver.h"
#include <list>

/**
 * 解码接口，支持硬解码
 */
class IDecode : public IObserver {
public:
    /**
     * 打开解码器参数
     *
     * @return 是否打开成功
     */
    virtual bool Open(XParameter para, bool isHard = false) = 0;

    virtual void Close() = 0;

    virtual void Clear();

    /**
     * Future模型：发送数据到线程解码
     *
     * @return
     */
    virtual bool SendPacket(XData pkt) = 0;

    /**
     * Future模型：从线程中获取解码结果，从线程中获取解码结果，再次调用会复用上次空间，线程不安全
     *
     * @return
     */
    virtual XData RecvFrame() = 0;

    /**
     * 观察者接收数据 --- 由解封装器（主体）的线程回调过来
     *
     * @param 音视频帧数据
     */
    virtual void Update(XData pkt);

    /**
     * 是否为音频数据
     */
    bool isAudio = false;

    /**
     * 最多存储100帧数据，如果按照25FPS计算，就是4秒缓存
     */
    int maxList = 100;

    /**
     * 同步时间，再次打开文件要清理
     */
    int synPts = 0;

    int pts = 0;
protected:
    /**
     * 线程解码主函数
     */
    virtual void Main();

    /**
     * 存放所有缓冲帧
     */
    std::list<XData> packs;

    /**
     * 缓存队列的互斥变量
     */
    std::mutex packsMutex;
};

#endif //FFMPEG_IDECODE_H