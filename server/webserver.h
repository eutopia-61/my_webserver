#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>
#include <assert.h>

#include "epoller.h"
#include "../pool/threadpool.h"
#include "../http/httpconn.h"

class WebServer
{
public:
    WebServer(int port, int trigMode, int timeoutMs, 
            bool OptLinger, int connPoolNum, int threadNum);

    ~WebServer();

    void Start();

private:
    bool InitSocket_();
    void InitEventMode_(int trigMode);
    void AddClient_(int fd, sockaddr_in addr);

    void DealListen_();
    void DealRead_(HttpConn* client);
    void DealWrite_(HttpConn* client);

    void SendError_(int fd, const char*info);   //给客户端返回错误消息
    void CloseConn_(HttpConn* client);

    void OnRead_(HttpConn* client);
    void OnWrite_(HttpConn* client);
    void OnProcess(HttpConn* client);

    static const int MAX_FD = 65536;    //最大监听文件描述符

    static int SetFdNonblock(int fd);   //设置非阻塞模式

    int port_;
    int timeoutMS_;     // epoll阻塞时间 MS
    bool openLinger_;   // 是否支持优雅断开
    bool isClose_;
    int listenFd_;
    
    char* srcDir_;      // 资源目录

    uint32_t listenEvent_;
    uint32_t connEvent_;

    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpConn> users_;
};

#endif