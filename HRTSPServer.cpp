#include "HRTSPServer.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include "frameSource.h"
#include "frameServerMediaSubsession.h"
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <iostream>

unsigned short HRTSPServer::port = 8554; // 默认 RTSP 端口
const char* HRTSPServer::inputFileName = nullptr;

void receiveFrames(FrameSource* frameSource);


void HRTSPServer::start() {
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

    RTSPServer* rtspServer = RTSPServer::createNew(*env, HRTSPServer::port);
    if (rtspServer == nullptr) {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        exit(1);
    }

    // ---------------- 实时帧流会话 ----------------
    FrameSource* frameSource = FrameSource::createNew(*env);

    ServerMediaSession* realtimeSms = ServerMediaSession::createNew(
        *env, "realtime", nullptr, "Realtime video session");
    realtimeSms->addSubsession(FrameServerMediaSubsession::createNew(*env, frameSource));
    rtspServer->addServerMediaSession(realtimeSms);

    *env << "Realtime RTSP server started at rtsp://<your-ip>:" << HRTSPServer::port << "/realtime\n";

    // 启动帧接收线程
    std::thread frameReceiverThread(&HRTSPServer::receiveFrames, this, frameSource); // 修正线程调用
    frameReceiverThread.detach();

    // ---------------- 本地视频文件流会话 ----------------
    ServerMediaSession* fileSms = ServerMediaSession::createNew(
        *env, "video", HRTSPServer::inputFileName, "Session streamed by RTSP server");
    fileSms->addSubsession(H264VideoFileServerMediaSubsession::createNew(
        *env, HRTSPServer::inputFileName, True));
    rtspServer->addServerMediaSession(fileSms);

    *env << "Local file RTSP server started at rtsp://<your-ip>:" << HRTSPServer::port << "/video\n";

    env->taskScheduler().doEventLoop(); // 阻塞事件循环
}

void HRTSPServer::receiveFrames(FrameSource* source) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return;
    }

    struct sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(9000);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return;
    }

    while (true) {
        uint8_t buffer[65536]; // 最大帧大小为 64 KB
        ssize_t bytesReceived = recv(sockfd, buffer, sizeof(buffer), 0);

        if (bytesReceived > 0) {
            source->deliverFrame(buffer, static_cast<unsigned>(bytesReceived));
        }
    }

    close(sockfd);
}

HRTSPServer::~HRTSPServer() {}
