#pragma once

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "epoller.h"
#include "../pool/threadpool.h"

class WebServer
{
public:
    WebServer(int port, bool OptLinger, int connPoolNum, int threadNum);
    ~WebServer();

    void Start();

private:
    bool InitSocket_();
    void AddClient_(int fd, sockaddr_in addr);


    static const int MAX_FD = 65536;

    //设置非阻塞模式
    static int SetFdNonblock(int fd);

    int port_;
    bool openLinger_;   // 是否支持优雅断开
    bool isClose_;
    int listenFd_;
    // 资源目录
    char* srcDir_;

    uint32_t listenEvent_;
    uint32_t connEvent_;

    std::unique_ptr<ThreadPool> threadpool_;
};