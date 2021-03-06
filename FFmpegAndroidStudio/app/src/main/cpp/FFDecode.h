//
// Created by luopan on 2020/4/12.
//

#ifndef FFMPEG_FFDECODE_H
#define FFMPEG_FFDECODE_H

#include "XParameter.h"
#include "IDecode.h"

struct AVCodecContext;
struct AVFrame;

/**
 * 解码接口，支持硬解码
 */
class FFDecode : public IDecode {
public:
    /**
     * 初始化硬解码
     *
     * @param vm java virtual meachine
     */
    static void InitHard(void *vm);

    /**
     * 打开解码器参数
     *
     * @return 是否打开成功
     */
    virtual bool Open(XParameter para, bool isHard = false);

    /**
     * 关闭
     */
    virtual void Close();

    /**
     * 清理
     */
    virtual void Clear();

    /**
     * Future模型：发送数据到线程解码
    *
    * @return
    */
    virtual bool SendPacket(XData pkt);

    /**
     * Future模型：从线程中获取解码结果，再次调用会复用上次空间，线程不安全
     *
     * @return
     */
    virtual XData RecvFrame();

protected:
    /**
     * 解码器上下文，复用
     */
    AVCodecContext *codec = 0;

    /**
     * 解码后的数据
     */
    AVFrame *frame = 0;
    std::mutex mux;
};


#endif //FFMPEG_FFDECODE_H
