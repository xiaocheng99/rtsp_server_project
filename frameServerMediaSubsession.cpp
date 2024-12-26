
#include "frameServerMediaSubsession.h"
#include "frameSource.h"
#include <GroupsockHelper.hh>

// 创建 FrameServerMediaSubsession 的实例
FrameServerMediaSubsession* FrameServerMediaSubsession::createNew(UsageEnvironment& env, FrameSource* source) {
    return new FrameServerMediaSubsession(env, source);
}

// 构造函数
FrameServerMediaSubsession::FrameServerMediaSubsession(UsageEnvironment& env, FrameSource* source)
    : OnDemandServerMediaSubsession(env, True /*reuse the first source*/),
      frameSource(source) {
}

FrameServerMediaSubsession::~FrameServerMediaSubsession() {
}

// 用于创建新的流源（每个客户端一个）
FramedSource* FrameServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate) {
    return frameSource;
}

// 用于创建 RTP Sink
RTPSink* FrameServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) {
    return nullptr;
}

