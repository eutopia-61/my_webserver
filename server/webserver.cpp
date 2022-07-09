#include "webserver.h"

using namespace std;

WebServer::WebServer(int port, bool OptLinger, int connPoolNum, int threadNum) :
    port_(port), openLinger_(OptLinger), isClose_(false), threadpool_(new ThreadPool(threadNum))
    {   
        // 设置资源目录
        srcDir_ = getcwd(nullptr, 256);
        assert(srcDir_);
        strncat(srcDir_, "/resources/", 16);

        if(!InitSocket_())
            isClose_ = true;
    }

WebServer::~WebServer()
{
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
}

void WebServer::Start() {
    if(!isClose_) {
        printf("=============== Server Started =================");
    }
    while(!isClose_) {
        
    }
}

//创建连接
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