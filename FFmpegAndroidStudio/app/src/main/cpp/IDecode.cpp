//
// Created by luopan on 2020/4/12.
//

#include "IDecode.h"
#include "XLog.h"

const int DECODE_WAIT_TIME = 1;

/**
 * 由解封装器（主体）的线程回调过来给解码器（观察者）
 *
 * @param pkt 解封装后的音视频数据
 */
void IDecode::Update(XData pkt) {
    if (pkt.isAudio != isAudio) {
        return;
    }
    while (!isExit) {
        packsMutex.lock();
        // 如果缓存过多，阻塞等待解码器
        if (packs.size() < maxList) {
            packs.push_back(pkt);
            packsMutex.unlock();
            break;
        }
        packsMutex.unlock();
        XSleep(DECODE_WAIT_TIME);
    }
}

void IDecode::Main() {
    while (!isExit) {
        packsMutex.lock();
        if (packs.empty()) {
            packsMutex.unlock();
            XSleep(DECODE_WAIT_TIME); // 所有循环的都必须添加睡眠，释放CPU
            continue;
        }
        // 取出Packet，并且从链表中删除
        XData pck = packs.front();
        packs.pop_front();

        // 发送数据到解码线程，并不是正真的解码
        if (this->SendPacket(pck)) {
            while (!isExit) {
                // 一个数据包可能有多个数据帧，获取解码后的帧数据
                XData frame = this->RecvFrame();
                if (!frame.data) {
                    break;
                }
                // XLOGW("RecvFrame size %d", frame.size);
                // 发送数据给观察者-----音频输出或者视频显示
                this->Notify(frame);
            }
        }
        pck.Drop(); // 必须释放，否则会内存泄漏
        packsMutex.unlock();
    }
}