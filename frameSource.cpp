#include "frameSource.h"
#include <cstring> // 用于 memmove

// 创建一个新的 FrameSource 实例
FrameSource* FrameSource::createNew(UsageEnvironment& env) {
    return new FrameSource(env);
}

// FrameSource 构造函数
FrameSource::FrameSource(UsageEnvironment& env)
    : FramedSource(env) {}

// 提供帧数据
void FrameSource::deliverFrame(const uint8_t* frameData, unsigned frameSize) {
    if (!isCurrentlyAwaitingData()) {
        // 如果客户端没有请求数据，则直接返回
        return;
    }

    // 将帧数据复制到父类的缓冲区中
    memmove(fTo, frameData, frameSize);
    fFrameSize = frameSize;

    // 通知 Live555，数据已经准备好
    FramedSource::afterGetting(this);
}

// 在客户端请求下一帧时调用
void FrameSource::doGetNextFrame() {
    // 此方法会被 Live555 的事件循环调用，当客户端需要新的一帧时触发。
    // 实际的帧数据需要通过 deliverFrame() 来提供。
}
