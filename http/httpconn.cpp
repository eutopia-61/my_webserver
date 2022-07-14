#include "httpconn.h"

using namespace std;

std::atomic<int> HttpConn::userCount;

HttpConn::HttpConn() { 
    fd_ = -1;
    addr_ = { 0 };
    isClose_ = true;
};

HttpConn::~HttpConn() { 
    Close(); 
};

void HttpConn::init(int fd, const sockaddr_in& addr) {
    assert(fd > 0);
    userCount++;
    addr_ = addr;
    fd_ = fd;
    isClose_ = false;
    printf("Client[%d](%s:%d) in, userCount:%d\n", fd_, GetIP(), GetPort(), (int)userCount);
}

void HttpConn::Close() {
    if(isClose_ == false){
        isClose_ = true; 
        userCount--;
        close(fd_);
        printf("Client[%d](%s:%d) quit, UserCount:%d\n", fd_, GetIP(), GetPort(), (int)userCount);
    }
}

int HttpConn::GetFd() const {
    return fd_;
};

struct sockaddr_in HttpConn::GetAddr() const {
    return addr_;
}

const char* HttpConn::GetIP() const {
    return inet_ntoa(addr_.sin_addr);
}

int HttpConn::GetPort() const {
    return addr_.sin_port;
}