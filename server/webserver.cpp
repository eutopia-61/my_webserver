#include "webserver.h"

using namespace std;

WebServer::WebServer(
        int port, int trigMode, int timeoutMs, bool OptLinger, 
        int connPoolNum, int threadNum) :
        port_(port), timeoutMS_(timeoutMs), openLinger_(OptLinger), isClose_(false), 
        threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller())
    {   
        // 设置资源目录
        srcDir_ = getcwd(nullptr, 256);
        assert(srcDir_);
        strncat(srcDir_, "/resources/", 16);

        InitEventMode_(trigMode);
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

void WebServer::Start() {
    int timeMS = -1;     /* epoll wait timeout == -1 无事件将阻塞 */
    if(!isClose_) 
        printf("=============== Server Started =================");
    while(!isClose_) {
        int eventCnt = epoller_->Wait(timeMS);

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
            }
            else if(events & EPOLLIN) {
                // 读事件
            }
            else if(events & EPOLLOUT) {
                // 写事件
            }
            else {
                printf("unexpected event");
            }
        }
    }
}

void WebServer::SendError_(int fd, const char* info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0)
        printf("send error to client[%d] error!", fd);

    close(fd);
}

void WebServer::CloseConn_(HttpConn* client) {

}

void WebServer::AddClient_(int fd, sockaddr_in addr) {
    assert(fd >= 0);
    
    users_[fd].init(fd, addr);
    epoller_->AddFd(fd, EPOLLIN);
    SetFdNonblock(fd);
    printf("Client[%d] in!", fd);
}

void WebServer::DealListen_() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_, (struct sockaddr *)&addr, &len);
        if(fd < 0)
            return;
        else if(HttpConn::userCount >= MAX_FD) {
            SendError_(fd, "Server busy!");
            printf("Clients is full!");
            return ;
        }
        AddClient_(fd, addr);
    } while(listenEvent_ & EPOLLET);
}

//创建socket
bool WebServer::InitSocket_() {
    int ret;
    struct sockaddr_in addr;
    if(port_ > 65535 || port_ < 1024) {
        printf("Port:%d error!",  port_);
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
    
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0) {
        printf("Create socket error!");
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
        printf("Init linger error!");
        return false;
    }

    //端口复用
    /* 只有最后一个套接字会正常接收数据。 */
    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    if(ret < 0) {
        printf("set socket setsockopt error !");
        close(listenFd_);
        return false;
    }

    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        printf("Bind Port:%d error!", port_);
        close(listenFd_);
        return false;
    }

    ret = listen(listenFd_, 6);
    if(ret < 0) {
        printf("Listen Port:%d error!", port_);
        close(listenFd_);
        return false;
    }

    SetFdNonblock(listenFd_);
    printf("Server port: %d", port_);
    return true;
}

int WebServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}