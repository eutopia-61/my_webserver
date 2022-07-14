#include "webserver.h"
#include <iostream>

using namespace std;

WebServer::WebServer(
        int port, int trigMode, int timeoutMs, bool OptLinger, 
        int connPoolNum, int threadNum) :
        port_(port), timeoutMS_(timeoutMs), openLinger_(OptLinger), isClose_(false), 
        threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller())
    {   
        printf("WebServer started\n");
        // 设置资源目录
        srcDir_ = getcwd(nullptr, 256);
        assert(srcDir_);
        strncat(srcDir_, "/resources/", 16);

        InitEventMode_(trigMode);
        printf("InitSocket_\r\n");
        if(!InitSocket_())
            isClose_ = true;
    }


WebServer::~WebServer()
{
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
}

void WebServer::InitEventMode_(int trigMode)
{
    listenEvent_ = EPOLLRDHUP;  // EPOLLRDHUP 事件，代表对端断开连接
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch(trigMode)
    {
        case 0:
            break;
        case 1:
            connEvent_ |= EPOLLET;  //EPOLLET边缘触发，只通知一次
            break;
        case 2:
            listenEvent_ |= EPOLLET;
            break;
        case 3:
            listenEvent_ |= EPOLLET;
            connEvent_ |= EPOLLET;
            break;
        default:
            listenEvent_ = EPOLLET;
            connEvent_ |= EPOLLET;
            break;
    }
}

/*Epoll事件处理*/
void WebServer::Start() {
    int timeMS = 500;     /* epoll wait timeout == -1 无事件将阻塞 */
    if(!isClose_) 
        printf("=============== Server Started =================\n");
    while(!isClose_) {
        int eventCnt = epoller_->Wait(timeMS);

        //printf("eventCnt = %d\n", eventCnt);

        for(int i = 0; i < eventCnt; i++) {
            /*处理事件*/
            int fd = epoller_->GetEventFd(i);
            uint32_t events = epoller_->GetEvents(i);
            if(fd == listenFd_) {
                // 处理监听事件，创建连接
                DealListen_();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                // 对方断开、错误事件
                printf("断开!\n");
                assert(users_.count(fd) > 0);
                CloseConn_(&users_[fd]);
            }
            else if(events & EPOLLIN) {
                // 读事件
                printf("test1\n");
                assert(users_.count(fd) > 0);
                DealRead_(&users_[fd]);
            }
            else if(events & EPOLLOUT) {
                // 写事件
                assert(users_.count(fd) > 0);
                DealWrite_(&users_[fd]);
            }
            else {
                printf("unexpected event\n");
            }
        }
    }
}

void WebServer::SendError_(int fd, const char* info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0)
        printf("send error to client[%d] error!\n", fd);

    close(fd);
}

void WebServer::CloseConn_(HttpConn* client) {
    assert(client);
    printf("Client[%d] quit!\n", client->GetFd());
    epoller_->DelFd(client->GetFd());
    client->Close();
}

void WebServer::AddClient_(int fd, sockaddr_in addr) {
    assert(fd >= 0);
    
    users_[fd].init(fd, addr);
    epoller_->AddFd(fd, EPOLLIN | connEvent_);
    SetFdNonblock(fd);
    printf("Client[%d] in!\n", users_[fd].GetFd());
}

void WebServer::DealListen_() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_, (struct sockaddr *)&addr, &len);
        if(fd < 0)
            return;
        else if(HttpConn::userCount >= MAX_FD) {
            SendError_(fd, "Server busy!\n");
            printf("Clients is full!\n");
            return ;
        }
        AddClient_(fd, addr);
    } while(listenEvent_ & EPOLLET);
}

void WebServer::DealRead_(HttpConn* client) {
    assert(client);

    send(client->GetFd(), "hello!", strlen("hello"), 0);
    epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);

    printf("read\n");
}

void WebServer::DealWrite_(HttpConn* client) {
    assert(client);

    printf("write\n");
}

void WebServer::OnRead_(HttpConn* client)
{
    assert(client);
}

//创建socket
bool WebServer::InitSocket_() {
    int ret;
    struct sockaddr_in addr;
    if(port_ > 65535 || port_ < 1024) {
        printf("Port:%d error!\n",  port_);
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
    
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0) {
        printf("Create socket error!\n");
        return false;
    }

    // TCP 关闭方式
    struct linger optLiger = {0};
    if(openLinger_) {
        /* 优雅关闭: 直到所剩数据发送完毕或超时 */
        optLiger.l_linger = 1;
        optLiger.l_onoff = 1;
    }
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLiger, sizeof(optLiger));
    if(ret < 0) {
        close(listenFd_);
        printf("Init linger error!\n");
        return false;
    }

    //端口复用
    /* 只有最后一个套接字会正常接收数据。 */
    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    if(ret < 0) {
        printf("set socket setsockopt error!\n");
        close(listenFd_);
        return false;
    }

    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        printf("Bind Port:%d error!\n", port_);
        close(listenFd_);
        return false;
    }

    ret = listen(listenFd_, 6);
    if(ret < 0) {
        printf("Listen Port:%d error!\n", port_);
        close(listenFd_);
        return false;
    }
    
    ret = epoller_->AddFd(listenFd_, listenEvent_ | EPOLLIN);
    if(ret == 0) {
        printf("Add Listen error!\n");
        close(listenFd_);
        return false;
    }
    SetFdNonblock(listenFd_);

    printf("Server port: %d\n", port_);
    return true;
}

int WebServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}